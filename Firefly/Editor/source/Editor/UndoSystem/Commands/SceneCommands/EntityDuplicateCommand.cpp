#include "EditorPch.h"
#include "EntityDuplicateCommand.h"

#include "Editor/EditorLayer.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/Entity.h"


EntityDuplicateCommand::EntityDuplicateCommand(Ptr<Firefly::Entity> aEntity, bool aNewIDFlag)
{
	myEntityToDuplicate = aEntity;
	myNewIDFlag = aNewIDFlag;
	myDuplicatedAtIndex = 0;
	myParentScene = aEntity.lock()->GetParentScene();
}

void EntityDuplicateCommand::Execute()
{
	if(myEntityToDuplicate.expired())
	{
		LOGERROR("EntityDuplicateCommand::Execute: Entity to duplicate is expired!");
		return;
	}
	auto scene = myParentScene;

	//Command has not been executed yet
	if (!myDuplicatedEntity)
	{
		//Duplicate the entity and save where it was inserted after duplication
		myDuplicatedEntity = Firefly::Entity::Duplicate(myEntityToDuplicate, scene, myNewIDFlag).lock();

		auto entities = scene->GetEntities();
		auto it = std::find_if(entities.begin(), entities.end(), [this](Ptr<Firefly::Entity> entity) { return entity.lock()->GetID() == myDuplicatedEntity->GetID(); });
		myDuplicatedAtIndex = it - entities.begin();
		//
	}
	//Command was already executed
	else
	{
		//Insert entity and its children back in original place
		scene->InsertEntity(myDuplicatedEntity, myDuplicatedAtIndex);
		auto children = myDuplicatedEntity->GetChildrenRecursive();

		for (int childIndex = 0; childIndex < myDuplicatedChildren.size(); childIndex++)
		{
			scene->InsertEntity(myDuplicatedChildren[childIndex], myDuplicatedAtIndex + childIndex + 1);// +1 because the children should be after the parent in the scene
			myDuplicatedChildren[childIndex]->SetParent(scene->GetEntityByID(myDuplicatedChildrenParentIDs[childIndex])); // can be right after since the children always are right after the parent
		}
		//
	}

	auto duplicatedEntity = myDuplicatedEntity;
	duplicatedEntity->SetParent(myEntityToDuplicate.lock()->GetParent(), -1, true, false);

	//Initialize entity and children
	duplicatedEntity->EarlyInitialize();
	for (auto& child : duplicatedEntity->GetChildrenRecursive())
	{
		if (child.expired())
		{
			LOGERROR("EntityDuplicateCommand::Execute(): Child is expired");
			continue;
		}
		child.lock()->EarlyInitialize();
	}
	duplicatedEntity->Initialize();
	for (auto& child : duplicatedEntity->GetChildrenRecursive())
	{
		if (child.expired())
		{
			LOGERROR("EntityDuplicateCommand::Execute(): Child is expired");
			continue;
		}
		child.lock()->Initialize();
	}
	//

	myPrevSelectedEntities = EditorLayer::GetSelectedEntities();
	EditorLayer::SelectEntity(duplicatedEntity);
}

void EntityDuplicateCommand::Undo()
{
	if (!myDuplicatedEntity)
	{
		LOGERROR("EntityDuplicateCommand::Undo: Duplicated entity is expired!");
		return;
	}
	auto scene = myParentScene;

	auto children = myDuplicatedEntity->GetChildrenRecursive();
	myDuplicatedChildren.clear();
	std::transform(children.begin(), children.end(), std::back_inserter(myDuplicatedChildren), [](Ptr<Firefly::Entity> entity) { return entity.lock(); });
	for (const auto& child : myDuplicatedChildren)
	{
		if (!child)
		{
			LOGERROR("EntityDuplicateCommand::Undo: Child is expired!");
			continue;
		}
		myDuplicatedChildrenParentIDs.push_back(child->GetParent().lock()->GetID());
	}

	scene->DeleteEntity(myDuplicatedEntity);

	EditorLayer::DeselectAllEntities();

	for (auto& entity : myPrevSelectedEntities)
	{
		EditorLayer::SelectEntity(entity, true);
	}
}