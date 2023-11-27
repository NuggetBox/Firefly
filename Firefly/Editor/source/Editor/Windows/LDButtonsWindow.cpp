#include "EditorPch.h"
#include "LDButtonsWindow.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/EditorLayer.h"
#include "Utils/Math/Random.hpp"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Prefab.h"
#include "Editor/UndoSystem/Commands/EntityCommands/TransformCommand.h"
#include <Firefly/Components/Mesh/MeshComponent.h>

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/Exporters/FBXExporter.h"
#include <Editor/Utilities/EditorUtils.h>
#include "Firefly/Asset/Mesh/Mesh.h"

REGISTER_WINDOW(LDButtonsWindow);

LDButtonsWindow::LDButtonsWindow()
	: EditorWindow("LDButtons")
{
	myPrefabsToRandomlyOffset.push_back("");
}

void LDButtonsWindow::OnImGui()
{
	if (ImGui::CollapsingHeader("Offset prefabs randomly"))
	{
		ImGui::PushID(ImGui::GetID("Offset prefabs randomly"));
		ImGui::Text("Target Prefabs:");
		ImGuiUtils::BeginParameters("##OffsetPrefabsRandomlyPrefabs");
		for (int i = 0; i < myPrefabsToRandomlyOffset.size(); i++)
		{
			ImGui::PushID(i);
			ImGuiUtils::FileParameter("Prefab", myPrefabsToRandomlyOffset[i], ".prefab");
			ImGui::PopID();
		}
		ImGuiUtils::EndParameters();
		ImGui::Button("-##Remove Prefab");
		if (ImGui::BeginPopupContextItem("Remove Prefab", ImGuiPopupFlags_MouseButtonLeft))
		{
			for (int i = 0; i < myPrefabsToRandomlyOffset.size(); i++)
			{

				std::string lable = myPrefabsToRandomlyOffset[i];
				if (lable.empty())
				{
					lable = "Empty Prefab " + std::to_string(i);
				}
				if (ImGui::Selectable((lable + "##" + std::to_string(i)).c_str()))
				{
					myPrefabsToRandomlyOffset.erase(myPrefabsToRandomlyOffset.begin() + i);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();;
		if (ImGui::Button("+##Add Prefab"))
		{
			myPrefabsToRandomlyOffset.push_back("");
		}

		ImGuiUtils::BeginParameters("##OffsetPrefabsRandomlyParametersMinMax");
		ImGuiUtils::Parameter("Offset Min", myOffsetMin, 1, 0, 0, "A minimum offset to apply to the prefab");
		ImGuiUtils::Parameter("Offset Max", myOffsetMax, 1, 0, 0, "A maximum offset to apply to the prefab");
		ImGuiUtils::EndParameters();
		if (ImGui::Button("Execute##OffsetPrefabsRandomlyExecute"))
		{
			if (Firefly::Application::Get().GetIsInPlayMode())
			{
				ImGuiUtils::NotifyErrorLocal("Cannot execute in play mode!");
			}
			else
			{
				auto& scenes = EditorLayer::GetEditingScenes();

				for (int i = 0; i < scenes.size(); i++)
				{
					auto& scene = scenes[i];
					for (auto& prefabPath : myPrefabsToRandomlyOffset)
					{
						auto prefab = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(prefabPath);
						auto entities = scene.lock()->GetEntities();
						for (auto& entity : entities)
						{
							if (!entity.lock()->IsPrefabRoot())
							{
								continue;
							}

							if (entity.lock()->GetPrefabID() == prefab->GetPrefabID())
							{
								auto& transform = entity.lock()->GetTransform();

								transform.SetLocalPosition(transform.GetPosition() + Utils::Vector3f(
									Utils::RandomFloat(myOffsetMin.x, myOffsetMax.x),
									Utils::RandomFloat(myOffsetMin.y, myOffsetMax.y),
									Utils::RandomFloat(myOffsetMin.z, myOffsetMax.z)));
							}

						}
					}
				}
			}
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Replace Prefabs"))
	{
		ImGui::PushID("Replace Prefabs");

		ImGui::Text("Target Prefab:");
		ImGuiUtils::BeginParameters("##ReplacePrefabsPrefabs");
		ImGuiUtils::FileParameter("Prefab", myReplacePrefabData.PrefabToReplace, ".prefab");
		ImGuiUtils::FileParameter("Replace With", myReplacePrefabData.PrefabToReplaceWith, ".prefab");
		ImGuiUtils::EndParameters();

		if (ImGui::Button("Replace"))
		{
			EditorLayer::DeselectAllEntities();
			auto prefabToReplace = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(myReplacePrefabData.PrefabToReplace, true);
			auto prefabToReplaceWith = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(myReplacePrefabData.PrefabToReplaceWith, true);

			if (prefabToReplace == nullptr)
			{
				ImGuiUtils::NotifyErrorLocal("Prefab to replace is null!");
				return;
			}
			if (prefabToReplaceWith == nullptr)
			{
				ImGuiUtils::NotifyErrorLocal("Prefab to replace with is null!");
				return;
			}

			auto& scenes = EditorLayer::GetEditingScenes();

			for (int i = 0; i < scenes.size(); i++)
			{
				auto& scene = scenes[i];
				auto entities = scene.lock()->GetEntities();
				for (auto& entity : entities)
				{
					if (!entity.lock()->IsPrefabRoot())
					{
						continue;
					}

					if (entity.lock()->GetPrefabID() == prefabToReplace->GetPrefabID())
					{
						auto& transform = entity.lock()->GetTransform();

						auto newEntity = scene.lock()->Instantiate(prefabToReplaceWith).lock();
						newEntity->SetParent(entity.lock()->GetParent());
						newEntity->GetTransform().SetPosition(transform.GetPosition());
						newEntity->GetTransform().SetRotation(transform.GetQuaternion());
						newEntity->GetTransform().SetScale(transform.GetScale());

						newEntity->UpdateTransformLocalPositionModification();
						newEntity->UpdateTransformLocalRotationModification();
						newEntity->UpdateTransformLocalScaleModification();
						scene.lock()->DeleteEntity(entity.lock()->GetID());
					}

				}
			}
		}


		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Replace Mesh object with prefab"))
	{
		ImGui::PushID("Replace Mesh object with prefab");

		ImGui::Text("Target Prefab:");
		ImGuiUtils::BeginParameters("##ReplacePrefabsPrefabs");
		ImGuiUtils::FileParameter("Mesh", myReplaceMeshObjectData.MeshToReplace, ".mesh");
		ImGuiUtils::FileParameter("Replace With", myReplaceMeshObjectData.PrefabToReplaceWith, ".prefab");
		ImGuiUtils::EndParameters();

		if (ImGui::Button("Replace"))
		{
			EditorLayer::DeselectAllEntities();
			auto prefabToReplaceWith = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(myReplaceMeshObjectData.PrefabToReplaceWith, true);

			if (!prefabToReplaceWith)
			{
				ImGuiUtils::NotifyErrorLocal("Prefab to replace with is null!");
				return;
			}

			auto& scenes = EditorLayer::GetEditingScenes();

			for (int i = 0; i < scenes.size(); i++)
			{
				auto& scene = scenes[i];
				auto entities = scene.lock()->GetEntities();
				for (int i = entities.size() - 1; i >= 0; i--)
				{
					auto entity = entities[i];
					if (entity.expired())
					{
						continue;
					}
					if (entity.lock()->IsPrefab())
					{
						continue;
					}
					if (entity.lock()->HasComponent<Firefly::MeshComponent>())
					{
						auto meshComponent = entity.lock()->GetComponent<Firefly::MeshComponent>();
						if (meshComponent.lock()->GetMeshPath() == myReplaceMeshObjectData.MeshToReplace)
						{
							auto& transform = entity.lock()->GetTransform();

							auto newEntity = scene.lock()->Instantiate(prefabToReplaceWith).lock();
							newEntity->SetParent(entity.lock()->GetParent());
							newEntity->GetTransform().SetPosition(transform.GetPosition());
							newEntity->GetTransform().SetRotation(transform.GetQuaternion());
							newEntity->GetTransform().SetScale(transform.GetScale());

							newEntity->UpdateTransformLocalPositionModification();
							newEntity->UpdateTransformLocalRotationModification();
							newEntity->UpdateTransformLocalScaleModification();
							scene.lock()->DeleteEntity(entity.lock()->GetID());
						}
					}
				}
			}
		}


		ImGui::PopID();
	}

	// Niklas: Added this for Artists
	if (ImGui::CollapsingHeader("Export FBX from selected entities"))
	{
		auto& selectedEntites = EditorLayer::GetSelectedEntities();
		ImGui::Text("Current amount of selected entities: %zu", selectedEntites.size());

		if (ImGui::Button("Export"))
		{
			auto fbxPath = EditorUtils::GetSaveFilePath("Fbx model (*.fbx)\0*.fbx\0", "fbx");

			if (fbxPath.empty())
			{
				return;
			}

			std::vector<std::pair<Ref<Firefly::Mesh>, Utils::Transform>> vector;

			for (auto& selectedEntity : selectedEntites)
			{
				FindMeshComponentAndExtract(selectedEntity, vector);
			}

			Firefly::FBXExporter exporter;
			exporter.ExportMeshPackage(vector, fbxPath);
		}
	}
}

void LDButtonsWindow::FindMeshComponentAndExtract(Ptr<Firefly::Entity> selectedEntity, std::vector<std::pair<Ref<Firefly::Mesh>, Utils::Transform>>& vector)
{
	if (selectedEntity.lock() == nullptr)
	{
		return;
	}

	auto entityPtr = selectedEntity.lock();

	if (entityPtr->HasComponent<Firefly::MeshComponent>())
	{
		ExtractMesh(entityPtr, vector);
	}


	for (auto& child : entityPtr->GetChildren())
	{
		FindMeshComponentAndExtract(child, vector);
	}
}

bool LDButtonsWindow::ExtractMesh(std::shared_ptr<Firefly::Entity>& entityPtr, std::vector<std::pair<Ref<Firefly::Mesh>, Utils::Transform>>& vector)
{
	auto meshComponent = entityPtr->GetComponent<Firefly::MeshComponent>();

	if (meshComponent.lock() == nullptr)
	{
		return false;
	}

	vector.emplace_back(meshComponent.lock()->GetMesh(), entityPtr->GetTransform());
	return true;
}
