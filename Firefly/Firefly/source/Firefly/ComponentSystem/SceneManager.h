#pragma once
#include <filesystem>
#include <memory>
#include <vector>
#include <unordered_set>

#include "Firefly/Core/Core.h"

namespace Explorer
{
	struct NavMeshBuildSettings;
}

class FireflyNavMesh;

namespace Firefly
{
	class Scene;
	struct Parameter;
	class PhysicsScene;

	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager() = default;

		static void Initialize();
		static SceneManager& Get();
		static std::vector<Ptr<Scene>> GetCurrentScenes();

		void SetCurrentScenes(const std::vector<std::shared_ptr<Scene>>& someScenes, bool aInPlayAndKeepPlay = false);

		void NewScene();
		void LoadScene(const std::filesystem::path& aScenePath);
		void LoadSceneAdd(const std::filesystem::path& aScenePath);
		void AddSceneAndInitialize(Ref<Scene> aScene);
		void UnloadScene(const std::filesystem::path& aScenePath);
		bool IsSceneLoaded(const std::filesystem::path& aScenePath);
		bool IsAnySceneModified();

		void OnRuntimeStartCurrentScenes(bool aBuildPhysicsScene = true);

		Ref<PhysicsScene> GetPhysicsScene() { return myPhysicsScene; }
		void DestroyPhysicsScene();
		void CreatePhysicsScene();

		std::shared_ptr<Scene> GetLoadScene(const std::filesystem::path& aScenePath);

		void PreloadPrefab(const std::filesystem::path& aPrefabPath);

		void QueueEntityDelete(uint64_t aEntityID);
		void DeleteQueuedEntities();

		void GetLoadScene(const std::filesystem::path& aScenePath, Ref<Scene>& aScene);
	private:
		void UnloadScene(Ref<Scene> aScene);

		static SceneManager* myInstance;

		std::vector<Ref<Scene>> myCurrentScenes;
		Ref<PhysicsScene> myPhysicsScene;
		std::unordered_set<uint64_t> myPreloadedPrefabIDs;

		Ref<Scene> myPreloadingScene;

		std::vector<uint64_t> myQueuedEntityDeletes;

		std::mutex mySceneMutex;
	};
}