#include "EditorPch.h"
#include "EntityCreateCommand.h"


#include "Editor/EditorLayer.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/Entity.h"

EntityCreateCommand::EntityCreateCommand(Ptr<Firefly::Entity> aParentEntity, Firefly::Scene* aScene, bool isPrefabEditorChild, bool aSelectOnSpawn)
{
	myScene = aScene;
	myStartParent = aParentEntity;
	mySelectOnSpawn = aSelectOnSpawn;
	myIsPrefabEditorChild = isPrefabEditorChild;
}

void EntityCreateCommand::Execute()
{
	if (!myCreatedEntity)
	{
		myCreatedEntity = myScene->Instantiate().lock();

		if (myIsPrefabEditorChild)
		{
			// should always have a parent if creating a new entity in a prefab
			myCreatedEntity->SetPrefabRootEntityID(myStartParent.lock()->GetPrefabRootEntityID());
		}
	}
	else
	{
		myScene->AddEntity(myCreatedEntity);
	}

	myCreatedEntity->SetParent(myStartParent);

	if (mySelectOnSpawn)
	{
		EditorLayer::SelectEntity(myCreatedEntity);
	}
}

void EntityCreateCommand::Undo()
{
	if (myScene)
	{
		myScene->DeleteEntity(myCreatedEntity);
		EditorLayer::DeselectAllEntities();
	}
}