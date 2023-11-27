#pragma once
#include "Scene.h"

#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/SceneManager.h"

namespace Firefly
{
	class Prefab;
}

namespace Firefly
{
	Ptr<Entity> Instantiate(Ptr<Scene> aScene = Ptr<Scene>());
	Ptr<Entity> Instantiate(Ref<Firefly::Prefab> aPrefab, Ptr<Scene> aScene = Ptr<Scene>());

	void PreloadPrefab(const std::filesystem::path& aPrefabPath);

	Ptr<Entity> GetEntityWithName(const std::string& aName);
	std::vector<Ptr<Entity>> GetEntitiesWithName(const std::string& aName);
	std::vector<Ptr<Entity>> GetAllEntities();
	Ptr<Entity> GetEntityWithTag(const std::string& aTag);
	std::vector<Ptr<Entity>> GetEntitiesWithTag(const std::string& aTag);
	Ptr<Entity> GetEntityWithID(uint64_t aID);

	template<class T>
	Ptr<T> GetComponentInScene();

	template<class T>
	std::vector<Ptr<T>> GetComponentsInScene();

	template<class T>
	Ptr<Entity> GetEntityWithComponent();

	template<class T>
	std::vector<Ptr<Entity>> GetEntitiesWithComponent();

	//These are still here for backwards compatibility
	void DeleteEntity(uint64_t aID);
	void DeleteEntity(const Ptr<Entity>& aEntity);
	void QueueDeleteEntity(uint64_t aID);
	void QueueDeleteEntity(const Ptr<Entity>& aEntity);

	template<class T>
	Ptr<T> GetComponentInScene()
	{
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					return entity.lock()->GetComponent<T>();
				}
			}
		}

		return Ref<T>();
	}

	template<class T>
	std::vector<Ptr<T>> GetComponentsInScene()
	{
		std::vector<Ref<T>> components;
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					components.push_back(entity.lock()->GetComponent<T>());
				}
			}
		}

		return components;
	}

	template<class T>
	Ptr<Entity> GetEntityWithComponent()
	{
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					return entity;
				}
			}
		}

		return Ptr<Entity>();
	}

	template<class T>
	Ptr<T> GetComponentOfEntity()
	{
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					return entity.lock()->GetComponent<T>();
				}
			}
		}

		return Ptr<T>();
	}

	template<class T>
	std::vector<Ptr<Entity>> GetEntitiesWithComponent()
	{
		std::vector<Ptr<Entity>> entities;
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					entities.push_back(entity);
				}
			}
		}

		return entities;
	}

	template<class T>
	std::vector<Ptr<T>> GetComponentsFromEntitiesWithComponent()
	{
		std::vector<Ptr<T>> comps;
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					comps.push_back(entity.lock()->GetComponent<T>());
				}
			}
		}

		return comps;
	}

	template<class T>
	std::vector<Ptr<T>> GetComponentsOnEntities()
	{
		std::vector<Ptr<T>> entities;
		for (const auto& sceneWeak : Firefly::SceneManager::GetCurrentScenes())
		{
			if (sceneWeak.expired())
			{
				continue;
			}

			const auto& scene = sceneWeak.lock();

			if (!scene->IsLoaded())
			{
				continue;
			}

			for (const auto& entity : scene->GetEntities())
			{
				if (entity.expired())
				{
					continue;
				}

				if (entity.lock()->HasComponent<T>())
				{
					entities.push_back(entity.lock()->GetComponent<T>());
				}
			}
		}

		return entities;
	}
}
