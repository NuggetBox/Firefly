#pragma once
#include "Editor/UndoSystem/UndoHandler.h"
class SeriesCommand : public UndoCommand
{

public:
	SeriesCommand(std::vector<std::shared_ptr< UndoCommand>> someReorderCommands);
	// Inherited via UndoCommand
	virtual void Execute() override;
	virtual void Undo() override;

private:
	std::vector<std::shared_ptr< UndoCommand>> myCommands;
};