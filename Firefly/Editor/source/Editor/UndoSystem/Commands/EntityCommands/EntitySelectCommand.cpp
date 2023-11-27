#include "EditorPch.h"
#include "EntitySelectCommand.h"

#include "Editor/EditorLayer.h"

EntitySelectCommand::EntitySelectCommand(const Ptr<Firefly::Entity>& aTargetEntity, bool aAddSelected)
{
	if (!aAddSelected)
	{
		myPreviousSelected = EditorLayer::GetSelectedEntities();
	}

	myTargetEntity = aTargetEntity;
	myAddSelected = aAddSelected;
}

void EntitySelectCommand::Execute()
{
	EditorLayer::SelectEntity(myTargetEntity, myAddSelected);
}

void EntitySelectCommand::Undo()
{
	if (myAddSelected)
	{
		EditorLayer::SelectEntity(myTargetEntity, myAddSelected);
	}
	else
	{
		EditorLayer::DeselectAllEntities();

		for (const auto& entity : myPreviousSelected)
		{
			EditorLayer::SelectEntity(entity, true);
		}
	}
}