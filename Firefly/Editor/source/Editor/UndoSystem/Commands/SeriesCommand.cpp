#include "EditorPch.h"
#include "SeriesCommand.h"

SeriesCommand::SeriesCommand(std::vector<std::shared_ptr<UndoCommand>> someReorderCommands)
{
	myCommands = someReorderCommands;
}

void SeriesCommand::Execute()
{
	for (int i = 0; i < myCommands.size(); i++)
	{
		myCommands[i]->Execute();
	}
}

void SeriesCommand::Undo()
{
	for (int i = myCommands.size() - 1; i >= 0; i--)
	{
		myCommands[i]->Undo();
	}
}
