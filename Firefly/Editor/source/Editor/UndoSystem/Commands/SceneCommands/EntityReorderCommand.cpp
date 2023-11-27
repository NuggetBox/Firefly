#include "EditorPch.h"
#include "EntityReorderCommand.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/Entity.h"

EntityReorderCommand::EntityReorderCommand(uint32_t indexToRemoveAt, uint32_t indexToInsertAt,
	Ptr<Firefly::Entity> aParentEntity, int aChildIndex, Firefly::Scene* aFromScene, Firefly::Scene* aToScene)
{
	myIndexToRemoveAt = indexToRemoveAt;
	myIndexToInsertAt = indexToInsertAt;

	myFromScene = aFromScene;
	myToScene = aToScene;

	myParent = aParentEntity;
	myChildIndex = aChildIndex;

	myPreviousParent = Ptr<Firefly::Entity>();
	myPreviousChildIndex = -1;

	myInsertIsBiggerThanRemoveIndexFlag = myIndexToInsertAt > myIndexToRemoveAt && myToScene == myFromScene;
}

void EntityReorderCommand::Execute()
{
	auto sceneEntities = myFromScene->GetEntities();

	auto it = sceneEntities.begin() + myIndexToRemoveAt;
	auto entity = it->lock();

	if (!entity)
	{
		LOGERROR("EntityReorderCommand::Execute: Entity is expired");
		return;
	}
	auto weakChildren = entity->GetChildrenRecursive();
	// should be a ref to make sure the children are not deconstructed since we are inserting them again later
	auto children = std::vector<Ref<Firefly::Entity>>();
	children.reserve(weakChildren.size());
	std::transform(weakChildren.begin(), weakChildren.end(), std::back_inserter(children), [](Ptr<Firefly::Entity> aWeakEntity)
		{
			return aWeakEntity.lock();
		});
	myFromScene->RemoveEntityINTERNALUSE(entity);
	//remove all chidren from scene
	for (int i = 0; i < children.size(); i++)
	{
		myFromScene->RemoveEntityINTERNALUSE(children[i]);
	}
	if (myInsertIsBiggerThanRemoveIndexFlag)
	{
		myIndexToInsertAt--; // remove the entity itself index
		myIndexToInsertAt -= children.size(); // remove all of the children index
	}

	myPreviousParent = entity->GetParent();
	if (!myPreviousParent.expired())
	{
		auto previousParent = myPreviousParent.lock();
		//loop through all children and find the index of the entity
		if (previousParent)
		{
			auto children = previousParent->GetChildren();
			for (int i = 0; i < children.size(); i++)
			{
				auto child = previousParent->GetChild(i).lock();
				if (child->GetID() == entity->GetID())
				{
					myPreviousChildIndex = i;
					break;
				}
			}
		}
	}

	Utils::BasicTransform previousLocal(entity->GetTransform().GetLocalPosition(), entity->GetTransform().GetLocalQuaternion(), entity->GetTransform().GetLocalScale());

	entity->SetParent(0, -1, false);
	//these set the parent scene of the entities
	myToScene->InsertEntity(entity, myIndexToInsertAt);
	myToScene->InsertEntities(children, myIndexToInsertAt + 1);

	entity->SetParent(myParent, myChildIndex, false);

	//Since we just called SetParent in the editor outside of a TransformCommand, we need to update the modifications
	if (entity->IsPrefab())
	{
		constexpr float modEpsilon = 0.00001f;
		if ((previousLocal.GetPosition() - entity->GetTransform().GetLocalPosition()).LengthSqr() > modEpsilon)
		{
			entity->UpdateTransformLocalPositionModification();
		}
		if (Utils::Abs(previousLocal.GetQuaternion().x - entity->GetTransform().GetLocalQuaternion().x) > modEpsilon ||
			Utils::Abs(previousLocal.GetQuaternion().y - entity->GetTransform().GetLocalQuaternion().y) > modEpsilon ||
			Utils::Abs(previousLocal.GetQuaternion().z - entity->GetTransform().GetLocalQuaternion().z) > modEpsilon ||
			Utils::Abs(previousLocal.GetQuaternion().w - entity->GetTransform().GetLocalQuaternion().w) > modEpsilon)
		{
			entity->UpdateTransformLocalRotationModification();
		}
		if ((previousLocal.GetScale() - entity->GetTransform().GetLocalScale()).LengthSqr() > modEpsilon)
		{
			entity->UpdateTransformLocalScaleModification();
		}
	}
}

void EntityReorderCommand::Undo()
{
	auto sceneEntities = myToScene->GetEntities();
	auto it = sceneEntities.begin() + myIndexToInsertAt;
	auto entityWeak = *it;
	if (entityWeak.expired())
	{
		LOGERROR("EntityReorderCommand::Undo: Entity is expired");
		return;
	}
	auto entity = entityWeak.lock();
	auto childrenWeak = entity->GetChildrenRecursive();
	auto children = std::vector<Ref<Firefly::Entity>>();
	children.reserve(childrenWeak.size());
	std::transform(childrenWeak.begin(), childrenWeak.end(), std::back_inserter(children), [](Ptr<Firefly::Entity> aWeakEntity)
		{
			return aWeakEntity.lock();
		});

	if (myInsertIsBiggerThanRemoveIndexFlag)
	{
		myIndexToInsertAt++; // add the entity itself index
		myIndexToInsertAt += children.size(); // remove all of the children index
	}
	myToScene->RemoveEntityINTERNALUSE(entityWeak);
	//remove all chidren from scene
	for (int i = 0; i < children.size(); i++)
	{
		myToScene->RemoveEntityINTERNALUSE(children[i]);
	}

	entity->SetParent(myPreviousParent, myPreviousChildIndex, false);
	myFromScene->InsertEntity(entity, myIndexToRemoveAt);
	myFromScene->InsertEntities(children, myIndexToRemoveAt + 1);

}

