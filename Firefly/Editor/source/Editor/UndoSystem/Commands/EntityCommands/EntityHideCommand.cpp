#include "EditorPch.h"
#include "EntityHideCommand.h"

#include "Editor/EditorLayer.h"

#include "Firefly/ComponentSystem/Entity.h"

EntityHideCommand::EntityHideCommand(const Ptr<Firefly::Entity>& aTargetEntity, bool aHide)
{
	myTargetEntity = aTargetEntity;
	myHide = aHide;
}

void EntityHideCommand::Execute()
{
	if (!myTargetEntity.expired())
	{
		myTargetEntity.lock()->SetActive(!myHide);
	}
}

void EntityHideCommand::Undo()
{
	if (!myTargetEntity.expired())
	{
		myTargetEntity.lock()->SetActive(myHide);
	}
}