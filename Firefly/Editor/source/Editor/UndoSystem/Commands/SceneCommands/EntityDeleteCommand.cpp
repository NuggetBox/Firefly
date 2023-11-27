#include "EditorPch.h"
#include "EntityDeleteCommand.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Editor/EditorLayer.h"

EntityDeleteCommand::EntityDeleteCommand(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("EntityDeleteCommand: Tried to delete an expired entity");
		return;
	}

	myDeleteEntity = aEntity.lock();
	myParentScene = myDeleteEntity->GetParentScene();
}

void EntityDeleteCommand::Execute()
{
	if (!myDeleteEntity)
	{
		LOGERROR("EntityDeleteCommand::Execute(): Entity is expired!");
		return;
	}
		
	auto sceneEntities = myParentScene->GetEntities();
	const auto it = std::find_if(sceneEntities.begin(), sceneEntities.end(), [this](const Ptr<Firefly::Entity>& entity)
	{
		return entity.lock()->GetID() == myDeleteEntity->GetID();
	});

	myDeleteIndex = static_cast<int>(it - sceneEntities.begin());

	if (!myDeleteEntity->GetParent().expired())
	{
		myPrevParent = myDeleteEntity->GetParent().lock();
	}

	myDeletedChildren.clear();
	const auto& childrenWeak = myDeleteEntity->GetChildrenRecursive();
	std::transform(childrenWeak.begin(), childrenWeak.end(), std::back_inserter(myDeletedChildren), 
		[](const std::weak_ptr<Firefly::Entity>& aWeakPtr) { return aWeakPtr.lock(); });

	myDeletedChildrenParentIDs.clear();
	for (const auto& child : myDeletedChildren)
	{
		if (!child)
		{
			LOGERROR("EntityDeleteCommand::Execute(): Child is expired!");
			continue;
		}

		myDeletedChildrenParentIDs.push_back(child->GetParent().lock()->GetID());
	}

	myParentScene->DeleteEntity(myDeleteEntity);
}

void EntityDeleteCommand::Undo()
{
	if (!myDeleteEntity)
	{
		LOGERROR("EntityDeleteCommand::Undo(): Entity is expired!");
		return;
	}

	auto currentScene = myParentScene;
	currentScene->InsertEntity(myDeleteEntity, myDeleteIndex);

	for (int childIndex = 0; childIndex < myDeletedChildren.size(); childIndex++)
	{
		const auto& child = myDeletedChildren[childIndex];
		if (!child)
		{
			LOGERROR("EntityDeleteCommand::Undo(): Child is expired!");
			continue;
		}
		currentScene->InsertEntity(child, myDeleteIndex + childIndex + 1);// +1 because the children should be after the parent in the scene
		child->SetParent(currentScene->GetEntityByID(myDeletedChildrenParentIDs[childIndex])); // can be right after since the children always are right after the parent
	}

	myDeleteEntity->SetParent(myPrevParent);
}