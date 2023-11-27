#include "EditorPch.h"
#include "PrefabCreateCommand.h"

#include "Editor/EditorLayer.h"
#include "Firefly/ComponentSystem/Scene.h"

PrefabCreateCommand::PrefabCreateCommand(Ref<Firefly::Prefab> aPrefab, Ptr<Firefly::Entity> aParentEntity, Firefly::Scene* aScene, bool aSelectOnSpawn)
{
	myPrefab = aPrefab;
	myStartParent = aParentEntity;
	myScene = aScene;
	mySelectOnSpawn = aSelectOnSpawn;
}

void PrefabCreateCommand::Execute()
{
	if (!myCreatedEntity)
	{
		myCreatedEntity = myScene->Instantiate(myPrefab).lock();
	}
	else
	{
		//Add entity and its children back in original place
		myScene->AddEntity(myCreatedEntity);
		auto children = myCreatedEntity->GetChildrenRecursive();

		for (int childIndex = 0; childIndex < myDuplicatedChildren.size(); childIndex++)
		{
			myScene->InsertEntity(myDuplicatedChildren[childIndex], myScene->GetEntities().size() + childIndex);
			myDuplicatedChildren[childIndex]->SetParent(myScene->GetEntityByID(myDuplicatedChildrenParentIDs[childIndex])); // can be right after since the children always are right after the parent
		}
		//

		myCreatedEntity->EarlyInitialize();
		for (auto& child : myDuplicatedChildren)
		{
			child->EarlyInitialize();
		}
		myCreatedEntity->Initialize();
		for (auto& child : myDuplicatedChildren)
		{
			child->Initialize();
		}
	}

	if (mySelectOnSpawn)
	{
		EditorLayer::SelectEntity(myCreatedEntity);
	}
}

void PrefabCreateCommand::Undo()
{
	if (myScene)
	{
		auto children = myCreatedEntity->GetChildrenRecursive();
		myDuplicatedChildren.resize(children.size());
		std::transform(children.begin(), children.end(), myDuplicatedChildren.begin(), [](Ptr<Firefly::Entity> aEntity) { return aEntity.lock(); });
		for (auto child : myDuplicatedChildren)
		{
			myDuplicatedChildrenParentIDs.push_back(child->GetParent().lock()->GetID());
		}

		myScene->DeleteEntity(myCreatedEntity);
		EditorLayer::DeselectAllEntities();
	}
}