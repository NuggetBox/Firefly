#include "EditorPch.h"
#include "EntityDeselectAllCommand.h"

#include "Editor/EditorLayer.h"

EntityDeselectAllCommand::EntityDeselectAllCommand()
{
	myDeselectedEntities = EditorLayer::GetSelectedEntities();
}

void EntityDeselectAllCommand::Execute()
{
	myDeselectedEntities = EditorLayer::GetSelectedEntities();
	EditorLayer::DeselectAllEntities();
}

void EntityDeselectAllCommand::Undo()
{
	EditorLayer::DeselectAllEntities();
	for (const auto& entity : myDeselectedEntities)
	{
		EditorLayer::SelectEntity(entity, true);
	}
}