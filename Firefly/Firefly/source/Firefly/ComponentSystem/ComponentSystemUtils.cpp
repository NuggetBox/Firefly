#include "FFpch.h"
#include "ComponentSystemUtils.h"
#include "Firefly/ComponentSystem/Scene.h"

namespace Firefly
{
	Ptr<Firefly::Entity> Instantiate(Ptr<Scene> aScene)
	{
		auto targetScene = aScene;
		if (!targetScene.lock())
		{
			targetScene = Firefly::SceneManager::GetCurrentScenes()[0];
		}
		return targetScene.lock()->Instantiate();
	}

	Ptr<Firefly::Entity> Instantiate(Ref<Firefly::Prefab> aPrefab, Ptr<Scene> aScene)
	{
		if (!aPrefab)
		{
			LOGERROR(__FUNCTION__" Could not instantiate prefab because it was null");
			return Ptr<Entity>();
		}

		auto targetScene = aScene;

		if (targetScene.expired())
		{
			const auto& scenes = SceneManager::GetCurrentScenes();

			for (int i = scenes.size() - 1; i >= 0 ; i--)
			{
				const auto& sceneWeak = scenes[i];

				if (!sceneWeak.expired())
				{
					const auto& scene = sceneWeak.lock();

					if (scene->IsLoaded())
					{
						targetScene = sceneWeak;
					}
				}
			}
		}

		return targetScene.lock()->Instantiate(aPrefab);
	}

	void PreloadPrefab(const std::filesystem::path& aPrefabPath)
	{
		SceneManager::Get().PreloadPrefab(aPrefabPath);
	}

	Ptr<Firefly::Entity> GetEntityWithName(const std::string& aName)
	{
		for (auto scene : Firefly::SceneManager::GetCurrentScenes())
		{
			for (auto entity : scene.lock()->GetEntities())
			{
				if (entity.lock()->GetName() == aName)
				{
					return entity;
				}
			}
		}
		return Ref<Firefly::Entity>();
	}

	std::vector<Ptr<Firefly::Entity>> GetEntitiesWithName(const std::string& aName)
	{
		std::vector<Ptr<Entity>> entities;
		for (auto scene : Firefly::SceneManager::GetCurrentScenes())
		{
			for (auto entity : scene.lock()->GetEntities())
			{
				if (entity.lock()->GetName() == aName)
				{
					entities.push_back(entity);
				}
			}
		}
		return entities;
	}

	std::vector<Ptr<Entity>> GetAllEntities()
	{
		std::vector<Ptr<Entity>> returnVec;
		for (auto& scene : SceneManager::GetCurrentScenes())
		{
			std::vector<Ptr<Entity>> ents = scene.lock()->GetEntities();
			returnVec.insert(returnVec.end(), ents.begin(), ents.end());
		}
		return returnVec;
	}

	Ptr<Firefly::Entity> GetEntityWithTag(const std::string& aTag)
	{
		for (auto scene : Firefly::SceneManager::GetCurrentScenes())
		{
			for (auto entity : scene.lock()->GetEntities())
			{
				if (entity.lock()->GetTag() == aTag)
				{
					return entity;
				}
			}
		}
		return Ptr<Firefly::Entity>();
	}

	std::vector<Ptr<Firefly::Entity>> GetEntitiesWithTag(const std::string& aTag)
	{
		std::vector<Ptr<Entity>> entities;
		for (auto scene : Firefly::SceneManager::GetCurrentScenes())
		{
			for (auto entity : scene.lock()->GetEntities())
			{
				if (entity.lock()->GetTag() == aTag)
				{
					entities.push_back(entity);
				}
			}
		}
		return entities;
	}

	Ptr<Firefly::Entity> GetEntityWithID(uint64_t aID)
	{
		for (auto scene : Firefly::SceneManager::GetCurrentScenes())
		{
			auto ent = scene.lock()->GetEntityByID(aID);
			if (ent.lock())
			{
				return ent;
			}
		}
		return Ptr<Entity>();
	}

	void DeleteEntity(uint64_t aID)
	{
		QueueDeleteEntity(aID);
	}

	void DeleteEntity(const Ptr<Entity>& aEntity)
	{
		QueueDeleteEntity(aEntity);
	}

	void QueueDeleteEntity(uint64_t aID)
	{
		SceneManager::Get().QueueEntityDelete(aID);
	}

	void QueueDeleteEntity(const Ptr<Entity>& aEntity)
	{
		if (!aEntity.expired())
		{
			SceneManager::Get().QueueEntityDelete(aEntity.lock()->GetID());
		}
	}
}