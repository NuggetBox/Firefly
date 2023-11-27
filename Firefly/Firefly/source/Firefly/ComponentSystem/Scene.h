#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Event;
}

namespace Firefly
{
	class Prefab;
	class Component;
	class Entity;
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		static Ref<Scene> DeepCopy(Ref<Scene> aScene);

		void OnEvent( Firefly::Event& aEvent);

		void OnRuntimeStart();

		void AddEntity(Ref<Entity> aEntity);
		Ptr<Entity> Instantiate();
		Ptr<Entity> Instantiate(Ref<Prefab> aPrefab, bool aNewIDFlag = true);

		void InsertEntity(Ref<Entity> aEntity, uint32_t aIndex);
		void InsertEntities(std::vector<Ref<Entity>> aEntity, uint32_t aIndex);

		void RemoveEntityINTERNALUSE(Ptr<Entity> aEntity);

		template<class T>
		Ptr<Entity> GetEntityWithComponent()
		{
			return GetEntityWithComponent(T::GetFactoryName());
		}

		Ptr<Entity> GetEntityWithComponent(const std::string& aComponentName);

		void DeleteEntity(Ptr<Entity> aEntity);
		void DeleteEntity(uint64_t aId);
		
		std::vector<Ptr<Entity>> GetEntities();
		std::vector<Ptr<Entity>> GetParentEntities();

		Ptr<Entity> GetEntityByID(uint64_t aID);

		void SetAsModified(bool aFlag = true);
		bool IsModified() const;

		void UpdatePrefabs();
		
		std::filesystem::path GetPath() const { return myPath; }
		void SetPath(const std::filesystem::path& aPath) { myPath = aPath; }

		void SetLoaded(bool aFlag = true) { myLoadedFlag = aFlag; }
		void SetInitialized(bool aFlag = true) { myInitializedFlag = aFlag; }
		bool IsLoaded() { return myLoadedFlag; }
		bool IsInitialized() { return myInitializedFlag; }


	private:
		friend class SceneManager;
		friend class SerializationUtils;

		bool myWasModifiedAfterSave = false;
		bool myLoadedFlag = false;
		bool myInitializedFlag = false;
		std::filesystem::path myPath;

		std::mutex myEntityListMutex;

		std::vector<Ref<Entity>> myEntities;
		std::unordered_map<uint64_t, Ref<Entity>> myEntitiesMap;
	};
}