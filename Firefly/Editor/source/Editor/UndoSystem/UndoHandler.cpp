#include "EditorPch.h"
#include "UndoHandler.h"
#include "Utils/InputHandler.h"
#include "Editor/UndoSystem/Commands/SeriesCommand.h"
#include "Firefly/Core/Log/DebugLogger.h"

void UndoHandler::Initialize()
{
	myUndoStack.reserve(myMaxUndoCount);
	myRedoStack.reserve(myMaxRedoCount);
}

void UndoHandler::ExecuteAndAdd(std::shared_ptr<UndoCommand> aCommand)
{
	aCommand->Execute();
	AddUndo(aCommand);
}

void UndoHandler::AddUndo(std::shared_ptr<UndoCommand> aCommand)
{
	//remove last if undo buffer is full
	if (myUndoStack.size() >= myMaxUndoCount)
	{
		myUndoStack.erase(myUndoStack.begin());
	}

	myRedoStack.clear();

	if (!myInSeries)
	{
		myUndoStack.push_back(aCommand);
	}
	else
	{
		mySeriesBuffer.push_back(aCommand);
	}
}

void UndoHandler::Undo()
{
	if (myUndoStack.size() > 0)
	{
		auto command = myUndoStack.back();
		command->Undo();
		myUndoStack.pop_back();

		if (myRedoStack.size() >= myMaxRedoCount)
		{
			myRedoStack.erase(myRedoStack.begin());
		}

		myRedoStack.push_back(command);
	}
}

void UndoHandler::Redo()
{
	if (myRedoStack.size() > 0)
	{
		auto command = myRedoStack.back();
		command->Execute();
		myRedoStack.pop_back();

		if (myUndoStack.size() >= myMaxUndoCount)
		{
			myUndoStack.erase(myUndoStack.begin());
		}

		myUndoStack.push_back(command);
	}
}

void UndoHandler::ClearUndo()
{
	myUndoStack.clear();
	myRedoStack.clear();
}

void UndoHandler::BeginSeries()
{
	assert(!myInSeries && "Tried to begin a series when already in a series");
	myInSeries = true;
}

void UndoHandler::EndSeries()
{
	assert(myInSeries && "Tried to end a series when not in a series");
	if (mySeriesBuffer.empty())
	{
		LOGWARNING("Ended a Undo series without adding any commands");
	}

	myInSeries = false;

	AddUndo(std::make_shared<SeriesCommand>(mySeriesBuffer));
	mySeriesBuffer.clear();
}

void UndoHandler::Update()
{
	const bool ctrlHeld = Utils::InputHandler::GetKeyHeld(VK_CONTROL);
	const bool shiftHeld = ImGui::IsKeyDown(ImGuiKey_LeftShift);
	const bool zPressed = ImGui::IsKeyPressed(ImGuiKey_Z, false);

	if (ctrlHeld)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Y, false) || shiftHeld && zPressed)
		{
			Redo();
		}
		else if (zPressed)
		{
			Undo();
		}
	}
}