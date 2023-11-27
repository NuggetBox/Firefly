#include "FFpch.h"
#include "SceneManager.h"

#include <Firefly/Asset/ResourceCache.h>

#include <nlohmann/json.hpp>

#include "ScriptEventManager.h"
#include "SerializationUtils.h"
#include "Firefly/ComponentSystem/Scene.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Components/Lights/EnvironmentLightComponent.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Core/Log/DebugLogger.h"
#include "Firefly/Event/SceneEvents.h"
#include "Firefly/Physics/PhysicsScene.h"

#include "imnotify/imgui_notify.h" 

#include "simdjson/simdjson.h"

#include "Utils/Timer.h"
#include "DiscordAPI/DiscordAPI.h"
#include <Utils/TimerManager.h>

namespace Firefly
{
	SceneManager* SceneManager::myInstance = nullptr;

	SceneManager::SceneManager()
	{
		myPreloadingScene = CreateRef<Scene>();
	}

	void Firefly::SceneManager::Initialize()
	{
		myInstance = new SceneManager();
	}

	SceneManager& Firefly::SceneManager::Get()
	{
		return *myInstance;
	}

	std::vector<Ptr<Scene>> Firefly::SceneManager::GetCurrentScenes()
	{
		std::vector<Ptr<Scene>> scenes;
		for (int i = 0; i < myInstance->myCurrentScenes.size(); i++)
		{
			if (myInstance->myCurrentScenes[i])
			{
				if (myInstance->myCurrentScenes[i]->IsLoaded())
				{
					scenes.push_back(myInstance->myCurrentScenes[i]);
				}
			}
		}
		return scenes;
	}

	void SceneManager::SetCurrentScenes(const std::vector<std::shared_ptr<Scene>>& someScenes, bool aInPlayAndKeepPlay)
	{
		if (someScenes.empty())
		{
			LOGERROR("SceneManager::SetCurrentScenes Tried To set scenes to a vector with zero elements ");
			return;
		}
		for (int i = 0; i < myCurrentScenes.size(); i++)
		{
			UnloadScene(myCurrentScenes[i]);
			i--;
		}
		for (int i = 0; i < someScenes.size(); i++)
		{
			std::scoped_lock temp(mySceneMutex);
			someScenes[i]->SetLoaded();
			myCurrentScenes.push_back(someScenes[i]);
		}
		SceneLoadedEvent ev(myCurrentScenes[0], false);
		Firefly::Application::Get().OnEvent(ev);
		for (int i = 1; i < myCurrentScenes.size(); i++)
		{
			SceneLoadedEvent ev(myCurrentScenes[i], true);
			Firefly::Application::Get().OnEvent(ev);
		}
		bool buildPhysScene = true;

		if (myPhysicsScene)
		{
			if (!aInPlayAndKeepPlay)
			{
				myPhysicsScene.reset();
			}
			buildPhysScene = false;
		}

		ScriptEventManager::CleanUp();
		Utils::TimerManager::RemoveAllTimers();

		/*if (Get().GetPhysicsScene())
		{
			Get().GetPhysicsScene()->GetCharacterControllerPtr()->purgeControllers();
		}*/

		OnRuntimeStartCurrentScenes(buildPhysScene);

	}

	void Firefly::SceneManager::NewScene()
	{
		for (int i = 0; i < myCurrentScenes.size(); i++)
		{
			UnloadScene(myCurrentScenes[i]);
			i--;
		}
		{
			std::scoped_lock temp(mySceneMutex);
			myCurrentScenes.push_back(std::make_shared<Scene>());
			myCurrentScenes.back()->SetPath("");
		}

		auto mainCamera = myCurrentScenes[0]->Instantiate().lock();
		mainCamera->SetName("Play Camera");
		mainCamera->AddComponent(ComponentRegistry::Create("CameraComponent"));
		mainCamera->GetTransform().SetLocalPosition(0, 100, -300);
		mainCamera->GetTransform().SetRotation(30, 0, 0);

		auto dirLight = myCurrentScenes[0]->Instantiate().lock();
		dirLight->SetName("Directional Light");
		dirLight->AddComponent(ComponentRegistry::Create("DirectionalLightComponent"));
		dirLight->GetTransform().SetRotation(-45, -45, 0);

		auto envLight = myCurrentScenes[0]->Instantiate().lock();
		envLight->SetName("Environment Light");
		envLight->AddComponent(ComponentRegistry::Create("EnvironmentLightComponent"));
		envLight->GetComponent<EnvironmentLightComponent>().lock()->SetCubeMap("FireflyEngine/Defaults/skansen_cubemap.dds");

		auto postProcess = myCurrentScenes[0]->Instantiate().lock();
		postProcess->SetName("Post Process");
		postProcess->AddComponent(ComponentRegistry::Create("PostProcessComponent"));
		SceneLoadedEvent ev(myCurrentScenes[0], false);
		Firefly::Application::Get().OnEvent(ev);
		myCurrentScenes[0]->myWasModifiedAfterSave = false;
		myCurrentScenes[0]->SetLoaded();
	}

	bool SceneManager::IsAnySceneModified()
	{
		for (int i = 0; i < myCurrentScenes.size(); i++)
		{
			if (myCurrentScenes[i]->IsModified())
			{
				return true;
			}
		}
		return false;
	}

	void SceneManager::OnRuntimeStartCurrentScenes(bool aBuildPhysicsScene)
	{
		FF_PROFILESCOPE("Scenes On Runtime Start");

		if (aBuildPhysicsScene)
		{
			myPhysicsScene = PhysicsScene::Create(PhysicsSceneInfo());
		}

		for (auto& scene : myCurrentScenes)
		{
			scene->OnRuntimeStart();
		}
	}

	void SceneManager::DestroyPhysicsScene()
	{
		myPhysicsScene.reset();
	}

	void SceneManager::CreatePhysicsScene()
	{
		if (!myPhysicsScene)
		{
			myPhysicsScene = PhysicsScene::Create(PhysicsSceneInfo());
		}
	}

	std::shared_ptr<Scene> SceneManager::GetLoadScene(const std::filesystem::path& aScenePath)
	{

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();

		GetLoadScene(aScenePath, scene);

		return scene;
	}

	void SceneManager::PreloadPrefab(const std::filesystem::path& aPrefabPath)
	{
		if (!myPreloadingScene)
		{
			LOGERROR("Preloading scene not initialized");
			return;
		}

		const auto prefabAsset = ResourceCache::GetAsset<Prefab>(aPrefabPath, true);

		if (!prefabAsset || !prefabAsset->IsLoaded())
		{
			LOGERROR("Preloading Prefab failed, couldn't load prefab");
			return;
		}

		if (myPreloadedPrefabIDs.contains(prefabAsset->GetPrefabID()))
		{
			return;
		}
		myPreloadedPrefabIDs.insert(prefabAsset->GetPrefabID());

		const auto instantiated = myPreloadingScene->Instantiate(prefabAsset);

		if (instantiated.expired())
		{
			LOGERROR("Prefab preloading failed, entity couldn't spawn");
			return;
		}

		myPreloadingScene->DeleteEntity(instantiated);
	}

	void SceneManager::QueueEntityDelete(uint64_t aEntityID)
	{
		myInstance->myQueuedEntityDeletes.push_back(aEntityID);
	}

	void SceneManager::DeleteQueuedEntities()
	{
		for (const auto id : myInstance->myQueuedEntityDeletes)
		{
			auto entityWeak = GetEntityWithID(id);

			if (!entityWeak.expired())
			{
				auto entity = entityWeak.lock();
				entity->GetParentScene()->DeleteEntity(entity->GetID());
			}
			else
			{
				LOGWARNING("Tried to delete a queued entity but it was already expired");
			}
		}

		myInstance->myQueuedEntityDeletes.clear();
	}

	void SceneManager::GetLoadScene(const std::filesystem::path& aScenePath, Ref<Scene>& aScene)
	{
		FF_PROFILESCOPE("Load Scene");

		Utils::Timer::Update();
		auto totalTimeBegin = Utils::Timer::GetTotalTime();

		simdjson::ondemand::parser parser;
		auto json = simdjson::padded_string::load(aScenePath.string());
		if (json.value().length() < 0)
		{
			const auto errorString = simdjson::error_message(json.error());
			LOGERROR("Scene failed to parse with error: {}", errorString);
			return;
		}
		simdjson::ondemand::document doc = parser.iterate(json);

		Utils::Timer::Update();
		auto parsingTime = Utils::Timer::GetTotalTime();
		LOGINFO("Loading scene {}: Parsing the scene json took {} seconds.", aScenePath.filename().string(), parsingTime - totalTimeBegin);

		aScene = SerializationUtils::DeserializeScene(doc.get_value());

		Utils::Timer::Update();
		auto deserializeTime = Utils::Timer::GetTotalTime();
		LOGINFO("Loading scene {}: Deserializing the scene took {} seconds.", aScenePath.filename().string(), deserializeTime - parsingTime);

		aScene->SetPath(aScenePath);
		aScene->myWasModifiedAfterSave = false;
	}

	void SceneManager::UnloadScene(Ref<Scene> aScene)
	{
		SceneUnloadedEvent ev(aScene);
		Application::Get().OnEvent(ev);
		aScene->SetLoaded(false);
		std::scoped_lock scopeLock(mySceneMutex);
		myCurrentScenes.erase(std::find(myCurrentScenes.begin(), myCurrentScenes.end(), aScene));
	}

	void Firefly::SceneManager::LoadScene(const std::filesystem::path& aScenePath)
	{
		for (int i = 0; i < myCurrentScenes.size(); i++)
		{
			UnloadScene(myCurrentScenes[i]);
			i--;
		}

		LoadSceneAdd(aScenePath);
	}

	void SceneManager::LoadSceneAdd(const std::filesystem::path& aScenePath)
	{
		if (IsSceneLoaded(aScenePath))
		{
			LOGERROR("Tried to load a scene that is already loaded!");
			return;
		}
		auto startTime = Utils::Timer::GetTotalTime();
		auto scene = GetLoadScene(aScenePath);
		{
			std::scoped_lock temp(mySceneMutex);
			myCurrentScenes.push_back(scene);
		}

		scene->SetLoaded();
		myCurrentScenes.back()->OnRuntimeStart();

		SceneLoadedEvent ev(scene, myCurrentScenes.size() > 1);
		Firefly::Application::Get().OnEvent(ev);

		Utils::Timer::Update();
		auto initializeTime = Utils::Timer::GetTotalTime();
		LOGINFO("Loading scene {}: Initializing the scene took {} seconds.", aScenePath.filename().string(), initializeTime - startTime);

		Utils::Timer::Update();
		auto totalTimeEnd = Utils::Timer::GetTotalTime();
		LOGINFO("Loading scene {}: Loading the entire scene took {} seconds in total.", aScenePath.filename().string(), (totalTimeEnd - startTime));

		//Prevents big delta time after loading scene
		Utils::Timer::Update();
		DiscordAPI::SetStartTimestampToNow();

		DiscordAPI::SetStatus(aScenePath.stem().string());
	}

	void SceneManager::UnloadScene(const std::filesystem::path& aScenePath)
	{
		if (aScenePath == "")
		{
			LOGERROR("Tried to unload scene with empty path");
			return;
		}
		if (aScenePath == myCurrentScenes[0]->GetPath())
		{
			LOGERROR("Tried to unload main scene, this is not allowed!");
			return;
		}
		for (int i = 0; i < myCurrentScenes.size(); i++)
		{
			if (myCurrentScenes[i]->GetPath() == aScenePath)
			{
				if (i == 0)
				{
					NewScene();
				}
				else
				{
					std::scoped_lock scopeLock(mySceneMutex);
					SceneUnloadedEvent ev(myCurrentScenes[i]);
					Application::Get().OnEvent(ev);
					myCurrentScenes[i]->SetLoaded(false);
					myCurrentScenes.erase(myCurrentScenes.begin() + i);
				}
				return;
			}
		}
	}

	bool SceneManager::IsSceneLoaded(const std::filesystem::path& aScenePath)
	{
		//check if scene is already loaded
		for (auto& scene : myCurrentScenes)
		{
			if (scene->GetPath() == aScenePath && scene->IsLoaded())
			{
				return true;
			}
		}
		return false;
	}
}
