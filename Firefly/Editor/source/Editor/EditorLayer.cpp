#include "EditorPch.h"
#include "EditorLayer.h"

#include <Firefly/Components/Mesh/AnimatedMeshComponent.h>

#include "Utilities/EditorUtils.h"

#include "Editor/Windows/HierarchyWindow.h"
#include "Editor/Windows/InspectorWindow.h"
#include "Editor/Windows/ViewportWindow.h"
#include "Editor/Windows/ContentBrowser.h"
#include "Editor/Windows/ConsoleWindow.h"
#include "Editor/Windows/MaterialEditorWindow.h"
#include "Windows/ParticleEditorWindow.h"

#include "Editor/UndoSystem/UndoHandler.h"
#include "Editor/UndoSystem/Commands/SceneCommands/EntityDuplicateCommand.h"

#include "Editor/Event/EditorOnlyEvents.h"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EngineEvents.h"
#include "Firefly/Event/SceneEvents.h"

#include "Firefly/Application/ExceptionHandler.h"

#include <Firefly/ComponentSystem/SceneManager.h>
#include <Firefly/ComponentSystem/Scene.h>
#include <Firefly/ComponentSystem/Entity.h>
#include <Firefly/ComponentSystem/ComponentRegistry.hpp>

#include "Firefly/Components/Physics/BoxColliderComponent.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Utils/InputHandler.h"

#include "Firefly/Components/Mesh/MeshComponent.h"

#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"
#include <Firefly/Event/EditorEvents.h>
#include <Utils/Timer.h>


#include "Editor/Windows/WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"

#include "UndoSystem/Commands/SceneCommands/EntityDeleteCommand.h"
#include "SerializationUtils.h"
#include <imnotify/imgui_notify.h>

#include <execution>

#include <imgui/imgui_internal.h>

#include "Firefly/Components/VisualScriptComponent.h"

#include "FmodWrapper/AudioManager.h"

#include "UndoSystem/Commands/EntityCommands/EntityDeselectAllCommand.h"
#include "UndoSystem/Commands/EntityCommands/EntityHideCommand.h"
#include "UndoSystem/Commands/SceneCommands/EntityCreateCommand.h"
#include "UndoSystem/Commands/SceneCommands/EntityReorderCommand.h"

#include "Windows/VisualScriptingWindow.h"

#include "Firefly/Components/ParticleSystem/ForceFieldManager.h"

#include "Utils/TimerManager.h"

nlohmann::json myEditorSettingsJson; // is here so i dont have to include nlohmann in header

using namespace Firefly;

EditorLayer::EditorLayer()
{
	ImGuiUtils::Initialize();
	ExceptionHandler::QueueCrashFunc(SaveCrashedScene);

	EditorUtils::CreateUserFolder();

	myBuildFolder = std::filesystem::current_path().parent_path().string() + "\\Build\\";
	/*myScenesToIncludeInBuild.push_back("Assets/Scenes/CoreScene.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/Corridor.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/MainMenuOffice.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/MainMenuDocks.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/SplashScreen.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/Levels/LVL_00_Tutorial.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/Levels/LVL_01_Docks.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/Levels/LVL_02_Factory.scene");
	myScenesToIncludeInBuild.push_back("Assets/Scenes/Levels/LVL_04_Shipping.scene");*/
}

void EditorLayer::OnAttach()
{
	ourEntityUndoHandler.Initialize(); // has to initialize before creating the first scene

	Firefly::SceneManager::Initialize();

	//load windows from json
	std::string userSettingsToLoad = "Editor/Templates/DefaultUserSettings.json";
	if (std::filesystem::exists("User/UserSettings.json"))
	{
		userSettingsToLoad = "User/UserSettings.json";
	}

	LOGINFO("Loading user settings... (From Path: {})", userSettingsToLoad.c_str());
	std::ifstream i(userSettingsToLoad);
	myEditorSettingsJson = myEditorSettingsJson.parse(i);
	i.close();
	auto jsonWindows = myEditorSettingsJson["Windows"];
	for (int i = 0; i < jsonWindows.size(); i++)
	{
		auto jsonWnd = jsonWindows[i];
		bool openFlag = jsonWnd["OpenFlag"];
		std::string factoryName = jsonWnd["FactoryName"];
		auto window = WindowRegistry::Create(factoryName);
		if (window)
		{
			window->SetOpen(openFlag);
			myEditorWindows.push_back(window);
		}
	}
	GetOrCreateWindow<ConsoleWindow>();

	//DefaultEditorScene Load
	if (myEditorSettingsJson.contains("DefaultEditorScene"))
	{
		auto jsonDefaultEditorScene = myEditorSettingsJson["DefaultEditorScene"];

		if (jsonDefaultEditorScene.is_array())
		{
			if (!jsonDefaultEditorScene.empty())
			{
				auto jsonDes = jsonDefaultEditorScene[0];

				if (jsonDes.contains("DefaultEditorScenePath"))
				{
					myDefaultEditorScenePath = jsonDes["DefaultEditorScenePath"];
				}
			}
		}
	}

	//DefaultSound OnOff
	if (myEditorSettingsJson.contains("DefaultSound"))
	{
		auto jsonDefaultSound = myEditorSettingsJson["DefaultSound"];

		if (jsonDefaultSound.is_array())
		{
			if (!jsonDefaultSound.empty())
			{
				auto jsonDes = jsonDefaultSound[0];

				if (jsonDes.contains("OnOff"))
				{
					isON = jsonDes["OnOff"];
				}
			}
		}
	}

	LoadEditorSettingsEvent loadEvent(myEditorSettingsJson);
	Application::Get().OnEvent(loadEvent);
	LOGINFO("User settings loaded!");

	//Load defaultEditorScene or load new scene
	if (!myDefaultEditorScenePath.empty() && myDefaultEditorScenePath != "NULL" && std::filesystem::exists(myDefaultEditorScenePath))
	{
		Firefly::SceneManager::Get().LoadScene(myDefaultEditorScenePath);
		LOGINFO("User Default Editor Scene loaded!");
	}
	else
	{
		Firefly::SceneManager::Get().NewScene();
		LOGINFO("New Scene loaded!");
	}

	//DefaultLjud OnOff
	AudioManager::StartStop(isON);

	SetEditingScenes(SceneManager::GetCurrentScenes());
	ourEditingScenes[0].lock()->OnRuntimeStart();

	//DebugLogger::QueueCrashFunc([this]() { DoSaveEditorSettings(); }); // this causes problems if the editor crashes when opening a window
}

void EditorLayer::OnDetach()
{

}

void EditorLayer::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	{
		FF_PROFILESCOPE("Editor Layer Editor App Update Event");
		dispatcher.Dispatch<Firefly::EditorAppUpdateEvent>(BIND_EVENT_FN(EditorLayer::OnEditorUpdate));
	}

	dispatcher.Dispatch<Firefly::SceneLoadedEvent>([&](Firefly::SceneLoadedEvent& e)
		{
			DeselectAllEntities();

			if (!e.GetAddFlag())
			{
				ourEditingScenes.clear();
			}
			ourEditingScenes.push_back(e.GetScene());

			ourEntityUndoHandler.ClearUndo();
			return false;
		});

	//unloaded
	dispatcher.Dispatch<Firefly::SceneUnloadedEvent>([&](Firefly::SceneUnloadedEvent& e)
		{
			DeselectAllEntities();
			for (int i = 0; i < ourEditingScenes.size(); i++)
			{
				if (ourEditingScenes[i].lock() == e.GetScene().lock())
				{
					ourEditingScenes.erase(ourEditingScenes.begin() + i);

					break;
				}
			}

			ourEntityUndoHandler.ClearUndo();
			return false;
		});
	dispatcher.Dispatch<PrefabAssetOpenForEditEvent>([&](PrefabAssetOpenForEditEvent& e)
		{
			ourEditingPrefab = Firefly::ResourceCache::GetAsset<Prefab>(e.GetPrefabID(), true);
			ourPrefabScene = std::make_shared<Firefly::Scene>();
			SetEditingScenes({ ourPrefabScene });
			DeselectAllEntities();
			ourEditingScenes[0].lock()->Instantiate(ourEditingPrefab, false);
			for (auto ent : ourEditingScenes[0].lock()->GetEntities())
			{
				for (auto& prefabEnt : ourEditingPrefab->GetEntities())
				{
					if (ent.lock()->GetCorrespondingSourceID() == prefabEnt->GetID())
					{
						ent.lock()->SetIDUnsafe(prefabEnt->GetID());
						break;
					}
				}
			}
			ourEditingScenes[0].lock()->SetAsModified(false);
			ourEditingScenes[0].lock()->SetLoaded(true);
			ourEditingScenes[0].lock()->SetInitialized(true);
			return false;
		});

	dispatcher.Dispatch<ShaderCompileErrorEvent>([&](ShaderCompileErrorEvent& e)
		{
			if (!e.DidCompile())
			{
				ImGuiUtils::NotifyError(e.GetErrorPtr());
				return false;
			}
			ImGuiUtils::NotifySuccess("Shader compiled");
			return false;
		});

	dispatcher.Dispatch<PrefabEditorBackButtonPressedEvent>([&](PrefabEditorBackButtonPressedEvent& e)
		{
			if (IsAnyEditSceneModified() && ourEditingPrefab)
			{
				UnsavedChangesInPrefabModal(
					[this]()
					{
						ourEditingPrefab = nullptr;
						ourEditingScenes = SceneManager::GetCurrentScenes();
						DeselectAllEntities();
					},
					[]() {});
			}
			else
			{
				ourEditingPrefab = nullptr;
				ourEditingScenes = SceneManager::GetCurrentScenes();
				ourPrefabScene = nullptr;
				DeselectAllEntities();
			}

			return false;
		});

	dispatcher.Dispatch< EditorLoadSceneEvent>([&](EditorLoadSceneEvent& e)
		{
			ChangeSceneModal([e]()
				{
					SceneManager::Get().LoadScene(e.GetPath());
				});

			return false;
		});

	for (int i = 0; i < ourEditingScenes.size(); i++)
	{
		if (ourEditingScenes[i].expired())
		{
			LOGERROR("EditorLayer::OnEvent: Scene expired");
			continue;
		}
		ourEditingScenes[i].lock()->OnEvent(aEvent);
	}

	{
		FF_PROFILESCOPE("Editor Windows Events");

		for (auto& windows : myEditorWindows)
		{
			// runs even when not open on previously opened windows, 
			//Save and load event currently depends on this beiong the case,
			//if you need to change this, make sure to change the save and load event as well
			windows->OnEvent(aEvent);
		}
	}
}

void EditorLayer::WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < myEditorWindows.size(); i++)
	{
		myEditorWindows[i]->WindowsMessages(message, wParam, lParam);
	}

	//Release mouse, show mouse and enable input on tab-out
	if (message == WM_KILLFOCUS)
	{
		Utils::InputHandler::ReleaseMouse();
		Utils::InputHandler::ShowMouseCursor();
		SetImGuiInputEnabled(true);
	}

	if (message == WM_CLOSE)
	{
		DoSaveEditorSettings();
	}
}

const std::vector<Ptr<Firefly::Entity>>& EditorLayer::GetSelectedEntities()
{
	for (int i = ourSelectedEntities.size() - 1; i >= 0; --i)
	{
		if (ourSelectedEntities[i].expired())
		{
			LOGWARNING("EditorLayer::GetSelectedEntities: A selected entity was expired, removing from selected entities. It will still exist in the u-map which might be a problem");
			ourSelectedEntities.erase(ourSelectedEntities.begin() + i);
		}
	}

	return ourSelectedEntities;
}

const Ptr<Firefly::Entity>& EditorLayer::GetFirstSelectedEntity()
{
	return GetSelectedEntities()[0];
}

const Ptr<Firefly::Entity>& EditorLayer::GetLastSelectedEntity()
{
	return GetSelectedEntities().back();
}

bool EditorLayer::IsEntitySelected(const Ptr<Firefly::Entity>& anEntity)
{
	return !anEntity.expired() && IsEntitySelected(anEntity.lock()->GetID());
}

bool EditorLayer::IsEntitySelected(uint64_t anID)
{
	const bool selected = ourSelectedEntitiesMap.contains(anID);
	return selected && !ourSelectedEntitiesMap[anID].expired();
}

const Ptr<Firefly::Entity>& EditorLayer::GetSelectedEntityById(uint64_t anID)
{
	if (IsEntitySelected(anID))
	{
		return ourSelectedEntitiesMap[anID];
	}

	return {};
}

std::vector<Ptr<Firefly::Scene>>& EditorLayer::GetEditingScenes()
{
	return ourEditingScenes;
}

std::shared_ptr<Firefly::Prefab>& EditorLayer::GetEditingPrefab()
{
	return ourEditingPrefab;
}

void EditorLayer::ExecuteAndAddEntityUndo(const Ref<UndoCommand>& aCommand, bool aOverrideFocusRequirement)
{
	if (IsCoreWindowsFocused() || aOverrideFocusRequirement)
	{
		ourEntityUndoHandler.ExecuteAndAdd(aCommand);
	}
}

void EditorLayer::AddEntityUndo(const Ref<UndoCommand>& aCommand, bool aOverrideFocusRequirement)
{
	if (IsCoreWindowsFocused() || aOverrideFocusRequirement)
	{
		ourEntityUndoHandler.AddUndo(aCommand);
	}
}

void EditorLayer::ClearAllEntityUndos(bool aOverrideFocusRequirement)
{
	if (IsCoreWindowsFocused() || aOverrideFocusRequirement)
	{
		ourEntityUndoHandler.ClearUndo();
	}
}

void EditorLayer::BeginEntityUndoSeries(bool aOverrideFocusRequirement)
{
	if (IsCoreWindowsFocused() || aOverrideFocusRequirement)
	{
		ourEntityUndoHandler.BeginSeries();
	}
}

void EditorLayer::EndEntityUndoSeries(bool aOverrideFocusRequirement)
{
	if (IsCoreWindowsFocused() || aOverrideFocusRequirement)
	{
		ourEntityUndoHandler.EndSeries();
	}
}

bool EditorLayer::IsHierarchyFocused()
{
	return GetWindow<HierarchyWindow>()->IsFocused();
}

bool EditorLayer::IsCoreWindowsFocused()
{
	return GetWindow<ViewportWindow>()->IsFocused() || GetWindow<HierarchyWindow>()->IsFocused() || GetWindow<InspectorWindow>()->IsFocused();
}

void EditorLayer::TogglePlayMode(bool aPlayFromHere)
{
	Utils::TimerManager::RemoveAllTimers();
	ForceFieldManager::Get().ClearForceFields();

	if (!Application::Get().GetIsInPlayMode())
	{
		OnEnterPlay(aPlayFromHere);
	}
	else
	{
		OnExitPlay();
	}

	DeselectAllEntities();
}

void EditorLayer::OnEnterPlay(bool aPlayFromHere)
{
	if (ourEditingPrefab)
	{
		ImGuiUtils::NotifyError("Cannot enter play mode while editing a prefab");
		return;
	}
	Application::Get().SetIsInPlayMode(true);

	//Cache current editor scene
	ourCachedEditingScenes.clear();
	for (int i = 0; i < ourEditingScenes.size(); i++)
	{
		ourCachedEditingScenes.push_back(ourEditingScenes[i].lock());
	}

	std::vector<Ref<Scene>> scenesToPlay;
	//Construct a play scene from current state of editor scene
	for (int i = 0; i < ourEditingScenes.size(); i++)
	{
		scenesToPlay.push_back(Scene::DeepCopy(ourEditingScenes[i].lock()));
	}

	//Move Editing status to the play scene
	ourEditingScenes.clear();
	for (auto scene : scenesToPlay)
	{
		ourEditingScenes.push_back(scene);
	}

	SceneManager::Get().SetCurrentScenes(scenesToPlay);

	if (aPlayFromHere)
	{
		const auto& playerWeak = GetEntityWithName("Player");

		if (playerWeak.expired())
		{
			LOGWARNING("No player found when trying to Play From Here");
		}
		else
		{
			const auto& player = playerWeak.lock();

			player->GetTransform().SetPosition(Renderer::GetActiveCamera()->GetTransform().GetPosition());
			player->GetTransform().SetRotation(Utils::Quaternion());
		}
	}


	EditorPlayEvent play;
	Application::Get().OnEvent(play);

#ifdef FF_RELEASE // This is cringe don't bring back. :( :( :( :( :( Men LD sa dem ville ha det :( :( :( :( :( L
	//SetImGuiInputEnabled(false);
	//Utils::InputHandler::CaptureMouse();
	//Utils::InputHandler::HideMouseCursor();
#endif

	ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "ENTERED PLAY MODE" });
}

void EditorLayer::OnExitPlay()
{
	EditorStopEvent stop;
	Application::Get().OnEvent(stop);

	Application::Get().SetIsInPlayMode(false);
	ourEditingScenes.clear();
	SceneManager::Get().SetCurrentScenes(ourCachedEditingScenes);
	ourCachedEditingScenes.clear();

	SetImGuiInputEnabled(true);
	Utils::InputHandler::ReleaseMouse();
	Utils::InputHandler::ShowMouseCursor();
	Utils::Timer::SetTimeScale(1);

	ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "ENTERED EDITOR MODE" });
}

void EditorLayer::SelectEntity(Ptr<Firefly::Entity> aEntity, bool aAddSelected)
{
	if (aEntity.expired())
	{
		LOGERROR("EditorLayer::SelectEntity: Tried selecting/deselecting a deleted Entity");
		return;
	}

	const auto& entity = aEntity.lock();

	if (aAddSelected)
	{
		if (!ourSelectedEntitiesMap.contains(entity->GetID()))
		{
			ourSelectedEntities.push_back(aEntity);
			ourSelectedEntitiesMap.insert({ entity->GetID(), aEntity });
			SetShouldOutline(aEntity, true);
		}
		else
		{
			auto it = std::find_if(ourSelectedEntities.begin(), ourSelectedEntities.end(), [aEntity](const Ptr<Firefly::Entity>& aSelectedEntity)
				{
					return aSelectedEntity.lock()->GetID() == aEntity.lock()->GetID();
				});

			ourSelectedEntities.erase(it);
			ourSelectedEntitiesMap.erase(entity->GetID());
			SetShouldOutline(aEntity, false);
		}
	}
	else
	{
		DeselectAllEntities();
		ourSelectedEntities.push_back(aEntity);
		ourSelectedEntitiesMap.insert({ entity->GetID(), aEntity });
		SetShouldOutline(aEntity, true);
	}

	EntitySelectedEvent ev;
	Application::Get().OnEvent(ev);
}

void EditorLayer::AddSelectedEntity(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("EditorLayer::AddSelectedEntity: Tried to select a deleted entity");
		return;
	}

	const auto& entity = aEntity.lock();

	if (!ourSelectedEntitiesMap.contains(entity->GetID()))
	{
		ourSelectedEntities.push_back(aEntity);
		ourSelectedEntitiesMap.insert({ entity->GetID(), aEntity });
		SetShouldOutline(aEntity, true);
	}

	EntitySelectedEvent ev;
	Application::Get().OnEvent(ev);
}

bool EditorLayer::DeselectEntity(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("EditorLayer::DeselectEntity: Tried to deselect a deleted entity");
		return false;
	}

	const auto& entity = aEntity.lock();

	if (ourSelectedEntitiesMap.contains(entity->GetID()))
	{
		auto it = std::find_if(ourSelectedEntities.begin(), ourSelectedEntities.end(),
			[aEntity](Ptr<Firefly::Entity>& aEntityToCheck) { return aEntityToCheck.lock()->GetID() == aEntity.lock()->GetID(); });

		ourSelectedEntities.erase(it);
		ourSelectedEntitiesMap.erase(aEntity.lock()->GetID());
		SetShouldOutline(aEntity, false);

		return true;
	}
	else
	{
		LOGWARNING("EditorLayer::DeselectEntity: Tried to deselect entity that wasn't kept track of");
		return false;
	}
}

void EditorLayer::DeselectAllEntities()
{
	for (const auto& selectedEntity : ourSelectedEntities)
	{
		SetShouldOutline(selectedEntity, false);
	}
	ourSelectedEntities.clear();
	ourSelectedEntitiesMap.clear();
}

void EditorLayer::DeselectAllEntitiesCommand()
{
	if (!ourSelectedEntities.empty())
	{
		const auto deselect = CreateRef<EntityDeselectAllCommand>();
		ExecuteAndAddEntityUndo(deselect);
	}
}

void EditorLayer::DeselectEntitiesWithSelectedParents()
{
	std::vector<Ptr<Entity>> toDeselect;
	const auto& selected = GetSelectedEntities();
	for (const auto& entity : selected)
	{
		if (entity.expired())
		{
			LOGERROR("EditorLayer::DeselectEntitiesWithSelectedParents: Entity is expired");
			continue;
		}
		const auto& children = entity.lock()->GetChildren();
		for (const auto& child : children)
		{
			toDeselect.push_back(child);
		}
	}

	for (const auto& entToDeselect : toDeselect)
	{
		DeselectEntity(entToDeselect);
		SetShouldOutline(entToDeselect, true);
	}
}

void EditorLayer::RemoveOutlineFromSelectedChildren()
{
	//Manual deselect because we have outline entities that are not selected
	const auto& selected = GetSelectedEntities();
	for (const auto& entity : selected)
	{
		const auto& children = entity.lock()->GetChildren();
		for (const auto& child : children)
		{
			SetShouldOutline(child, false);
		}
	}
}

void EditorLayer::DeleteSelectedEntities()
{
	for (auto& entityWeak : ourSelectedEntities)
	{
		auto entity = entityWeak.lock();
		//if thew entity we are trying to delete belongs to a prefab and is not the root of the prefab, ane we are not editing the prefab
		if (entity->IsPrefab() && entity->GetPrefabRootEntityID() != entity->GetID() && !ourEditingPrefab)
		{
			ImGuiUtils::OpenYesNoPopupModal("You cannot delete the child of a prefab locally!",
				"To delete a child from the prefab you have to open the prefab in the prefab editor. Do you want to open the prefab in the prefab editor?",
				[entity]()
				{
					//Open the prefab in the prefab editor
					PrefabAssetOpenForEditEvent openPrefabEvent(entity->GetPrefabRoot().lock()->GetPrefabID());
					Application::Get().OnEvent(openPrefabEvent);
				},
				[]()
				{
					//Do nothing
				});
			return;
		}
	}

	bool begunSeries = false;

	if (ourSelectedEntities.size() > 1)
	{
		ourEntityUndoHandler.BeginSeries();
		begunSeries = true;
	}

	for (auto& entity : ourSelectedEntities)
	{
		//dont delete the root if editing a prefab
		if (ourEditingPrefab && ourEditingScenes[0].lock()->GetEntities()[0].lock()->GetID() == entity.lock()->GetID())
		{
			ImGuiUtils::NotifyError("You are not allowed to delete the root entity of a prefab!");
		}
		else
		{
			if (entity.expired())
			{
				LOGERROR("EditorLayer::DeleteSelectedEntities: Tried to delete an expired Entity");
				continue;
			}

			ourEntityUndoHandler.ExecuteAndAdd(std::make_shared<EntityDeleteCommand>(entity));
		}
	}

	DeselectAllEntitiesCommand();

	if (begunSeries)
	{
		ourEntityUndoHandler.EndSeries();
	}
}

void EditorLayer::DuplicateEntities(const std::vector<Ptr<Firefly::Entity>>& someEntities, UndoHandler& aUndoHandler)
{
	for (auto& entWeak : someEntities)
	{
		auto ent = entWeak.lock();
		//if the entity we are trying to delete belongs to a prefab and is not the root of the prefab, and we are not editing the prefab
		if (ent->IsPrefab() && ent->GetPrefabRootEntityID() != ent->GetID() && !ourEditingPrefab)
		{
			ImGuiUtils::NotifyError("You are not allowed to duplicate a non-root prefab instance!");
			return;
		}
		//if in prefab edit mode and the entity is the root of the prefab
		if (ourEditingPrefab && ourEditingScenes[0].lock()->GetEntities()[0].lock()->GetID() == ent->GetID())
		{
			ImGuiUtils::NotifyError("You are not allowed to duplicate the root of a prefab while in the prefab editor!");
			return;
		}
	}

	bool startedSeries = false;
	std::vector<Ptr<Firefly::Entity>> newEntities;
	if (someEntities.size() > 1)
	{
		startedSeries = true;
		aUndoHandler.BeginSeries();
	}

	for (auto entity : someEntities)
	{
		auto cmd = std::make_shared<EntityDuplicateCommand>(entity, true);
		aUndoHandler.ExecuteAndAdd(cmd);
		newEntities.push_back(cmd->GetDuplicatedEntity());
	}

	if (startedSeries)
	{
		DeselectAllEntities();
		for (const auto& entity : newEntities)
		{
			AddSelectedEntity(entity);
		}
		aUndoHandler.EndSeries();
	}
}

void EditorLayer::DuplicateSelectedEntities()
{
	DuplicateEntities(ourSelectedEntities, ourEntityUndoHandler);
}

void EditorLayer::CreateEntityFolderForSelectedEntities()
{
	const auto& selectedEntities = EditorLayer::GetSelectedEntities();

	std::set<uint64_t> parentIDs{};
	bool prefabChildSelected = false;
	Utils::Vector3f averagePosition;

	for (const auto& entity : selectedEntities)
	{
		if (!entity.expired())
		{
			const auto& ent = entity.lock();

			//Check if prefab child, don't allow moving it to empty folder
			if (ent->IsPrefab() && !ent->IsPrefabRoot())
			{
				ImGuiUtils::NotifyWarning("Cannot create entity folder including prefab children, they belong to the prefab.");
				prefabChildSelected = true;
				break;
			}

			parentIDs.insert(ent->GetParentID());

			averagePosition += ent->GetTransform().GetPosition();
		}
		else
		{
			LOGERROR("EditorLayer::CreateEntityFolderForSelectedEntities: Error while trying to access a selected entity");
		}
	}

	averagePosition /= static_cast<float>(selectedEntities.size());

	if (prefabChildSelected)
	{
		LOGWARNING("Tried to create empty folder with prefab child selected");
	}
	else
	{
		Ptr<Firefly::Entity> targetParent{};
		Firefly::Scene* targetScene = selectedEntities.back().lock()->GetParentScene();

		if (!parentIDs.empty())
		{
			targetParent = GetEntityWithID(*parentIDs.begin());

			if (!targetParent.expired())
			{
				targetScene = targetParent.lock()->GetParentScene();
			}
		}

		ourEntityUndoHandler.BeginSeries();
		const auto createCommand = CreateRef<EntityCreateCommand>(targetParent, targetScene, false, false);
		ourEntityUndoHandler.ExecuteAndAdd(createCommand);

		const auto& newEntityFolderWeak = createCommand->GetCreatedEntity();

		if (newEntityFolderWeak.expired())
		{
			LOGERROR("EditorLayer::CreateEntityFolderForSelectedEntities: Error while trying to access created entity folder");
			return;
		}

		const auto& entityFolder = newEntityFolderWeak.lock();
		entityFolder->GetTransform().SetPosition(averagePosition);

		const auto& targetSceneEntities = entityFolder->GetParentScene()->GetEntities();
		auto copyOfSelected = selectedEntities;

		for (int i = 0; i < copyOfSelected.size(); ++i)
		{
			if (HasSelectedParentRecursive(copyOfSelected[i]))
			{
				copyOfSelected.erase(copyOfSelected.begin() + i);
				i--;
			}
		}

		std::sort(copyOfSelected.begin(), copyOfSelected.end(),
			[=](Ptr<Firefly::Entity> a, Ptr<Firefly::Entity> b)
			{
				int aIndex = std::find_if(targetSceneEntities.begin(), targetSceneEntities.end(), [a](Ptr<Firefly::Entity> aEntityToCompare) { return a.lock()->GetID() == aEntityToCompare.lock()->GetID(); }) - targetSceneEntities.begin();
				int bIndex = std::find_if(targetSceneEntities.begin(), targetSceneEntities.end(), [b](Ptr<Firefly::Entity> aEntityToCompare) { return b.lock()->GetID() == aEntityToCompare.lock()->GetID(); }) - targetSceneEntities.begin();
				return aIndex < bIndex;
			});

		for (const auto& selected : copyOfSelected)
		{
			GetWindow<HierarchyWindow>()->OnDropEntityOnEntity(selected, entityFolder);
		}

		ourEntityUndoHandler.EndSeries();

		SelectEntity(newEntityFolderWeak);
		GetWindow<HierarchyWindow>()->ScrollToSelected();
	}
}

void EditorLayer::ToggleHideEntityCommand(const Ptr<Firefly::Entity>& aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("Entity expired");
		return;
	}

	const auto hideCmd = CreateRef<EntityHideCommand>(aEntity, aEntity.lock()->GetIsActive());
	ExecuteAndAddEntityUndo(hideCmd);
}

void EditorLayer::HideEntityCommand(const Ptr<Firefly::Entity>& aEntity, bool aHide)
{
	if (aEntity.expired())
	{
		LOGERROR("Entity expired");
		return;
	}

	const auto hideCmd = CreateRef<EntityHideCommand>(aEntity, aHide);
	ExecuteAndAddEntityUndo(hideCmd);
}

void EditorLayer::RenameEntity(const std::vector<Ptr<Firefly::Entity>>& someEntities)
{

	for (auto& entWeak : someEntities)
	{
		auto ent = entWeak.lock();
		myRenameBuffer = ent->GetName();


		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			LOGINFO("Start rename In hierarky");
			return;
		}

		ImGui::SetKeyboardFocusHere();
		bool renamed = ImGui::InputText("##renamingHierarcy", &myRenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
		renamed = renamed || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemHovered();
		LOGINFO("Start rename In hierarky= {}:", renamed);

		if (renamed)
		{
			LOGINFO("Done renaming In hierarky Name= {}:", myRenameBuffer);
			ent->SetName(myRenameBuffer);
			/*auto newPath = aEntry.Path.parent_path().string() + "\\" + myRenameBuffer + aEntry.Extension;
			if (!std::filesystem::exists(newPath))
			{
				std::filesystem::rename(aEntry.Path, newPath);
				RegenerateEntries();
			}
			else
			{
				ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Entry with path:\"%s\"Already exists, entry was not renamed",newPath.c_str() });
			}
			EndRename(newPath);*/
		}
	}
}

void EditorLayer::RenameSelectedEntity()
{
	RenameEntity(ourSelectedEntities);
}

void EditorLayer::SetEditingScenes(std::vector<Ptr<Firefly::Scene>> someScenes)
{
	ourEditingScenes.clear();
	ourEditingScenes = someScenes;
}

bool EditorLayer::IsAnyEditSceneModified()
{
	for (int i = 0; i < ourEditingScenes.size(); i++)
	{
		if (ourEditingScenes[i].lock()->IsModified())
		{
			return true;
		}
	}
	return false;
}

void EditorLayer::SaveScene(Ptr<Firefly::Scene> aScene)
{
	if (aScene.expired())
	{
		LOGERROR("EditorLayer::SaveScene: Scene is expired");
		return;
	}
	auto scene = aScene.lock();
	if (scene->GetPath() == "")
	{
		SaveSceneAs(aScene);
		return;
	}
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't save a scene while in play mode! " });
		//AudioManager::PlayEventOneShot(EVENT_Editor_Error);
		return;
	}
	bool hasWritePermission = (std::filesystem::status(scene->GetPath()).permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

	if (!hasWritePermission)
	{
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Could not save scene at path:\n\"%s\"\nMake sure the file is checked out in Perforce!", scene->GetPath().string().c_str()});
	}
	else
	{
		nlohmann::json saveFile;
		SerializationUtils::SerializeScene(saveFile, scene);

		auto path = scene->GetPath();
		std::ofstream file(path);
		file << std::setw(4) << saveFile << std::endl;
		file.close();

		GetWindow<ContentBrowser>()->RegenerateEntries();

		ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Scene successfully saved to path:\n\"%s\"", path.string().c_str() });
		//AudioManager::PlayEventOneShot(EVENT_Editor_Success);
		scene->SetAsModified(false);
	}

}

void EditorLayer::SaveSceneAs(Ptr<Firefly::Scene> aScene)
{
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't save a scene while in play mode! " });
		//play error sound
		AudioManager::PlayEventOneShot(EVENT_Editor_Error);
		return;
	}
	auto scenePath = EditorUtils::GetSaveFilePath("Firefly Scene (*.scene)\0*.scene\0", "scene");

	if (scenePath != "")
	{
		aScene.lock()->SetPath(scenePath);
		SaveScene(aScene);
	}
}

void EditorLayer::UnloadScene(Ptr<Firefly::Scene> aScene)
{
	if (aScene.expired())
	{
		LOGERROR("EditorLayer::UnloadScene: Scene is expired");
		return;
	}
	if (aScene.lock()->IsModified())
	{
		ImGuiUtils::OpenYesNoPopupModal("Scene has unsaved Changes! Discard Changes?",
			"Do you want to discard the changes?",
			[aScene]()
			{
				auto it = std::find_if(ourEditingScenes.begin(), ourEditingScenes.end(), [aScene](const Ptr<Firefly::Scene>& scene) { if (!aScene.expired() && !scene.expired()) return scene.lock() == aScene.lock(); return false; });
				if (it != ourEditingScenes.end())
				{
					const auto path = (*it).lock()->GetPath();
					ourEditingScenes.erase(it);
					SceneManager::Get().UnloadScene(path);
				}
				DeselectAllEntities();
			},
			[aScene]()
			{
			});
	}
	else
	{
		auto it = std::find_if(ourEditingScenes.begin(), ourEditingScenes.end(), [aScene](const Ptr<Firefly::Scene>& scene) { return scene.lock() == aScene.lock(); });
		if (it != ourEditingScenes.end())
		{
			SceneManager::Get().UnloadScene((*it).lock()->GetPath());
			ourEditingScenes.erase(it);
		}
		DeselectAllEntities();
	}

}

bool EditorLayer::OnEditorUpdate(Firefly::EditorAppUpdateEvent& aEvent)
{
	if (GetWindow<ViewportWindow>()->IsFocused() || GetWindow<HierarchyWindow>()->IsFocused() || GetWindow<InspectorWindow>()->IsFocused())
	{
		ourEntityUndoHandler.Update();
	}

	if (ourEditingPrefab)
	{
		Firefly::DirLightPacket pack{};
		pack.Direction = { -0.71, 0.71, -0.71, 1 };
		pack.ColorAndIntensity = { 1,1,1,1 };
		pack.dirLightInfo.x = 0;
		Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res1024);
	}


	auto dockspaceID = ImGui::GetID("DockSpaceMain");
	auto viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
	host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	host_window_flags |= ImGuiWindowFlags_NoBackground;

	char label[32];
	ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_Main", viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin(label, NULL, host_window_flags))
	{
		if (!ImGui::DockBuilderGetNode(dockspaceID))
		{
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());

			auto dockMainID = dockspaceID;
			auto dockInspectorID = ImGui::DockBuilderSplitNode(dockMainID,
				ImGuiDir_Right, 0.2f, nullptr, &dockMainID);
			auto dockContentBrowserID = ImGui::DockBuilderSplitNode(dockMainID,
				ImGuiDir_Down, 0.4f, nullptr, &dockMainID);
			auto dockSceneHierarchyID = ImGui::DockBuilderSplitNode(dockMainID,
				ImGuiDir_Left, 0.3f, nullptr, &dockMainID);
			auto consoleID = ImGui::DockBuilderSplitNode(dockContentBrowserID,
				ImGuiDir_Right, 0.5f, nullptr, &dockContentBrowserID);

			ImGui::DockBuilderGetNode(dockMainID)->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar; //Auto-hide viewport tab bar

			ImGui::DockBuilderDockWindow(GetWindow<ContentBrowser>()->GetIDString().c_str(), dockContentBrowserID);
			ImGui::DockBuilderDockWindow(GetWindow<ConsoleWindow>()->GetIDString().c_str(), consoleID);
			ImGui::DockBuilderDockWindow(GetWindow<HierarchyWindow>()->GetIDString().c_str(), dockSceneHierarchyID);
			ImGui::DockBuilderDockWindow(GetWindow<InspectorWindow>()->GetIDString().c_str(), dockInspectorID);
			ImGui::DockBuilderDockWindow(GetWindow<ViewportWindow>()->GetIDString().c_str(), dockMainID);

			ImGui::DockBuilderFinish(dockspaceID);
		}

		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();
	}
	ImGui::PopStyleVar(3);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl + N (SHORTCUT NOT IMPLEMENTED)"))
			{
				NewScene();
			}

			if (ImGui::MenuItem("Save Scene", "Ctrl + S"))
			{
				SaveScenes();
			}

			if (ImGui::MenuItem("Save Scene As", "Ctrl + Shift + S"))
			{
				SaveCurrentSceneAs();
			}

			if (ImGui::MenuItem("Load Scene", "Ctrl + O"))
			{
				LoadScene();
			}

			if (ourEditingPrefab)
			{
				if (ImGui::MenuItem("Save Prefab"))
				{
					SavePrefab();
				}
			}

			if (ImGui::MenuItem("Build Project", "Ctrl + B"))
			{
				BuildProject();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Recompile all shaders"))
			{
				myNumberOfCompiledShaders = 0;
				Firefly::ShaderLibrary::ReCompileAllShaders(&myNumberOfCompiledShaders, &myTotalNumberToCompiledShaders, &myCurrentCompilingShader);
				myIsCompilingShaders = true;
			}
			if (ImGui::MenuItem("Fabian Collider Button"))
			{
				OnClickFabianColliderButton();
			}
			if (ImGui::MenuItem("Compile CloudShadow postprocess"))
			{
				Firefly::ShaderLibrary::Recompile("CloudShadow");
			}

			static float editorTimeScale = 1.0f;
			ImGui::DragFloat("Time Scale", &editorTimeScale, 0.01f, 0.001f, FLT_MAX);
			Utils::Timer::SetTimeScale(editorTimeScale);

			/*static float beginPlayDelay = 5.0f;
			ImGui::DragFloat("Script Begin Play Delay", &beginPlayDelay);
			VisualScriptComponent::SetBeginPlayDelay(beginPlayDelay);*/

			//TurnOffOnAudio
			if (ImGui::MenuItem("TurnOffOnAudio"))
			{
				isON = !isON;
				AudioManager::StartStop(isON);
			}

			//DefaultEditorScene
			if (ImGui::MenuItem("DefaultEditorScene"))
			{

				if (Firefly::Application::Get().GetIsInPlayMode())
				{
					ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't Set a Default Editor Scene while in play mode! " });
					//play error sound
					AudioManager::PlayEventOneShot(EVENT_Editor_Error);
				}
				else
				{
					auto scenePath = EditorUtils::GetOpenFilePath("Firefly Scene (*.scene)\0*.scene\0");
					if (scenePath != "")
					{
						auto myDefaultEditorSceneFileSystemPath = scenePath;
						myDefaultEditorScenePath = myDefaultEditorSceneFileSystemPath.generic_string();
						Firefly::SceneManager::Get().LoadScene(myDefaultEditorScenePath);
						LOGINFO("User Default Editor Scene loaded!");
					}
				}
			}

			ImGui::EndMenu();
		}

		CreateWindowsImGuiMenu();

		if (myIsCompilingShaders)
		{
			const float fraction = static_cast<float>(myNumberOfCompiledShaders) / static_cast<float>(myTotalNumberToCompiledShaders);
			const std::string shaderLabel = "Current shader: " + myCurrentCompilingShader + " [" + std::to_string(myNumberOfCompiledShaders) + "/" + std::to_string(myTotalNumberToCompiledShaders) + "]";
			ImGui::ProgressBar(fraction, ImVec2(400, 0), shaderLabel.c_str());
			if (myNumberOfCompiledShaders >= myTotalNumberToCompiledShaders)
			{
				myIsCompilingShaders = false;
			}
		}
		ImGui::EndMainMenuBar();
	}

	if (myBuildingProjectSetting || myIsBuildingProject)
	{
		UpdateBuildProjectImGui();
	}

	InputShortcuts();

	ImGuiUtils::UpdateYesNoPopupModal();

	{
		FF_PROFILESCOPE("Editor Windows ImGui Update");

		for (int i = 0; i < myEditorWindows.size(); ++i)
		{
			if (myEditorWindows[i]->IsOpen())
			{
				myEditorWindows[i]->OnImGuiUpdate();
			}
		}
	}

#ifndef FF_SHIPIT
	//if holding C + R + A + S+ H at the same time
	if (Utils::InputHandler::GetKeyHeld('C') &&
		Utils::InputHandler::GetKeyHeld('R') &&
		Utils::InputHandler::GetKeyHeld('A') &&
		Utils::InputHandler::GetKeyHeld('S') &&
		Utils::InputHandler::GetKeyHeld('H'))
	{
		int* ss = nullptr;
		*ss = 1337;
	}

#endif

	return false;
}

void EditorLayer::CreateWindowsImGuiMenu()
{
	if (ImGui::BeginMenu("Windows"))
	{
		auto& map = WindowRegistry::GetFactoryMap();
		for (auto windowEntry : map)
		{
			auto windowFactoryName = windowEntry.first;
			if (ImGui::MenuItem(windowFactoryName.c_str()))
			{
				auto windows = GetWindows(windowFactoryName);
				bool anyClosed = false;

				if (windows.size() > 0)
				{
					for (int i = 0; i < windows.size(); i++)
					{
						if (!windows[i]->IsOpen())
						{
							windows[i]->SetOpen(true);
							anyClosed = true;
							break;
						}
					}
				}

				if (!anyClosed && windows.size() == 0 || windowFactoryName == ContentBrowser::GetFactoryName())
				{
					CreateEditorWindow(windowFactoryName);
				}
			}
		}
		ImGui::EndMenu();
	}
}

std::shared_ptr<EditorWindow> EditorLayer::GetWindow(std::string aName)
{
	//find window with name
	for (int i = 0; i < myEditorWindows.size(); ++i)
	{
		if (myEditorWindows[i]->GetName() == aName)
		{
			return myEditorWindows[i];
		}
	}
	return std::shared_ptr<EditorWindow>();
}

std::vector<Ref<EditorWindow>> EditorLayer::GetWindows(std::string aName)
{
	std::vector<Ref<EditorWindow>> windows;
	for (int i = 0; i < myEditorWindows.size(); ++i)
	{
		if (myEditorWindows[i]->GetName() == aName)
		{
			windows.push_back(myEditorWindows[i]);
		}
	}
	return windows;
}

void EditorLayer::DoSaveEditorSettings()
{
	auto& jsonWindows = myEditorSettingsJson["Windows"];
	//loop through and save open state of all windows
	for (int i = 0; i < myEditorWindows.size(); i++)
	{
		auto& jsonWnd = jsonWindows[i];
		jsonWnd["OpenFlag"] = myEditorWindows[i]->IsOpen();
		jsonWnd["FactoryName"] = myEditorWindows[i]->GetName();
	}

	//DefaultEditorScene Save
	auto& jsonDefaultEditorScene = myEditorSettingsJson["DefaultEditorScene"];
	auto& jsonDes = jsonDefaultEditorScene[0];
	jsonDes["DefaultEditorScenePath"] = myDefaultEditorScenePath;

	//DefaultAudio OnOff
	auto& jsonDefaultAudio = myEditorSettingsJson["DefaultSound"];
	auto& jsonDa = jsonDefaultAudio[0];
	jsonDa["OnOff"] = isON;

	SaveEditorSettingsEvent saveEvent(myEditorSettingsJson);
	OnEvent(saveEvent);

	if (!std::filesystem::exists("User"))
	{
		std::filesystem::create_directory("User");
	}

	//save json to file
	std::ofstream o("User/UserSettings.json");
	o << myEditorSettingsJson;
	o.close();
}

void EditorLayer::SaveCurrentScenes()
{
	for (int i = 0; i < SceneManager::GetCurrentScenes().size(); i++)
	{
		SaveScene(SceneManager::GetCurrentScenes()[i]);
	}
}

void EditorLayer::OnClickFabianColliderButton()
{
	if (ourSelectedEntities.size() > 0)
	{
		for (int i = 0; i < ourSelectedEntities.size(); i++)
		{

			if (ourSelectedEntities[i].lock()->HasComponent<Firefly::MeshComponent>())
			{
				auto mesh = ourSelectedEntities[i].lock()->GetComponent<Firefly::MeshComponent>().lock()->GetMesh();

				Utils::Vector3f min = Utils::Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
				Utils::Vector3f max = Utils::Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
				for (auto& submesh : mesh->GetSubMeshes())
				{
					for (auto& vert : submesh.GetVerticesPositions())
					{
						min.x = std::min(min.x, vert.x);
						min.y = std::min(min.y, vert.y);
						min.z = std::min(min.z, vert.z);

						max.x = std::max(max.x, vert.x);
						max.y = std::max(max.y, vert.y);
						max.z = std::max(max.z, vert.z);
					}
				}

				auto collider = std::reinterpret_pointer_cast<Firefly::BoxColliderComponent>(
					Firefly::ComponentRegistry::Create(Firefly::BoxColliderComponent::GetFactoryName()));
				collider->SetWithMinMax(min, max);

				auto rigidbody = std::reinterpret_pointer_cast<Firefly::RigidbodyComponent>(
					Firefly::ComponentRegistry::Create(Firefly::RigidbodyComponent::GetFactoryName()));
				rigidbody->SetStatic(true);

				auto newEnt = ourEditingScenes.front().lock()->Instantiate().lock();
				newEnt->AddComponent(collider, true);
				newEnt->AddComponent(rigidbody, true);

				newEnt->SetParent(ourSelectedEntities[i].lock(), -1, true, false);
			}
			else if (ourSelectedEntities[i].lock()->HasComponent<Firefly::AnimatedMeshComponent>())
			{
				auto comp = ourSelectedEntities[i].lock()->GetComponent<Firefly::AnimatedMeshComponent>().lock();
				auto mesh = comp->GetMesh();

				Utils::Vector3f min = Utils::Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
				Utils::Vector3f max = Utils::Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
				int submeshIndex = 0;
				for (auto& submesh : mesh->GetSubMeshes())
				{
					auto vec = comp->GetOnlyRenderOneSubMeshIndices();
					if (!vec.empty())
					{
						auto it = std::find(vec.begin(), vec.end(), submeshIndex);
						if (it == vec.end())
						{
							submeshIndex++;
							continue;
						}
					}
					for (auto& vert : submesh.GetVerticesPositions())
					{
						min.x = std::min(min.x, vert.x);
						min.y = std::min(min.y, vert.y);
						min.z = std::min(min.z, vert.z);

						max.x = std::max(max.x, vert.x);
						max.y = std::max(max.y, vert.y);
						max.z = std::max(max.z, vert.z);
					}
					submeshIndex++;

				}

				auto collider = std::reinterpret_pointer_cast<Firefly::BoxColliderComponent>(
					Firefly::ComponentRegistry::Create(Firefly::BoxColliderComponent::GetFactoryName()));
				collider->SetWithMinMax(min, max);

				auto rigidbody = std::reinterpret_pointer_cast<Firefly::RigidbodyComponent>(
					Firefly::ComponentRegistry::Create(Firefly::RigidbodyComponent::GetFactoryName()));

				ourSelectedEntities[i].lock()->AddComponent(collider, true);
				ourSelectedEntities[i].lock()->AddComponent(rigidbody, true);
			}
		}
	}
}

void EditorLayer::DoSave()
{
	if (ourEditingPrefab)
	{
		//save prefab
		SavePrefab();
	}
	else
	{
		//save scene
		SaveScenes();
	}
}

void EditorLayer::DoSaveAs()
{
	if (ourEditingPrefab)
	{
		//Cant save prefab as
	}
	else
	{
		//save scene
		SaveCurrentSceneAs();
	}
}

void EditorLayer::DoLoad()
{
	if (ourEditingPrefab)
	{
		//cant open prefab this way
	}
	else
	{
		//open scene
		LoadScene();
	}
}

void EditorLayer::DoNew()
{
	if (ourEditingPrefab)
	{
		//cant create a new prefab this way
	}
	else
	{
		//new sceneD
		NewScene();
	}
}

void EditorLayer::NewScene()
{
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't create a new scene while in play mode!" });
		return;
	}
	ChangeSceneModal([this]()
		{
			//create a new scene
			Firefly::SceneManager::Get().NewScene();
			//if we are editing a prefab, exit from it
			PrefabEditorBackButtonPressedEvent ev;
			OnEvent(ev);
			//then set the new scene as editing scene
			ourEditingScenes = SceneManager::GetCurrentScenes();

		});
}

void EditorLayer::SaveScenes()
{
	//we only have to check the first scene since we cannot add a new scene to an existing scene
	if (ourEditingScenes[0].lock()->GetPath() == "")
	{
		auto scenePath = EditorUtils::GetSaveFilePath("Firefly Scene (*.scene)\0*.scene\0", "scene");
		ourEditingScenes[0].lock()->SetPath(scenePath);
	}
	SaveCurrentScenes();
}

void EditorLayer::SaveCurrentSceneAs()
{
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't save a scene while in play mode! " });
		//play error sound
		AudioManager::PlayEventOneShot(EVENT_Editor_Error);
		return;
	}

	if (ourEditingScenes.size() > 1)
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't save a scene with a new path this way with multiple scenes open! You have to right click the scene in the hierarchy and press \"Save as\". " });
		//play error sound
		AudioManager::PlayEventOneShot(EVENT_Editor_Error);
		return;
	}

	SaveSceneAs(ourEditingScenes[0]);
}

void EditorLayer::LoadScene()
{
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		ImGui::InsertNotification({ ImGuiToastType_Error,3000, "Can't load a new scene while in play mode! " });
		//play error sound
		AudioManager::PlayEventOneShot(EVENT_Editor_Error);
		return;
	}
	ChangeSceneModal([this]()
		{
			auto scenePath = EditorUtils::GetOpenFilePath("Firefly Scene (*.scene)\0*.scene\0");
			if (scenePath != "")
			{
				Firefly::SceneManager::Get().LoadScene(scenePath);
				PrefabEditorBackButtonPressedEvent ev;
				OnEvent(ev);
				ourEditingScenes = SceneManager::GetCurrentScenes();
			}
		});

}

void EditorLayer::BuildProject()
{
	//save all scenes
	if (ourEditingScenes[0].lock()->GetPath() != "")
	{
		SaveCurrentScenes();
	}

	myBuildingProjectSetting = true;
}

void EditorLayer::StartBuild()
{
	//build project
	auto contentBrowser = GetWindow<ContentBrowser>();

	//compile all fbx

	auto allEntriesToCopyOver = contentBrowser->GetAllEntriesOfTypes(
		{
			EntryType::Animator,
			EntryType::Animation,
			EntryType::Mesh,
			EntryType::Skeleton,
			EntryType::Material,
			EntryType::Texture,
			EntryType::Prefab,
			EntryType::Font,// shouldnt need .ttf files
			EntryType::Emitter,
			EntryType::SoundBank,
			EntryType::BlendSpace,
			EntryType::Pipeline,
			EntryType::AvatarMask,
			EntryType::VisualScript,
			EntryType::GameplayStateMachine,
			EntryType::VoiceLineData
		});

	for (auto& file : allEntriesToCopyOver)
	{
		if (file->Parent->Name == "Primitives")
		{
			continue;
		}

		if (file->Path.string().find("Assets\\TestingStuff\\") != std::string::npos)
		{
			continue;
		}

		myBuildFiles.push_back(file->Path);
	}

	for (int i = 0; i < myScenesToIncludeInBuild.size(); i++)
	{
		myBuildFiles.push_back(myScenesToIncludeInBuild[i]);
	}

	for (auto& entry : std::filesystem::recursive_directory_iterator("FireflyEngine\\"))
	{
		if (entry.path().string().find("FireflyEngine\\Shaders-bin\\") != std::string::npos)
		{
			continue;
		}
		if (!entry.is_directory())
		{
			myBuildFiles.push_back(entry.path().string());
		}
	}

	for (auto& entry : std::filesystem::recursive_directory_iterator("Editor\\CompiledAssets\\"))
	{
		if (!entry.is_directory())
		{
			myBuildFiles.push_back(entry.path().string());
		}
	}

	myBuildFiles.push_back(std::filesystem::current_path().string() + "\\Assets\\Textures\\Icons\\icon_game.ico");


	std::array<std::string, 4> excludeDll = { /*"dpp.dll",*/ "libcrypto-1_1-x64.dll", "libsodium.dll", "libssl-1_1-x64.dll", "opus.dll"/*, "zlib1.dll"*/ };

	for (auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
	{
		if (entry.path().extension() == ".dll")
		{
			/*if (std::find(excludeDll.begin(), excludeDll.end(), entry.path().filename().string()) != excludeDll.end())
			{
				continue;
			}*/
			myBuildFiles.push_back(entry.path().string());
		}
	}

	myShaderKeysToCompileInBuild = Firefly::ShaderLibrary::GetAllShaderKeys();
	//start building

	myIsBuildingProject = true;
	myBuildingProjectSetting = false;
	myBuildOperationsTotalCount = myBuildFiles.size() + myShaderKeysToCompileInBuild.size();
	myBuildOperationsCompletedCount = 0;

	//copy exe file
	auto targetExePath = myBuildPath + myBuildExeName + ".exe";
	std::filesystem::path directoryPath = targetExePath;
	directoryPath = directoryPath.remove_filename();
	if (!std::filesystem::exists(directoryPath))
	{
		std::filesystem::create_directories(directoryPath);
	}
	std::filesystem::copy_file(std::filesystem::current_path().string() + "\\Can Strike.exe", targetExePath, std::filesystem::copy_options::overwrite_existing);

	myBuildIsCompilingShaderKeys = true;

	for (int32_t i = 0; i < myShaderKeysToCompileInBuild.size(); ++i)
	{
		myCurrentBuildOperationsText.push_back("Compiling shader with key:" + myShaderKeysToCompileInBuild[i]);
		Firefly::ShaderLibrary::Recompile(myShaderKeysToCompileInBuild[i]);
		myBuildOperationsCompletedCount++;
		myCurrentBuildOperationsText.pop_back();
	}

	for (auto& entry : std::filesystem::recursive_directory_iterator("FireflyEngine\\Shaders-bin\\"))
	{
		if (!entry.is_directory())
		{
			myBuildFiles.push_back(entry.path().string());
		}
	}

	myBuildIsCompilingShaderKeys = false;

	myBuildFuture = std::async([this]() {
		//compile fbx

		std::mutex buildMutex;

		myBuildIsCopyingFiles = true;


		//copy files
		std::for_each(std::execution::par, myBuildFiles.begin(), myBuildFiles.end(), [this, &buildMutex](auto path)
			{
				/*buildMutex.lock();
				myCurrentBuildOperationsText.push_back("Copying file: " + myBuildFiles[i].string());
				buildMutex.unlock();*/

				auto toPath = myBuildPath + std::filesystem::relative(path).string();
				std::filesystem::path directoryPath = toPath;
				directoryPath = directoryPath.remove_filename();
				if (!std::filesystem::exists(directoryPath))
				{
					std::filesystem::create_directories(directoryPath);
				}

				if (!std::filesystem::copy_file(path, toPath, std::filesystem::copy_options::overwrite_existing))
				{
					LOGERROR("FAILED TO COPY FILE WHILE BUILDING PROJECT, PATH: {}", path.string());
				}
				if (path.extension() == ".dds")
				{
					if (!Firefly::Texture2D::IsPOW2(path))
					{
						buildMutex.lock();
						myBuildWarningMessages.push_back("Texture with path: " + std::filesystem::relative(path, std::filesystem::current_path()).string() + " is not a power of 2! This will give us IG.");
						buildMutex.unlock();
					}
				}
				buildMutex.lock();
				myBuildOperationsCompletedCount++;
				buildMutex.unlock();
			});
		myBuildIsCopyingFiles = false;
		myBuildIsCheckingForGarbage = true;


		myBuildIsCheckingForGarbage = false;
		myBuildIsDone = true;
		});
}

void EditorLayer::UpdateBuildProjectImGui()
{
	if (myBuildingProjectSetting)
	{
		ImGui::OpenPopup("Build Project");
		if (ImGui::BeginPopupModal("Build Project", &myBuildingProjectSetting))
		{
			ImGui::InputText("Build Folder Name", &myBuildFolderName);
			ImGui::InputText("Build EXE Name", &myBuildExeName);

			for (int i = 0; i < myScenesToIncludeInBuild.size(); i++)
			{
				std::string text = myScenesToIncludeInBuild[i].string() + "##BuildScene" + std::to_string(i);
				if (ImGui::Button(text.c_str(),
					{ Utils::Max(200.f,ImGui::CalcTextSize(myScenesToIncludeInBuild[i].string().c_str()).x + 20),20.f }))
				{
					auto path = EditorUtils::GetOpenFilePath("Firefly Scene (*.scene)\0*.scene\0");
					bool alreadyExists = std::find(myScenesToIncludeInBuild.begin(),
						myScenesToIncludeInBuild.end(),
						path) != myScenesToIncludeInBuild.end();
					//if we didnt cancel the file dialog and we havent already added the scene
					if (path != "" && !alreadyExists)
					{
						myScenesToIncludeInBuild[i] = path;
					}
					else if (alreadyExists)
					{
						ImGuiUtils::NotifyErrorLocal("Scene already added!");
					}
				}
				ImGui::SameLine();
				std::string removeSceneText = "Remove##RemoveBuildScene" + std::to_string(i);
				if (ImGui::Button(removeSceneText.c_str()))
				{
					myScenesToIncludeInBuild.erase(myScenesToIncludeInBuild.begin() + i);
					i--;
					continue;
				}
			}
			if (ImGui::Button("ADD SCENE"))
			{
				auto path = EditorUtils::GetOpenFilePath("Firefly Scene (*.scene)\0*.scene\0");
				bool alreadyExists = std::find(myScenesToIncludeInBuild.begin(),
					myScenesToIncludeInBuild.end(),
					path) != myScenesToIncludeInBuild.end();
				//if we didnt cancel the file dialog and we havent already added the scene
				if (!path.empty() && !alreadyExists)
				{
					myScenesToIncludeInBuild.push_back(path);
				}
				else if (alreadyExists)
				{
					ImGuiUtils::NotifyErrorLocal("Scene already added!");
				}

			}

			if (ImGui::Button("Build"))
			{
				myBuildWarningMessages.clear();
				myBuildFiles.clear();
				myShaderKeysToCompileInBuild.clear();
				//Get a path to build at 
				myBuildPath = myBuildFolder + myBuildFolderName + "\\";

				std::filesystem::path buildPathAsPath = myBuildPath;
				buildPathAsPath._Remove_filename_and_separator();
				ImGui::CloseCurrentPopup();

				if (std::filesystem::exists(buildPathAsPath))
				{
					std::filesystem::remove_all(buildPathAsPath);
				}
				StartBuild();
			}

			ImGui::EndPopup();
		}
	}
	else if (myIsBuildingProject)
	{
		if (!myBuildIsDone)
		{

			ImGui::OpenPopup("Building Project...");
			if (ImGui::BeginPopupModal("Building Project..."))
			{
				std::string actionText = "";
				if (myBuildIsCompilingShaderKeys)
				{
					actionText = "Compiling Shaders...";
				}
				else if (myBuildIsCopyingFiles)
				{
					actionText = "Copying Files...";
				}
				else if (myBuildIsCheckingForGarbage)
				{
					actionText = "Checking for garbage...";
				}
				ImGui::Text(actionText.c_str());
				ImGui::ProgressBar(myBuildOperationsCompletedCount / (float)myBuildOperationsTotalCount);
				for (int i = 0; i < myCurrentBuildOperationsText.size(); i++)
				{
					ImGui::Text(myCurrentBuildOperationsText[i].c_str());
				}
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::OpenPopup("Build Complete!");
			if (ImGui::BeginPopupModal("Build Complete!", nullptr))
			{
				ImGui::Text("Build Complete!");
				ImGui::Text("Build Path: %s", myBuildPath.c_str());
				ImGui::Text("Build Exe Name: %s", myBuildExeName.c_str());

				//display warnings
				if (ImGui::BeginChild("##BuildCompleteWarningsChild", ImVec2(0, ImGui::GetContentRegionAvail().y - 50), true, ImGuiWindowFlags_HorizontalScrollbar))
				{

					if (myBuildWarningMessages.size() > 0)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 1, 0, 0, 1 });
						ImGui::Text("Warnings:");
						for (int i = 0; i < myBuildWarningMessages.size(); i++)
						{
							ImGui::Text(myBuildWarningMessages[i].c_str());
						}
						ImGui::PopStyleColor();
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 0, 1, 0, 1 });
						ImGui::Text("No warnings!");
						ImGui::PopStyleColor();
					}
				}
				ImGui::EndChild();

				if (ImGui::Button("Open Build Folder"))
				{
					ShellExecuteA(NULL, "open", myBuildPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				}
				ImGui::SameLine();
				if (ImGui::Button("Close"))
				{
					myBuildIsDone = false;
					myIsBuildingProject = false;
					ImGui::CloseCurrentPopup();

					//clear all stuff
					myBuildWarningMessages.clear();
					myBuildFiles.clear();
					myShaderKeysToCompileInBuild.clear();

				}
				ImGui::EndPopup();
			}
		}
	}
}

void EditorLayer::ChangeSceneModal(std::function<void()> anAction)
{
	if (ourEditingPrefab)
	{
		if (IsAnyEditSceneModified())
		{
			UnsavedChangesInPrefabModal(
				[this, anAction]()
				{
					if (SceneManager::Get().IsAnySceneModified())
					{
						UnsavedChangesInSceneModal(
							[this, anAction]()
							{
								anAction();
							},
							[this]()
							{
								//Do Nothing if press no
							});
					}
					else
					{
						anAction();
					}
				},
				[this]()
				{
					//Do Nothing if press no
				});

		}
		else if (SceneManager::Get().IsAnySceneModified())
		{
			UnsavedChangesInSceneModal(
				[this, anAction]()
				{
					anAction();
				},
				[this]()
				{
					//Do Nothing if press no
				});
		}
	}
	else
	{
		if (ourEditingScenes[0].lock()->IsModified())
		{
			UnsavedChangesInSceneModal(
				[this, anAction]()
				{
					anAction();
				},
				[this]()
				{
					//Do Nothing if press no
				});
		}
		else
		{
			anAction();
		}
	}
}

void EditorLayer::UnsavedChangesInPrefabModal(std::function<void()> aYesFunc, std::function<void()> aNoFunc)
{
	ImGuiUtils::OpenYesNoPopupModal("Unsaved changes in PREFAB! Discard changes?", "You have unsaved changes in the PREFAB, do you want to discard the changes?", aYesFunc, aNoFunc);
}

void EditorLayer::UnsavedChangesInSceneModal(std::function<void()> aYesFunc, std::function<void()> aNoFunc)
{
	ImGuiUtils::OpenYesNoPopupModal("Unsaved changes in SCENE! Discard changes?", "You have unsaved changes in the SCENE, do you want to discard the changes?", aYesFunc, aNoFunc);
}

void EditorLayer::InputShortcuts()
{
	const bool controlHeld = Utils::InputHandler::GetKeyHeld(VK_CONTROL);
	const bool shiftHeld = ImGui::IsKeyDown(ImGuiKey_LeftShift);

	if (Application::Get().GetIsInPlayMode())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			TogglePlayMode();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F8, false))
		{
			if (Utils::InputHandler::IsMouseHidden() || Utils::InputHandler::IsMouseCaptured())
			{
				SetImGuiInputEnabled(true);
				Utils::InputHandler::ReleaseMouse();
				Utils::InputHandler::ShowMouseCursor();
			}
			else
			{
				SetImGuiInputEnabled(false);
				Utils::InputHandler::CaptureMouse();
				Utils::InputHandler::HideMouseCursor();
			}
		}
	}
	else
	{
		if (controlHeld && ImGui::IsKeyPressed(ImGuiKey_P, false))
		{
			TogglePlayMode(true);
		}
	}

	const bool viewportFocused = GetWindow<ViewportWindow>()->IsFocused();
	const bool hierarchyFocused = GetWindow<HierarchyWindow>()->IsFocused();
	const bool inspectorFocused = GetWindow<InspectorWindow>()->IsFocused();

	if (viewportFocused || hierarchyFocused)
	{
		if (controlHeld && ImGui::IsKeyPressed(ImGuiKey_D, false))
		{
			DuplicateSelectedEntities();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		{
			DeleteSelectedEntities();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F) && !(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_LeftShift)))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_F, false) && EditorLayer::GetSelectedEntities().size() == 1)
			{
				auto& cameraTransform = GetWindow<ViewportWindow>()->GetEditorCamera()->GetCamera()->GetTransform();
				Utils::Vector3f positionToFocusOn = GetFirstSelectedEntity().lock()->GetTransform().GetPosition();
				cameraTransform.LookAt(positionToFocusOn);
				cameraTransform.SetLocalPosition(positionToFocusOn);
				cameraTransform.AddPosition(cameraTransform.GetBackward() * 300.f);
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_F))
		{
			const auto& selectedEntities = GetSelectedEntities();

			if (ImGui::IsKeyPressed(ImGuiKey_F, false) && selectedEntities.size() >= 1)
			{
				for (const auto& ent : selectedEntities)
				{
					if (!ent.expired())
					{
						const auto& entity = ent.lock();

						if (!HasSelectedParentRecursive(ent))
						{
							auto& cameraTransform = GetWindow<ViewportWindow>()->GetEditorCamera()->GetCamera()->GetTransform();

							entity->GetTransform().SetPosition(cameraTransform.GetPosition());
							entity->GetTransform().SetRotation(cameraTransform.GetQuaternion());

							if (entity->IsPrefab())
							{
								entity->UpdateTransformLocalPositionModification();
								entity->UpdateTransformLocalRotationModification();
							}
						}
					}
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_H, false))
		{
			//Unhide all entities on press CTRL + H
			if (controlHeld)
			{
				BeginEntityUndoSeries();

				for (const auto& scene : GetEditingScenes())
				{
					if (scene.expired())
					{
						LOGERROR("Scene is expired!");
						continue;
					}

					for (const auto& entity : scene.lock()->GetEntities())
					{
						if (!entity.expired())
						{
							if (!entity.lock()->GetIsActive())
							{
								HideEntityCommand(entity, false);
							}
						}
					}
				}

				EndEntityUndoSeries();
			}
			//Toggle active/inactive on press H for selected entities
			else
			{
				BeginEntityUndoSeries();

				const auto& entities = GetSelectedEntities();
				for (const auto& entity : entities)
				{
					ToggleHideEntityCommand(entity);
				}

				EndEntityUndoSeries();
			}
		}

		//F2 Rename hierarky
		if (ImGui::IsKeyPressed(ImGuiKey_F2, false) && EditorLayer::GetSelectedEntities().size() == 1)
		{
			for (auto ent : EditorLayer::GetSelectedEntities())
			{
				if (!HasSelectedParentRecursive(ent))
				{
					if (!ent.expired())
					{
						LOGINFO("Rename In hierarky");
						RenameSelectedEntity();

					}
				}
			}
		}
	}

	if (inspectorFocused || viewportFocused || hierarchyFocused)
	{
		if (controlHeld && ImGui::IsKeyPressed(ImGuiKey_N, false))
		{
			//DONT IMPLEMENT THIS UNTIL IT CHECKS WHEN SCENE ISNT SAVED
			//NewScene();
		}
		else if (controlHeld && ImGui::IsKeyPressed(ImGuiKey_S, false))
		{
			DoSave();
		}
		else if (controlHeld && shiftHeld && ImGui::IsKeyPressed(ImGuiKey_S, false))
		{
			DoSaveAs();
		}
		else if (controlHeld && ImGui::IsKeyPressed(ImGuiKey_O, false))
		{
			DoLoad();
		}
	}
}

void EditorLayer::SavePrefab()
{
	auto prefab = ourEditingPrefab;
	if (!prefab)
	{
		LOGERROR("EditorLayer::SavePrefab: Prefab is null");
		return;
	}

	auto path = prefab->GetPath();
	bool hasWritePermission = (std::filesystem::status(path).permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

	if (!hasWritePermission)
	{
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Could not save prefab at path:\n\"%s\"\nMake sure the file is checked out in Perforce!", path.string().c_str() });
	}
	else 
	{
		Firefly::SerializationUtils::SaveEntityAsPrefab(ourEditingScenes[0].lock()->GetEntities()[0], path);
		for (auto scene : SceneManager::GetCurrentScenes())
		{
			if (scene.expired())
			{
				LOGERROR("EditorLayer::SavePrefab(): Scene is expired!");
				continue;
			}
			//scene.lock()->UpdatePrefabs();
		}
		ourEditingScenes[0].lock()->SetAsModified(false);

		ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Prefab successfully saved to path:\n\"%s\"", path.string().c_str() });
	}
}


void EditorLayer::CreateEditorWindow(const std::string& aWindowFactoryName)
{
	myEditorWindows.push_back(WindowRegistry::Create(aWindowFactoryName));
	LoadEditorSettingsEvent e(myEditorSettingsJson);
	myEditorWindows.back()->OnEvent(e);
}

void EditorLayer::SetShouldOutline(Ptr<Firefly::Entity> aEntity, bool aShouldOutline)
{
	if (aEntity.expired())
	{
		LOGERROR("EditorLayer::SetShouldOutline(): Entity is expired!");
		return;
	}
	if (auto meshComp = aEntity.lock()->GetComponent<MeshComponent>().lock())
	{
		meshComp->SetShouldOutline(aShouldOutline);
	}
	else if (auto animComp = aEntity.lock()->GetComponent<AnimatedMeshComponent>().lock())
	{
		animComp->SetShouldOutline(aShouldOutline);
	}
}

bool EditorLayer::HasSelectedParentRecursive(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("EditorLayer::HasSelectedParentRecursive(): Entity is expired!");
		return false;
	}
	while (aEntity.lock()->HasParent())
	{
		if (std::find_if(ourSelectedEntities.begin(), ourSelectedEntities.end(), [aEntity](Ptr<Firefly::Entity> ent) { return  aEntity.lock()->GetParentID() == ent.lock()->GetID(); }) != ourSelectedEntities.end())
		{
			return true;
		}

		aEntity = aEntity.lock()->GetParent();
	}

	return false;
}

bool EditorLayer::HasSelectedChildren(Ptr<Firefly::Entity> aEntity)
{
	const auto& selectedEntities = GetSelectedEntities();
	const auto& children = aEntity.lock()->GetChildrenRecursive();

	for (const auto& child : children)
	{
		auto it = std::find_if(selectedEntities.begin(), selectedEntities.end(),
			[child](const Ptr<Firefly::Entity>& aEntityToCheck) { return aEntityToCheck.lock()->GetID() == child.lock()->GetID(); });

		if (it != selectedEntities.end())
		{
			return true;
		}
	}

	return false;
}

void EditorLayer::SetImGuiInputEnabled(bool aEnable)
{
	auto& io = ImGui::GetIO();

	if (aEnable)
	{
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
	else
	{
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
}

void EditorLayer::SaveCrashedScene()
{
	int existsCount = 0;
	std::string	newPath = "CRASHED SCENE WITHOUT SAVE.scene";
	std::string testPath = newPath;
	while (std::filesystem::exists(testPath))
	{
		testPath = newPath;
		auto it = newPath.find_last_of(".");
		testPath.insert(it, std::to_string(existsCount));
		existsCount++;
	}
	newPath = testPath;
	for (int i = 0; i < SceneManager::GetCurrentScenes().size(); i++)
	{
		auto it = newPath.find_last_of(".");
		newPath.insert(it, "(" + SceneManager::GetCurrentScenes()[i].lock()->GetPath().stem().string() + ")");
		std::ofstream file(newPath);
		nlohmann::json saveFile;
		SerializationUtils::SerializeScene(saveFile, SceneManager::GetCurrentScenes()[i]);
		file << saveFile;
		file.close();
	}
}
