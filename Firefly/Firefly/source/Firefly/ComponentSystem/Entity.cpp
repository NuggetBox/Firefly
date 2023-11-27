#include "FFpch.h"
#include "Entity.h"

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/ComponentSystem/SceneManager.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Prefab.h"
#include <random>
#include <Utils/Timer.h>

namespace Firefly
{
	Entity::Entity(std::string aName, uint32_t aID, std::string aTag)
	{
		myParentScene = nullptr;
		myParentID = 0;
		myName = aName;
		myTag = aTag;
		myID = aID;
		myIsActiveFlag = true;
	}

	Entity::~Entity()
	{
#ifndef FF_INLAMNING
		LOGERROR("Entity was decontructed! Name: {}", myName);
#endif
	}

	Ptr<Entity> Entity::Duplicate(Ptr<Entity> aEntityToCopy, Scene* aParentScene, bool aNewIDFlag, uint64_t aPrefabRootEntityID)
	{
		if (aEntityToCopy.expired())
		{
			LOGERROR("Entity to copy is expired!");
			return Ptr<Entity>();
		}
		auto ent = std::make_shared<Entity>();
		auto entityToCopy = aEntityToCopy.lock();

		ent->myTransform.SetLocalPosition(entityToCopy->myTransform.GetLocalPosition());
		ent->myTransform.SetLocalRotation(entityToCopy->myTransform.GetLocalQuaternion());
		ent->myTransform.SetLocalScale(entityToCopy->myTransform.GetLocalScale());
		ent->myTransform.SetAsModified();

		if (aNewIDFlag)
		{
			ent->myID = GenerateRandomID();
		}
		else
		{
			ent->myID = entityToCopy->myID;
		}

		if (aParentScene == entityToCopy->myParentScene)
		{
			auto entities = entityToCopy->myParentScene->GetEntities();
			auto it = std::find_if(entities.begin(), entities.end(), [&](Ptr<Entity> aEntity)
				{
					return aEntity.lock()->GetID() == entityToCopy->myID;
				});
			int indexToInsertAt = it - entities.begin();
			indexToInsertAt += entityToCopy->GetRecursiveChildrenCount() + 1;
			aParentScene->InsertEntity(ent, indexToInsertAt);
		}
		else
		{
			aParentScene->AddEntity(ent);
		}

		uint64_t prefabRootEntityID = 0;
		if (entityToCopy->IsPrefab() && entityToCopy->GetPrefabRootEntityID() == entityToCopy->GetID())
		{
			prefabRootEntityID = ent->GetID();
			ent->SetPrefabRootEntityID(ent->GetID());
		}
		else if (aPrefabRootEntityID != 0)
		{
			prefabRootEntityID = aPrefabRootEntityID;
		}

		ent->SetCorrespondingSourceID(entityToCopy->GetCorrespondingSourceID());

		ent->SetPrefabID(entityToCopy->GetPrefabID());

		ent->SetPrefabRootEntityID(prefabRootEntityID);

		ent->myParentID = 0;

		auto mods = entityToCopy->GetModifications();

		if (prefabRootEntityID && entityToCopy->GetParentScene())
		{
			auto prefabRoot = entityToCopy->GetPrefabRoot().lock();
			if (prefabRoot)
			{
				mods = prefabRoot->GetModifications();
			}
		}

		for (auto& mod : mods)
		{
			if (mod.ID == entityToCopy->GetID())
			{
				mod.ID = ent->GetID();
				ent->AddModification(mod, true);
			}
		}

		ent->myName = entityToCopy->myName;
		ent->myTag = entityToCopy->myTag;
		ent->myIsActiveFlag = entityToCopy->myIsActiveFlag;

		auto componentsToCopy = entityToCopy->GetComponents();
		ent->myComponents.reserve(componentsToCopy.size());
		//ent->myComponentMap = entityToCopy->myComponentMap;
		for (int i = 0; i < componentsToCopy.size(); i++)
		{
			auto comp = Component::Duplicate(componentsToCopy[i], ent);
		}
		ent->myChildren.reserve(entityToCopy->myChildren.size());
		for (int i = 0; i < entityToCopy->myChildren.size(); i++)
		{
			auto childToCopy = entityToCopy->GetChild(i).lock();
			if (!childToCopy)
			{
				LOGERROR("Child to copy is expired! Skipping child!");
				continue;
			}

			auto child = Duplicate(childToCopy, aParentScene, aNewIDFlag, childToCopy->IsPrefab() ? prefabRootEntityID : 0).lock();
			if (child)
			{
				child->SetParent(ent, -1, true, false);
			}
		}

		return ent;
	}

	void Entity::UpdatePrefabValues(Ptr<Entity> aPrefabRoot)
	{
		if (aPrefabRoot.expired())
		{
			LOGERROR("Prefab root is expired!");
			return;
		}
		auto prefabRoot = aPrefabRoot.lock();

		if (!prefabRoot->IsPrefabRoot())
		{
			LOGERROR("Tried to Call UpdatePrefab on a non-prefab root entity. Entity ID: {}", prefabRoot->GetID());
			return;
		}

		auto prefabAsset = ResourceCache::GetAsset<Prefab>(prefabRoot->GetPrefabID(), true);
		if (!prefabAsset)
		{
			LOGERROR("The prefab was not found!");
			prefabRoot->SetName("MISSING PREFAB");
			return;
		}
		auto prefabAssetEntities = prefabAsset->GetEntities();
		auto children = prefabRoot->GetChildrenRecursive();
		//Check if the prefab root has all of it's children, if not add the ones not present
		//skip first entity since it is the one we are checking right now
		for (int prefabChildIndex = 1; prefabChildIndex < prefabAssetEntities.size(); prefabChildIndex++)
		{
			auto& prefabEnt = prefabAssetEntities[prefabChildIndex];
			bool found = false;
			for (auto& child : children)
			{
				if (child.expired())
				{
					LOGERROR("Child is expired!");
					continue;
				}
				if (child.lock()->GetCorrespondingSourceID() == prefabEnt->GetID())
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				auto newPrefabEnt = Entity::Duplicate(prefabEnt, prefabRoot->GetParentScene(), true, prefabRoot->GetID());

				if (newPrefabEnt.expired())
				{
					LOGERROR("Failed to duplicate prefab entity!");
					continue;
				}
				newPrefabEnt.lock()->SetCorrespondingSourceID(prefabEnt->GetID());
				if (prefabEnt->GetParentID() == prefabAssetEntities[0]->GetID())
				{
					newPrefabEnt.lock()->SetParent(aPrefabRoot);
				}
				else
				{
					//find the parent of the prefab entity
					auto prefabParentID = prefabEnt->GetParentID();
					//find the corresponding entity in the scene
					for (auto& child : prefabRoot->GetChildrenRecursive())
					{
						if (child.expired())
						{
							LOGERROR("Child is expired!");
							continue;
						}
						if (child.lock()->GetCorrespondingSourceID() == prefabParentID)
						{
							newPrefabEnt.lock()->SetParent(child);
							break;
						}
					}

				}
			}
		}


		auto scenePrefabEntities = prefabRoot->GetChildrenRecursive();
		//add the prefab root to the list since we also want to set the base values for it
		scenePrefabEntities.push_back(aPrefabRoot);
		for (auto& enta : scenePrefabEntities)
		{
			auto ent = enta.lock();
			if (!ent)
			{
				LOGERROR("Entity is expired!");
				continue;
			}
			//we dont want to set the values of children that dont belong to the root prefab
			if (ent->IsPrefab() && ent->GetPrefabRootEntityID() == prefabRoot->GetID())
			{
				// check if the entity has a corresponding source entity in the prefab asset. if it exists set values, if not delete it
				auto floundPrefabSourceIt = std::find_if(prefabAssetEntities.begin(), prefabAssetEntities.end(),
					[ent](const Ref<Entity>& aPrefabEnt) {return aPrefabEnt->GetID() == ent->GetCorrespondingSourceID(); });
				if (floundPrefabSourceIt != prefabAssetEntities.end())
				{
					//set the base values from the prefab on the entity
					ent->SetName((*floundPrefabSourceIt)->GetName());
					ent->SetTag((*floundPrefabSourceIt)->GetTag());
					ent->SetActive((*floundPrefabSourceIt)->GetIsActive());
					if (!ent->IsParameterModified("Transform_Pos"))
					{
						ent->GetTransform().SetLocalPosition((*floundPrefabSourceIt)->GetTransform().GetLocalPosition());
					}
					if (!ent->IsParameterModified("Transform_Rot"))
					{
						ent->GetTransform().SetLocalRotation((*floundPrefabSourceIt)->GetTransform().GetLocalRotation());
					}
					if (!ent->IsParameterModified("Transform_Scale"))
					{
						ent->GetTransform().SetLocalScale((*floundPrefabSourceIt)->GetTransform().GetLocalScale());
					}
					auto components = (*floundPrefabSourceIt)->GetComponents();
					for (auto sourceCompa : components)
					{
						auto sourceComp = sourceCompa.lock();
						if (!sourceComp)
						{
							LOGERROR("Component is expired!");
							continue;
						}
						Ptr<Component> foundComp = Ptr<Component>();
						if (!ent->HasComponent(sourceComp->GetName()))
						{
							foundComp = Component::Duplicate(sourceComp, ent);
						}
						else
						{
							foundComp = ent->GetComponent(sourceComp->GetName());
							if (foundComp.expired())
							{
								LOGERROR("Component not found!");
								continue;
							}
							//since we didnt duplicate the component we need to set the base values by hand. We also have to check if it ha been modified
							for (int paramIndex = 0; paramIndex < sourceComp->mySerializedVariables.size(); paramIndex++)
							{
								//the index is the same in both since they are the same component type
								const auto& sourceParam = sourceComp->mySerializedVariables[paramIndex];
								auto& param = foundComp.lock()->mySerializedVariables[paramIndex];

								//if the parameter has a local change, dont override it
								if (!ent->IsParameterModified(foundComp.lock()->GetName() + "_" + param.Name))
								{
									CopyParameterValue(param, sourceParam);
								}
							}

						}
						if (foundComp.expired())
						{
							LOGERROR("Component not found!");
							continue;
						}
						//have to go though entity parameters and link the corresponding id to the corresponding id in the scene
						//for (auto& param : foundComp.lock()->mySerializedVariables)
						for (int i = 0; i < Utils::Min(foundComp.lock()->mySerializedVariables.size(), sourceComp->mySerializedVariables.size()); i++)
						{
							auto& param = foundComp.lock()->mySerializedVariables[i];
							auto& prefabParam = sourceComp->mySerializedVariables[i];
							if (param.Type == Firefly::ParameterType::Entity)
							{
								// we dont want to override the local modification
								if (!ent->IsParameterModified(foundComp.lock()->GetName() + "_" + param.Name))
								{
									auto correspondingSourceID = prefabParam.EntityID;
									for (auto& correspondingSourceEnt : scenePrefabEntities)
									{
										if (correspondingSourceEnt.expired())
										{
											LOGERROR("Entity is expired!");
											continue;
										}
										if (correspondingSourceEnt.lock()->GetCorrespondingSourceID() == correspondingSourceID)
										{
											param.EntityID = correspondingSourceEnt.lock()->GetID();
											//set the entity parameter to the entity in the scene
											*reinterpret_cast<Ptr<Entity>*>(param.Pointer) = correspondingSourceEnt; /////
											break;
										}
									}
								}
							}
							else if (param.Type == Firefly::ParameterType::List)
							{
								if (param.ListType == Firefly::ParameterType::Entity)
								{
									size_t index = 0;
									auto paramVec = (*reinterpret_cast<std::vector<Ptr<Entity>>*>(param.Pointer));
									paramVec.resize(param.EntityIDVector.size());
									for (auto& idToSet : param.EntityIDVector)
									{
										for (auto& correspondingSourceEnt : scenePrefabEntities)
										{
											if (correspondingSourceEnt.expired())
											{
												LOGERROR("Entity is expired!");
												continue;
											}
											if (correspondingSourceEnt.lock()->GetCorrespondingSourceID() == idToSet)
											{
												idToSet = correspondingSourceEnt.lock()->GetID();
												//set the entity parameter to the entity in the scene
												paramVec[index] = correspondingSourceEnt; /////
												break;
											}
										}
										index++;
									}
								}
							}
						}
					}
				}
				//if the entity belonged to the prefab but was removed from it, we want to remove it from the scene
				else
				{
					prefabRoot->GetParentScene()->DeleteEntity(ent);

					continue;
				}
			}
		}


	}

	uint64_t Entity::GenerateRandomID()
	{
		//generate random uint64_t
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<uint64_t> dis;
		return  dis(gen);
	}

	bool Entity::GetIsActive() const
	{
		return myIsActiveFlag;
	}

	void Entity::EarlyInitialize()
	{
		FF_PROFILESCOPE("Entity Early Initialize");

		for (int i = 0; i < myComponents.size(); i++)
		{
			FF_PROFILESCOPE((myComponents[i]->GetName() + " Component Early Initialize").c_str());

			myComponents[i]->EarlyInitialize();
		}
	}

	void Entity::Initialize()
	{
		FF_PROFILESCOPE("Entity Initialize");

		ResetLifeTime();

		for (int i = 0; i < myComponents.size(); i++)
		{
			FF_PROFILESCOPE((myComponents[i]->GetName() + " Component Initialize").c_str());

			myComponents[i]->Initialize();
		}
	}

	void Entity::SetActive(bool aActiveFlag)
	{
		myIsActiveFlag = aActiveFlag;
		EntityOnEnableEvent e(myIsActiveFlag);
		OnEvent(e);
		for (auto child : GetChildren())
		{
			if (child.expired())
			{
				LOGERROR("Entity::SetActive: Child is expired!");
				continue;
			}
			if (child.lock())
				child.lock()->SetActive(aActiveFlag);
		}
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	void Entity::OnEvent(Firefly::Event& aEvent)
	{
		std::string profilerThing = "Entity Event : " + aEvent.ToString() + ' ' + myName.c_str();

		//FF_PROFILESCOPE(profilerThing.c_str());

		if (myIsActiveFlag)
		{
			for (int i = 0; i < myComponents.size(); i++)
			{
				if (!myComponents[i])
				{
					LOGERROR("Scene::OnEvent: Entity is null!");
					continue;
				}
				if (myComponents[i]->myIsActive)
				{
					//FF_PROFILESCOPE((myComponents[i]->GetName() + " Component Event: " + aEvent.ToString()).c_str());

					myComponents[i]->OnEvent(aEvent);
				}
			}
		}
	}

	const std::string& Entity::GetName() const
	{
		return myName;
	}

	const std::string& Entity::GetTag() const
	{
		return myTag;
	}

	float Entity::GetCreationTime() const
	{
		return myCreationTime;
	}

	void Entity::ResetLifeTime()
	{
		myCreationTime = Utils::Timer::GetScaledTotalTime();
	}

	float Entity::GetLifeTime() const
	{
		return Utils::Timer::GetScaledTotalTime() - GetCreationTime();
	}

	uint64_t Entity::GetID() const
	{
		return myID;
	}

	void Entity::SetName(const std::string& aName)
	{
		myName = aName;
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	void Entity::SetTag(const std::string& aTag)
	{
		myTag = aTag;
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	void Entity::SetParent(Ptr<Entity> aParent, int aChildIndex, bool aShouldMove, bool aKeepWorldTransformations)
	{
		if (!aParent.expired())
		{
			if (aParent.lock()->GetParentScene() != myParentScene)
			{
				myParentScene = aParent.lock()->GetParentScene();
			}
			SetParent(aParent.lock()->GetID(), aChildIndex, aShouldMove, aKeepWorldTransformations);
		}
		else
		{
			SetParent(0, -1, aShouldMove, aKeepWorldTransformations);
		}
	}

	void Entity::SetParent(uint64_t aParentID, int aChildIndex, bool aShouldMove, bool aKeepWorldTransformations)
	{
		//make sure you arent setting an entity as its own child
		auto testParent = myParentScene->GetEntityByID(aParentID).lock();
		bool invalid = false;
		while (testParent)
		{
			if (testParent->GetID() == myID)
			{
				invalid = true;
				break;
			}
			testParent = testParent->GetParent().lock();
		}
		if (invalid)
		{
			return;
		}

		auto parent = std::shared_ptr<Entity>();
		if (myParentID != 0)
		{
			parent = GetParent().lock();
			if (!parent)
			{
				LOGERROR("Entity::SetParent: Parent is expired!");
				return;
			}
			if (parent)
			{
				//remove the transform relation from current parent
				parent->myTransform.RemoveChild(&myTransform);
				//find the child in the current parent's children and remove it
				auto itChild = std::find(parent->myChildren.begin(), parent->myChildren.end(), myID);
				parent->myChildren.erase(itChild);

				if (aShouldMove)
				{
					//find the entity in the current scene and reorder it to maintin children being right after their parent in the scene
					auto ent = myParentScene->GetEntityByID(myID).lock(); // lock to make sure it doesnt get deleted

					myParentScene->RemoveEntityINTERNALUSE(ent);

					//insert this entity at the index right after the last 
					auto sceneEntities = myParentScene->GetEntities();
					auto itEnt = std::find_if(sceneEntities.begin(), sceneEntities.end(), [parent](const std::weak_ptr<Entity>& aEntity)
						{
							return aEntity.lock()->GetID() == parent->GetID();
						});
					auto index = static_cast<int>(std::distance(sceneEntities.begin(), itEnt));
					myParentScene->InsertEntity(ent, index + parent->GetRecursiveChildrenCount() + 1);
				}

			}
		}

		myParentID = aParentID;
		//have to get parent again since ID was changed
		parent = GetParent().lock();

		if (aChildIndex == -1)
		{
			if (parent)
			{
				aChildIndex = parent->myChildren.size();
			}
		}
		if (myParentID != 0)
		{
			if (!parent)
			{
				LOGERROR("Entity::SetParent: New Parent is expired!");
				return;
			}
			myTransform.SetParent(&parent->myTransform, aKeepWorldTransformations);
		}
		else
		{
			myTransform.SetParent(nullptr, aKeepWorldTransformations);
		}

		//add as child to parent
		if (parent)
		{
			assert(aChildIndex >= 0 && aChildIndex <= parent->myChildren.size());
			parent->myTransform.AddChild(&myTransform);

			if (aShouldMove)
			{
				//reorder this entity to maintain the order in the scene

				//find this entity in the scene and remove ir
				auto ent = myParentScene->GetEntityByID(myID).lock(); // lokced so it doesnt get deleted
				myParentScene->RemoveEntityINTERNALUSE(ent);

				auto childrenWeak = ent->GetChildrenRecursive();
				auto children = std::vector<std::shared_ptr<Entity>>();
				//transform weak ptrs to shared ptrs using std::transform to avoid them getting destructed
				std::transform(childrenWeak.begin(), childrenWeak.end(), std::back_inserter(children), [](const std::weak_ptr<Entity>& aWeakPtr) { return aWeakPtr.lock(); });


				for (int i = 0; i < children.size(); i++)
				{
					myParentScene->RemoveEntityINTERNALUSE(children[i]);
				}

				//find the parent in the scene and insert this entity after it according to the given child index 
				auto sceneEntities = myParentScene->GetEntities();
				auto itEnt = std::find_if(sceneEntities.begin(), sceneEntities.end(), [parent](const std::weak_ptr<Entity>& aEntity)
					{
						return aEntity.lock()->GetID() == parent->GetID();
					});
				//find recursive how many children are before this entity
				auto parentChildren = parent->GetChildren();

				int extraInsertOffset = 0;
				for (int i = 0; i < aChildIndex; i++)
				{
					extraInsertOffset += parentChildren[i].lock()->GetRecursiveChildrenCount();
				}

				int parentIndex = itEnt - sceneEntities.begin();
				int index = (itEnt - sceneEntities.begin()) + extraInsertOffset + 1;
				// +1 because the parent is at itEnt and we want to insert after and child index begins at 0
				auto insertIt = itEnt + 1 + aChildIndex + extraInsertOffset;
				int insertIndex = insertIt - sceneEntities.begin();

				//reinsert the entities
				myParentScene->InsertEntity(ent, insertIndex);
				myParentScene->InsertEntities(children, insertIndex + 1);
			}
			//insert the child ID in the parent's children vector
			auto it = parent->myChildren.begin() + aChildIndex;
			parent->myChildren.insert(it, myID);
		}
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	bool Entity::HasParent() const
	{
		return myParentID != 0;
	}

	Ptr<Entity> Entity::GetParent()
	{
		return myParentScene->GetEntityByID(myParentID);
	}

	uint64_t Entity::GetParentID()
	{
		return myParentID;
	}

	std::vector<Ptr<Entity>> Entity::GetChildren()
	{


		std::vector<Ptr<Entity>> children;
		for (int i = 0; i < myChildren.size(); i++)
		{
			auto child = myParentScene->GetEntityByID(myChildren[i]).lock();
			if (child)
			{
				children.push_back(child);
			}
			else
			{
				LOGERROR("Entity::GetChildren: Child is expired!");
			}
		}
		return children;
	}
	bool Entity::HasComponent(const std::string& aComponentFactoryName)
	{
		if (myComponentMap.contains(aComponentFactoryName))
		{
			return true;
		}
		return false;
	}
	void Entity::SetAllComponentsActive(const bool& aIsActive)
	{
		for (size_t i = 0; i < myComponents.size(); i++)
		{
			EntityOnComponentEnableEvent e(aIsActive);
			myComponents[i]->OnEvent(e);
			myComponents[i]->myIsActive = aIsActive;
		}
	}
	bool Entity::RemoveComponent(const std::string& aFactoryName)
	{
		auto it = myComponentMap.find(aFactoryName);
		if (it == myComponentMap.end())
		{
			return false;
		}

		auto comp = it->second;
		comp->OnRemove();
		myComponentMap.erase(it);

		//find and erase from myComponents
		for (auto vecIt = myComponents.begin(); vecIt != myComponents.end(); ++vecIt)
		{
			if (*vecIt == comp)
			{
				myComponents.erase(vecIt);
				return true;
			}
		}
		return false;
	}

	int Entity::GetRecursiveChildrenCount()
	{
		int count = 0;
		auto children = GetChildren();
		while (children.size() > 0)
		{
			auto child = *children.begin();
			if (child.expired())
			{
				LOGERROR("Entity::GetRecursiveChildrenCount: Child is expired!");
			}
			for (auto c : child.lock()->GetChildren())
			{
				children.push_back(c);
			}
			children.erase(children.begin());
			count++;
		}
		return count;
	}

	std::vector<Ptr<Entity>> Entity::GetChildrenRecursive()
	{
		std::vector<Ptr<Entity>> children;
		auto childrenVec = GetChildren();
		while (childrenVec.size() > 0)
		{
			auto child = *childrenVec.begin();
			if (child.expired())
			{
				LOGERROR("Entity::GetChildrenRecursive: Child is expired!");
			}
			int index = 1;
			for (auto c : child.lock()->GetChildren())
			{
				childrenVec.insert(childrenVec.begin() + index, c);
				index++;
			}
			children.push_back(child);
			childrenVec.erase(childrenVec.begin());
		}
		return children;
	}

	Ptr<Entity> Entity::GetChild(uint32_t aIndex)
	{
		return myParentScene->GetEntityByID(myChildren[aIndex]);
	}

	Ptr<Component> Entity::GetComponent(const std::string& aComponentName)
	{
		if (!myComponentMap.contains(aComponentName))
		{
			LOGWARNING("Entity::GetComponent: Entity with name \"{}\" and ID \"{}\"does not have component of type: {}", GetName(), GetID(), aComponentName);
			return Ptr<Component>();
		}
		return myComponentMap[aComponentName];
	}
	std::vector<Ptr<Component>> Entity::GetComponents()
	{
		auto components = std::vector<Ptr<Component>>();
		std::transform(myComponents.begin(), myComponents.end(), std::back_inserter(components), [](auto& comp) { return comp; });
		return components;
	}

	bool Entity::AddComponent(std::shared_ptr<Component> aComponent, bool aShouldInitialize)
	{
		if (aComponent)
		{
			std::string name = aComponent->GetName();

			auto it = myComponentMap.find(name);
			if (it == myComponentMap.end())
			{
				if (myParentScene)
				{
					myParentScene->SetAsModified();
				}
				aComponent->myEntity = this;

				myComponents.push_back(aComponent);
				myComponentMap[name] = aComponent;

				if (aShouldInitialize)
				{
					aComponent->EarlyInitialize();
					aComponent->Initialize();
				}
				return true;
			}
		}
		return false;
	}

	void Entity::RemoveComponent(Ptr<Component> aComponent)
	{
		if (aComponent.expired())
		{
			LOGERROR("Entity::RemoveComponent: Component is expired!");
			return;
		}
		std::string name = aComponent.lock()->GetName();
		auto it = std::find(myComponents.begin(), myComponents.end(), aComponent.lock());
		if (it != myComponents.end())
		{
			myComponents.erase(it);
			myComponentMap.erase(name);
		}
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	Utils::Transform& Entity::GetTransform()
	{
		return myTransform;
	}

	uint64_t Entity::GetCorrespondingSourceID()
	{
		return myCorrespondingSourceID;
	}

	void Entity::SetPrefabID(uint64_t aID)
	{
		myPrefabID = aID;
		if (myParentScene)
		{
			myParentScene->SetAsModified();
		}
	}

	uint64_t Entity::GetPrefabRootEntityID()
	{
		return myPrefabRootEntityID;
	}

	void Entity::SetPrefabRootEntityID(uint64_t aRootID)
	{
		myPrefabRootEntityID = aRootID;
	}

	Ptr<Entity> Entity::GetPrefabRoot()
	{
		if (myPrefabRootEntityID != 0)
		{
			//recursive parent check to find the root prefab parent
			auto parent = myParentScene->GetEntityByID(myPrefabRootEntityID);
			if (parent.expired())
			{
				LOGERROR("Entity::GetPrefabRoot: Parent is expired!");
				return {};
			}
			if (parent.lock()->GetID() == myPrefabRootEntityID)
			{
				return parent;
			}
		}

		return {};
	}

	void Entity::UnpackPrefab()
	{
		if (IsPrefabRoot())
		{
			myPrefabRootEntityID = 0;
			myPrefabID = 0;
			myCorrespondingSourceID = 0;
			for (auto& childa : GetChildrenRecursive())
			{
				if (childa.expired())
				{
					LOGERROR("Entity::UnpackPrefab: Child is expired!");
					continue;
				}
				auto child = childa.lock();
				if (child->IsPrefab() && child->GetPrefabRootEntityID() == myID)
				{
					child->myPrefabRootEntityID = 0;
					child->myPrefabID = 0;
					child->myCorrespondingSourceID = 0;
				}
			}
			myPrefabModifications.clear();
		}
		else
		{
			LOGERROR("Tried to unpack prefab on an entity that is not a prefab root");
		}
	}

	void Entity::AddModification(const EntityModification& aModification, bool aAddToRoot)
	{
		if (!IsPrefab())
			return;

		auto* modifications = &myPrefabModifications;

		if (aAddToRoot)
		{
			const auto& rootWeak = GetPrefabRoot();
			if (!rootWeak.expired())
			{
				const auto& root = rootWeak.lock();
				modifications = &root->myPrefabModifications;
			}
		}

		//make sure we don't add the same modification twice
		for (auto& mod : *modifications)
		{
			if (mod.ID == aModification.ID && mod.Key == aModification.Key)
			{
				mod = aModification;
				return;
			}
		}

		(*modifications).push_back(aModification);
	}

	void Entity::RevertModification(Variable& aParam, const Component* const aComponent)
	{
		std::string key = aComponent->GetName() + "_" + aParam.Name;
		auto weakRoot = GetPrefabRoot();
		if (weakRoot.expired())
		{
			LOGERROR("Entity::RevertModification: Prefab root is expired!");
			return;
		}
		auto root = weakRoot.lock();
		auto prefab = Firefly::ResourceCache::GetAsset<Prefab>(root->GetPrefabID(), true);
		for (auto it = root->myPrefabModifications.begin(); it != root->myPrefabModifications.end(); ++it)
		{
			if (it->Key == key && it->ID == myID)
			{
				//Get prefab parameter
				for (auto& prefabEnt : prefab->GetEntities())
				{
					if (prefabEnt->GetID() == myCorrespondingSourceID)
					{
						auto weakComp = prefabEnt->GetComponent(aComponent->GetName());
						if (weakComp.expired())
						{
							LOGERROR("Entity::RevertModification: Component is expired!");
							return;
						}
						auto comp = weakComp.lock();
						//Should be copy of parameter
						for (auto& param : comp->mySerializedVariables)
						{
							if (param.Name == aParam.Name)
							{
								for (auto& weakChild : root->GetChildrenRecursive())
								{
									if (weakChild.expired())
									{
										LOGERROR("Entity::RevertModification: Child is expired!");
										continue;
									}
									auto child = weakChild.lock();
									if (child->IsPrefab() && child->GetPrefabRootEntityID() == root->GetID() && child->GetCorrespondingSourceID() == param.EntityID)
									{
										param.EntityID = child->GetID();
									}
								}
								CopyParameterValue(aParam, param, myParentScene, true);
							}
						}

					}
				}

				root->myPrefabModifications.erase(it);
				return;
			}
		}

	}
	void Entity::RemoveModification(const std::string& key, uint64_t aID)
	{
		for (auto it = myPrefabModifications.begin(); it != myPrefabModifications.end(); ++it)
		{
			if (it->Key == key && it->ID == aID)
			{
				myPrefabModifications.erase(it);
				return;
			}
		}
	}
	const std::vector<EntityModification>& Entity::GetModifications() const
	{
		return myPrefabModifications;
	}

	//todo: unoptimized since it does std::find every time, dont have time to figure out a better way
	bool Entity::IsParameterModified(const std::string& aKey)
	{
		if (!IsPrefab())
		{
			LOGERROR("Tried to check if a parameter was modified on a non-prefab entity");
			return false;
		}
		Entity* targetEnt = this;
		if (!targetEnt)
		{
			LOGERROR("Entity::IsParameterModified: Target entity is null!");
			return false;
		}

		if (myPrefabRootEntityID != myID)
		{
			targetEnt = GetPrefabRoot().lock().get();
		}
		auto it = std::find_if(targetEnt->myPrefabModifications.begin(), targetEnt->myPrefabModifications.end(), [this, &aKey](const EntityModification& aMod)
			{
				return aMod.ID == myID && aMod.Key == aKey;
			});
		return it != targetEnt->myPrefabModifications.end();
	}

	void Entity::UpdateTransformLocalPositionModification()
	{
		if (!IsPrefab())
		{
			LOGERROR("Cannot update local position modification on Non-Prefabs!");
			return;
		}

		float x = GetTransform().GetLocalPosition().x;
		float y = GetTransform().GetLocalPosition().y;
		float z = GetTransform().GetLocalPosition().z;
		Firefly::EntityModification mod;
		mod.ID = GetID();
		mod.Key = "Transform_Pos";
		mod.FloatValues.push_back(x);
		mod.FloatValues.push_back(y);
		mod.FloatValues.push_back(z);
		GetPrefabRoot().lock()->AddModification(mod, true);
	}

	void Entity::UpdateTransformLocalRotationModification()
	{
		if (!IsPrefab())
		{
			LOGERROR("Cannot update local rotation modification on Non-Prefabs!");
			return;
		}

		float x = GetTransform().GetLocalQuaternion().x;
		float y = GetTransform().GetLocalQuaternion().y;
		float z = GetTransform().GetLocalQuaternion().z;
		float w = GetTransform().GetLocalQuaternion().w;
		Firefly::EntityModification mod;
		mod.ID = GetID();
		mod.Key = "Transform_Rot";
		mod.FloatValues.push_back(x);
		mod.FloatValues.push_back(y);
		mod.FloatValues.push_back(z);
		mod.FloatValues.push_back(w);
		GetPrefabRoot().lock()->AddModification(mod, true);
	}

	void Entity::UpdateTransformLocalScaleModification()
	{
		if (!IsPrefab())
		{
			LOGERROR("Cannot update local scale modification on Non-Prefabs!");
			return;
		}

		float x = GetTransform().GetLocalScale().x;
		float y = GetTransform().GetLocalScale().y;
		float z = GetTransform().GetLocalScale().z;
		Firefly::EntityModification mod;
		mod.ID = GetID();
		mod.Key = "Transform_Scale";
		mod.FloatValues.push_back(x);
		mod.FloatValues.push_back(y);
		mod.FloatValues.push_back(z);
		GetPrefabRoot().lock()->AddModification(mod, true);
	}
}