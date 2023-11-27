#include "FFpch.h"
#include "Scene.h"

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/ComponentSystem/Entity.h"

#include "Firefly/Event/Event.h"

#include "Firefly/Asset/Prefab.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Event/EntityEvents.h"

#include <random>

namespace Firefly
{
	Ref<Scene> Scene::DeepCopy(Ref<Scene> aScene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->myPath = aScene->myPath;
		newScene->myEntities.clear();

		for (auto parentToCopy : aScene->GetParentEntities())
		{
			Entity::Duplicate(parentToCopy, newScene.get(), false);
		}

		auto entities = newScene->GetEntities();

		for (auto entity : entities)
		{
			if (entity.expired())
			{
				LOGERROR("Scene::DeepCopy: Entity is expired!");
				continue;
			}
			for (auto component : entity.lock()->GetComponents())
			{
				if (component.expired())
				{
					LOGERROR("Scene::DeepCopy: Component is expired!");
					continue;
				}
				for (auto& parameter : component.lock()->GetSerializedVariablesMutable())
				{
					if (parameter.Type == ParameterType::Entity)
					{
						const uint64_t entityID = parameter.EntityID;
						Ptr<Entity> realEntity = newScene->GetEntityByID(entityID);
						*static_cast<Ptr<Entity>*>(parameter.Pointer) = realEntity;
					}
					if (parameter.Type == ParameterType::List && parameter.ListType == ParameterType::Entity)
					{
						std::vector<Ptr<Entity>>& entityList = *static_cast<std::vector<Ptr<Entity>>*>(parameter.Pointer);
						entityList.resize(parameter.EntityIDVector.size());
						size_t index = 0;
						for (auto& entityElement : parameter.EntityIDVector)
						{

							if (entityElement == 0)
							{
								//list element is not set
								continue;
							}

							//const uint64_t entityID = entityElement;
							Ptr<Entity> realEntity = newScene->GetEntityByID(entityElement);
							entityList[index] = realEntity;
							index++;
						}
					}
				}
			}
		}
		newScene->myLoadedFlag = true;
		return newScene;
	}

	void Scene::OnEvent(Firefly::Event& aEvent)
	{
		if (!myLoadedFlag)
		{
			return;
		}
		if (!myInitializedFlag)
		{
			return;
		}
		FF_PROFILESCOPE(("Scene event: " + aEvent.ToString()).c_str());

		for (size_t i = 0; i < myEntities.size(); i++)
		{
			if (!myEntities[i])
			{
				LOGERROR("Scene::OnEvent: Entity is null!");
				continue;
			}
			myEntities[i]->OnEvent(aEvent);
		}
	}
	void Scene::OnRuntimeStart()
	{
		FF_PROFILESCOPE("Scene On Runtime Start");

		{
			FF_PROFILESCOPE("Scene Early Initialize");
			for (size_t i = 0; i < myEntities.size(); i++)
			{
				myEntities[i]->EarlyInitialize();
			}
		}


		{
			FF_PROFILESCOPE("Scene Initialize");
			for (size_t i = 0; i < myEntities.size(); i++)
			{
				myEntities[i]->Initialize();
			}
		}

		myInitializedFlag = true;
	}
	void Scene::AddEntity(Ref<Entity> aEntity)
	{
		if (!aEntity)
		{
			LOGERROR("Scene::AddEntity: Entity is null! Can't add entity to scene!");
			return;
		}
		std::scoped_lock mutexLock(myEntityListMutex);
		aEntity->myParentScene = this;
		aEntity->ResetLifeTime();
		myEntities.push_back(aEntity);
		myEntitiesMap[aEntity->GetID()] = aEntity;
		SetAsModified();
	}

	std::vector<Ptr<Entity>> Scene::GetEntities()
	{
		std::vector<Ptr<Entity>> entities;
		entities.reserve(myEntities.size());

		for (const auto& ent : myEntities)
		{
			entities.push_back(ent);
		}

		/*std::transform(myEntities.begin(), myEntities.end(), std::back_inserter(entities),
			[](const Ref<Entity>& entity) { return entity; });*/

		return entities;
	}

	std::vector<Ptr<Entity>> Scene::GetParentEntities()
	{
		std::vector<Ptr<Entity>> parents;

		for (auto& entity : myEntities)
		{
			if (!entity->HasParent())
			{
				parents.push_back(entity);
			}
		}

		return parents;
	}

	Ptr<Entity> Scene::GetEntityByID(uint64_t aID)
	{
		std::scoped_lock mutexLock(myEntityListMutex);
		if (myEntitiesMap.contains(aID))
		{
			return myEntitiesMap[aID];
		}
		return {};
	}

	Ptr<Entity> Scene::Instantiate()
	{
		std::shared_ptr<Entity> entity = std::make_shared<Entity>("New Entity", Entity::GenerateRandomID());
		AddEntity(entity);
		return entity;
	}

	Ptr<Firefly::Entity> Scene::Instantiate(Ref<Prefab> aPrefab, bool aNewIDFlag)
	{
		auto& prefabEntities = aPrefab->GetEntities();
		std::vector<std::shared_ptr<Firefly::Entity>> spawnedEntities;
		uint64_t rootID = 0;
		//loop through and create all entities
		for (auto ent : prefabEntities)
		{
			Ptr<Firefly::Entity> newEntWeak = Entity::Duplicate(ent, this, aNewIDFlag);
			if (!newEntWeak.lock())
			{
				LOGERROR("Scene::Instantiate: Failed to duplicate entity!");
				return Ptr<Entity>();
			}
			auto newEnt = newEntWeak.lock();
			if (rootID == 0)
			{
				newEnt->GetTransform().SetLocalPosition(0, 0, 0);
				rootID = newEnt->GetID();
				newEnt->SetPrefabID(aPrefab->GetPrefabID());
			}
			newEnt->SetPrefabRootEntityID(rootID);
			newEnt->SetCorrespondingSourceID(ent->GetID());
			spawnedEntities.push_back(newEnt);
		}

		//setup parent child relationships and also set entity pointers
		for (int i = 0; i < prefabEntities.size(); i++)
		{
			auto& prefabEnt = prefabEntities[i];
			auto& spawnedEnt = spawnedEntities[i];

			if (prefabEnt->GetParentID() != 0)
			{
				for (int j = 0; j < prefabEntities.size(); j++)
				{
					if (prefabEntities[j]->GetID() == prefabEnt->GetParentID())
					{
						spawnedEnt->SetParent(spawnedEntities[j], -1, true, false);
						break;
					}
				}
			}

			//go through all entity parameters and set them to the prefab values
			auto components = prefabEnt->GetComponents();
			auto newComponents = spawnedEnt->GetComponents();
			for (int compIndex = 0; compIndex < components.size(); compIndex++)
			{
				auto comp = components[compIndex].lock();
				auto newComp = newComponents[compIndex].lock();
				for (int paramIndex = 0; paramIndex < comp->GetSerializedVariables().size(); paramIndex++)
				{
					auto& param = comp->GetSerializedVariables()[paramIndex];
					auto& newParam = newComp->GetSerializedVariablesMutable()[paramIndex];

					if (param.Type == Firefly::ParameterType::Entity)
					{
						for (int j = 0; j < prefabEntities.size(); j++)
						{
							if (prefabEntities[j]->GetID() == param.EntityID)
							{
								*static_cast<Ptr<Entity>*>(newParam.Pointer) = spawnedEntities[j];

								EntityPropertyUpdatedEvent ev(param.Name, param.Type);
								newComp->OnEvent(ev);
								break;
							}
						}
					}
					else if (param.Type == Firefly::ParameterType::List && param.ListType == Firefly::ParameterType::Entity)
					{
						auto& newEntityList = *static_cast<std::vector<Ptr<Entity>>*>(newParam.Pointer);
						newEntityList.resize(param.EntityIDVector.size());
						bool anyChanged = false;
						for (int i = 0; i < param.EntityIDVector.size(); i++)
						{
							for (int j = 0; j < prefabEntities.size(); j++)
							{
								if (prefabEntities[j]->GetID() == param.EntityIDVector[i])
								{
									newEntityList[i] = spawnedEntities[j];
									anyChanged = true;
									break;
								}
							}
						}
						EntityPropertyUpdatedEvent ev(param.Name, param.Type);
						newComp->OnEvent(ev);
					}
				}
			}

		}

		for (auto ent : spawnedEntities)
		{
			ent->EarlyInitialize();
		}
		for (auto ent : spawnedEntities)
		{
			ent->Initialize();
		}
		return spawnedEntities[0];
	}

	void Scene::InsertEntity(Ref<Entity> aEntity, uint32_t aIndex)
	{
		std::scoped_lock mutexLock(myEntityListMutex);
		aEntity->myParentScene = this;
		myEntities.insert(myEntities.begin() + aIndex, aEntity);
		myEntitiesMap[aEntity->GetID()] = aEntity;

		SetAsModified();
	}

	void Scene::InsertEntities(std::vector<Ref<Entity>> aEntity, uint32_t aIndex)
	{
		std::scoped_lock mutexLock(myEntityListMutex);
		for (auto& entity : aEntity)
		{
			entity->myParentScene = this;
		}
		myEntities.insert(myEntities.begin() + aIndex, aEntity.begin(), aEntity.end());
		for (auto& entity : aEntity)
		{
			myEntitiesMap[entity->GetID()] = entity;
		}
		SetAsModified();
	}

	void Scene::RemoveEntityINTERNALUSE(Ptr<Entity> aEntity)
	{
		//find 
		auto it = std::find_if(myEntities.begin(), myEntities.end(), [aEntity](const Ref<Entity>& ent)
			{
				if (!ent || aEntity.expired())
				{
					return false;
				}
				return ent->GetID() == aEntity.lock()->GetID();
			});
		if (it != myEntities.end())
		{
			std::scoped_lock mutexLock(myEntityListMutex);
			myEntities.erase(it);
		}
	}

	Ptr<Entity> Scene::GetEntityWithComponent(const std::string& aComponentName)
	{
		for (const auto& entity : GetEntities())
		{
			if (entity.expired())
			{
				continue;
			}

			if (entity.lock()->HasComponent(aComponentName))
			{
				return entity;
			}
		}

		return {};
	}

	void Scene::DeleteEntity(Ptr<Entity> aEntityWeak)
	{
		if (aEntityWeak.expired())
		{
			LOGERROR("Scene::DeleteEntity: Entity is expired!");
			return;
		}
		auto aEntity = aEntityWeak.lock();
		if (aEntity)
		{
			for (auto child : aEntity->GetChildren())
			{
				DeleteEntity(child);
			}
			aEntity->SetParent(0);

			auto it = std::find(myEntities.begin(), myEntities.end(), aEntity);

			if (it != myEntities.end())
			{
				std::scoped_lock mutexLock(myEntityListMutex);
				myEntities.erase(it);
				myEntitiesMap.erase(aEntity->GetID());
				SetAsModified();
			}
		}
	}
	void Scene::DeleteEntity(uint64_t aId)
	{
		auto ent = GetEntityByID(aId);

		if (ent.expired())
		{
			LOGERROR("Scene::DeleteEntity: Entity with id {} not found", aId);
			return;
		}
		DeleteEntity(ent);
	}

	void Scene::SetAsModified(bool aFlag)
	{
		myWasModifiedAfterSave = aFlag;
	}

	bool Scene::IsModified() const
	{
		return myWasModifiedAfterSave;
	}

	void Scene::UpdatePrefabs()
	{
		auto lastSize = myEntities.size();
		for (int i = 0; i < myEntities.size(); i++)
		{
			if (myEntities[i]->IsPrefabRoot())
			{
				Entity::UpdatePrefabValues(myEntities[i]);
			}

		}
	}


}
