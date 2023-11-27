#include "EditorPch.h"
#include "EditorUtils.h"

#include "Firefly/Application/Window.h"
#include "Firefly/Application/Application.h"

#include "imgui/imgui_internal.h"
#include <shlobj_core.h>
#include <Editor/EditorLayer.h>
#include <Editor/UndoSystem/Commands/EntityCommands/TransformCommand.h>
#include <Editor/UndoSystem/Commands/SceneCommands/EntityCreateCommand.h>
#include <Editor/UndoSystem/Commands/SceneCommands/PrefabCreateCommand.h>
#include <Firefly/Asset/Animation.h>
#include <Firefly/Components/Animation/AnimationPlayerComponent.h>
#include <Firefly/Components/Mesh/AnimatedMeshComponent.h>
#include <Firefly/Components/Mesh/MeshComponent.h>
#include <Firefly/Components/ParticleSystem/ParticleEmitterComponent.h>
#include <Firefly/ComponentSystem/SceneManager.h>
#include <Utils/Math/Vector3.hpp>

#include "ImGuiUtils.h"

#include "Firefly/Asset/ResourceCache.h"

namespace EditorUtils
{

	std::filesystem::path GetSaveFilePath(const char* aFilter, const char* fileEnding)
	{
		std::string fileEndingWithDot = "." + std::string(fileEnding);
		OPENFILENAMEA ofn = {};
		char szFileName[MAX_PATH] = "";

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = Firefly::Application::Get().GetWindow()->GetHandle();
		ofn.lpstrFilter = aFilter;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = fileEnding;

		GetSaveFileNameA(&ofn);

		std::filesystem::path path = szFileName;

		if (path != "")
		{
			if (path.has_extension())
			{

				if (path.extension() != fileEndingWithDot)
				{
					path += fileEnding;
				}
			}
			else
			{
				path += fileEndingWithDot;
			}
		}


		return path;
	}

	std::filesystem::path GetOpenFilePath(const char* aFilter)
	{
		OPENFILENAMEA ofn = {};
		char szFileName[MAX_PATH] = "";

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = Firefly::Application::Get().GetWindow()->GetHandle();
		ofn.lpstrFilter = aFilter;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

		GetOpenFileNameA(&ofn);

		return szFileName;
	}

	std::filesystem::path GetFolder()
	{
		std::filesystem::path path = "";
		IFileDialog* pfd = NULL;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));
		if (SUCCEEDED(hr))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}

			hr = pfd->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem* psiResult;
				hr = pfd->GetResult(&psiResult);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						path = pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					psiResult->Release();
				}
			}
			pfd->Release();
		}

		return path;
	}

	bool IsOnlySamePrefabChildrenSelected()
	{
		const auto& selected = EditorLayer::GetSelectedEntities();

		uint64_t rootID = 0;

		for (const auto& ent : selected)
		{
			if (!ent.expired())
			{
				const auto& entity = ent.lock();

				if (rootID == 0)
				{
					rootID = entity->GetPrefabRootEntityID();
				}
				else
				{
					if (entity->GetPrefabRootEntityID() != rootID)
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	bool IsAnySamePrefabChildrenSelected(const Ptr<Firefly::Entity>& aEntity)
	{
		if (!aEntity.expired() && aEntity.lock()->IsPrefab())
		{
			const uint64_t rootID = aEntity.lock()->GetPrefabRootEntityID();

			const auto& selected = EditorLayer::GetSelectedEntities();

			for (const auto& ent : selected)
			{
				if (!ent.expired())
				{
					const auto& entity = ent.lock();

					if (entity->GetPrefabRootEntityID() == rootID && !entity->IsPrefabRoot())
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	int GetParentCount(const Ptr<Firefly::Entity>& aEntity)
	{
		int parentCount = 0;

		if (!aEntity.expired())
		{
			const auto& entity = aEntity.lock();

			Ptr<Firefly::Entity> parent = entity->GetParent();

			while (!parent.expired())
			{
				parentCount++;

				parent = parent.lock()->GetParent();
			}
		}

		return parentCount;
	}

	void SetImGuiPayloadActive(bool aActive)
	{
		auto& context = *ImGui::GetCurrentContext();

		context.DragDropActive = aActive;
	}

	const std::filesystem::path GetRelativePath(const std::filesystem::path& aFullPath)
	{
		return std::filesystem::relative(aFullPath);
	}

	void CreateUserFolder()
	{
		if (!std::filesystem::exists("User/"))
		{
			std::filesystem::create_directory("User/");
		}
	}

	bool AcceptAllDragDrops(const Utils::Vector3f& aPosition)
	{
		const bool dropped =
			AcceptSceneToAdd() ||
			AcceptPrefabToAddToScene(aPosition) ||
			AcceptMeshToAddToScene(aPosition) ||
			AcceptSkeletonToAddToScene(aPosition) ||
			AcceptAnimationToAddToScene(aPosition) ||
			AcceptEmitterToAddToScene(aPosition);

		return dropped;
	}

	bool AcceptSceneToAdd()
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".scene", false))
		{
			const char* file = static_cast<const char*>(payload->Data);

			if (Firefly::SceneManager::Get().IsSceneLoaded(file))
			{
				ImGuiUtils::NotifyError("You can't add a scene that is already loaded!");
				return false;
			}

			Firefly::SceneManager::Get().LoadSceneAdd(file);

			return true;
		}

		return false;
	}

	bool AcceptPrefabToAddToScene(const Utils::Vector3f& aPosition)
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".prefab", false))
		{
			std::filesystem::path file = (const char*)(payload->Data);

			EditorLayer::BeginEntityUndoSeries(true);
			Ref<PrefabCreateCommand> prefabCreateCommand =
				CreateRef<PrefabCreateCommand>(Firefly::ResourceCache::GetAsset<Firefly::Prefab>(file, true), Ptr<Firefly::Entity>(), EditorLayer::GetEditingScenes()[0].lock().get());
			EditorLayer::ExecuteAndAddEntityUndo(prefabCreateCommand, true);

			Ref<TransformCommand> transformCommand = CreateRef<TransformCommand>(prefabCreateCommand->GetCreatedEntity(), aPosition, Utils::Vector3f::Zero(), Utils::Vector3f::One());
			EditorLayer::ExecuteAndAddEntityUndo(transformCommand, true);
			EditorLayer::EndEntityUndoSeries(true);

			if (!prefabCreateCommand->GetCreatedEntity().expired())
			{
				prefabCreateCommand->GetCreatedEntity().lock()->SetName(file.stem().string());
			}

			return true;
		}

		return false;
	}

	bool AcceptMeshToAddToScene(const Utils::Vector3f& aPosition)
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".mesh", true))
		{
			const char* file = static_cast<const char*>(payload->Data);

			//Create the entity
			Ref<EntityCreateCommand> entCreateCommand = CreateRef<EntityCreateCommand>(Ptr<Firefly::Entity>(), EditorLayer::GetEditingScenes()[0].lock().get());
			EditorLayer::ExecuteAndAddEntityUndo(entCreateCommand, true);

			//Add the mesh that was dropped to it
			Ref<Firefly::MeshComponent> meshComp = Firefly::ComponentRegistry::Create<Firefly::MeshComponent>();
			meshComp->SetMesh(file);

			if (!entCreateCommand->GetCreatedEntity().expired())
			{
				const auto& ent = entCreateCommand->GetCreatedEntity().lock();
				ent->AddComponent(meshComp);

				ent->SetName(std::filesystem::path(file).stem().string());
				ent->GetTransform().SetLocalPosition(aPosition);
			}

			EditorLayer::SelectEntity(entCreateCommand->GetCreatedEntity());

			return true;
		}

		return false;
	}

	bool AcceptSkeletonToAddToScene(const Utils::Vector3f& aPosition)
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".skeleton"))
		{
			const char* file = static_cast<const char*>(payload->Data);

			//Create the entity
			Ref<EntityCreateCommand> entCreateCommand = CreateRef<EntityCreateCommand>(Ptr<Firefly::Entity>(), EditorLayer::GetEditingScenes()[0].lock().get());
			EditorLayer::ExecuteAndAddEntityUndo(entCreateCommand, true);

			//Add the mesh that was dropped to it
			Ref<Firefly::AnimatedMeshComponent> skeletonComp = Firefly::ComponentRegistry::Create<Firefly::AnimatedMeshComponent>();
			skeletonComp->SetMesh(file);

			if (!entCreateCommand->GetCreatedEntity().expired())
			{
				const auto& ent = entCreateCommand->GetCreatedEntity().lock();
				ent->AddComponent(skeletonComp);

				ent->SetName(std::filesystem::path(file).stem().string());
				ent->GetTransform().SetLocalPosition(aPosition);
			}

			EditorLayer::SelectEntity(entCreateCommand->GetCreatedEntity());

			return true;
		}

		return false;
	}

	bool AcceptAnimationToAddToScene(const Utils::Vector3f& aPosition)
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".anim", true))
		{
			const char* file = static_cast<const char*>(payload->Data);

			//Create the entity
			Ref<EntityCreateCommand> entCreateCommand = CreateRef<EntityCreateCommand>(Ptr<Firefly::Entity>(), EditorLayer::GetEditingScenes()[0].lock().get());
			EditorLayer::ExecuteAndAddEntityUndo(entCreateCommand, true);

			if (!entCreateCommand->GetCreatedEntity().expired())
			{
				const auto& ent = entCreateCommand->GetCreatedEntity().lock();

				ent->SetName(std::filesystem::path(file).stem().string());

				Ref<Firefly::Animation> anim = Firefly::ResourceCache::GetAsset<Firefly::Animation>(file, true);

				if (anim && anim->IsLoaded())
				{
					Ref<Firefly::AnimatedMeshComponent> meshComp = Firefly::ComponentRegistry::Create<Firefly::AnimatedMeshComponent>();
					meshComp->SetMesh(anim->GetAnimatedMeshPath().string());
					ent->AddComponent(meshComp);

					Ref<Firefly::AnimationPlayerComponent> animPlayer = Firefly::ComponentRegistry::Create<Firefly::AnimationPlayerComponent>();
					animPlayer->SetAnimation(file);
					ent->AddComponent(animPlayer);
				}

				ent->GetTransform().SetLocalPosition(aPosition);

				EditorLayer::SelectEntity(ent);
			}

			return true;
		}

		return false;
	}

	bool AcceptEmitterToAddToScene(const Utils::Vector3f& aPosition)
	{
		if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".emitter", false))
		{
			const char* file = static_cast<const char*>(payload->Data);

			//Create the entity
			Ref<EntityCreateCommand> entCreateCommand = CreateRef<EntityCreateCommand>(Ptr<Firefly::Entity>(), EditorLayer::GetEditingScenes()[0].lock().get());
			EditorLayer::ExecuteAndAddEntityUndo(entCreateCommand, true);

			//Add the emitter component that was dropped
			Ref<Firefly::ParticleEmitterComponent> emitterComp = Firefly::ComponentRegistry::Create<Firefly::ParticleEmitterComponent>();
			emitterComp->SetEmitterTemplatePath(file);
			emitterComp->Initialize();

			if (!entCreateCommand->GetCreatedEntity().expired())
			{
				const auto& ent = entCreateCommand->GetCreatedEntity().lock();
				ent->AddComponent(emitterComp);

				ent->SetName(std::filesystem::path(file).stem().string());
				ent->GetTransform().SetLocalPosition(aPosition);

				EditorLayer::SelectEntity(entCreateCommand->GetCreatedEntity());
			}


			return true;
		}

		return false;
	}
}
