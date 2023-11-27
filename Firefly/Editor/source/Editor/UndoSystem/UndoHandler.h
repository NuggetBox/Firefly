#pragma once
#include "Editor/UndoSystem/UndoCommand.h"

class UndoHandler
{
public:
	void Initialize();
	/// <summary>
	/// Checks for inputs
	/// </summary>
	void Update();

	void ExecuteAndAdd(std::shared_ptr<UndoCommand> aCommand);
	void AddUndo(std::shared_ptr<UndoCommand> aCommand);
	void Undo();
	void Redo();
	void ClearUndo();

	void BeginSeries();
	void EndSeries();

private:
	std::vector<std::shared_ptr<UndoCommand>> myUndoStack;
	std::vector<std::shared_ptr<UndoCommand>> myRedoStack;

	std::vector<std::shared_ptr<UndoCommand>> mySeriesBuffer;
	bool myInSeries = false;

	static constexpr int myMaxUndoCount = 1024;
	static constexpr int myMaxRedoCount = 1024;
};
