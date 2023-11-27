#pragma once

class UndoCommand
{
public:
	UndoCommand() = default;
	virtual ~UndoCommand() = default;

	virtual void Execute() = 0;
	virtual void Undo() = 0;
};