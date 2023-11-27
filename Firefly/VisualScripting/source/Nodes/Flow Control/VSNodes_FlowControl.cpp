#include "VSNodes_FlowControl.h"

#include "Utils/StringUtils.hpp"
#include "Utils/TimerManager.h"

void VSNode_Branch::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("True", PinDirection::Output);
	CreateExecPin("False", PinDirection::Output);
	CreateDataPin<bool>("Condition", PinDirection::Input);

	SetPinData("Condition", true);
}

size_t VSNode_Branch::DoOperation()
{
	bool condition;

	if (GetPinData("Condition", condition))
	{
		if (condition)
		{
			return ExitViaPin("True");
		}
		else
		{
			return ExitViaPin("False");
		}
	}

	return 0;
}

void VSNode_DoOnce::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Reset", PinDirection::Input);

	CreateExecPin("Out", PinDirection::Output, true);
}

size_t VSNode_DoOnce::DoOperation()
{
	assert(false);
	return 0; //SHOULDNT RUN
}

size_t VSNode_DoOnce::Exec(size_t anEntryPinUID)
{
	const std::string& label = GetPin(anEntryPinUID).GetLabel();

	if (label == "In")
	{
		if (myHasExecuted)
		{
			return 0;
		}
		else
		{
			myHasExecuted = true;
			return ExitViaPin("Out");
		}
	}
	else if (label == "Reset")
	{
		myHasExecuted = false;
	}

	return 0;
}

void VSNode_ForLoop::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Break", PinDirection::Input);
	CreateDataPin<int>("First Index", PinDirection::Input);
	CreateDataPin<int>("Last Index", PinDirection::Input);

	CreateExecPin("Loop", PinDirection::Output);
	CreateDataPin<int>("Index", PinDirection::Output);
	CreateExecPin("Completed", PinDirection::Output);
}

size_t VSNode_ForLoop::DoOperation()
{
	int firstIndex, lastIndex;

	if (GetPinData("First Index", firstIndex) && GetPinData("Last Index", lastIndex))
	{
		for (int i = firstIndex; i <= lastIndex; ++i)
		{
			SetPinData("Index", i);
			ExitViaPin("Loop");
		}

		return ExitViaPin("Completed");
	}

	return 0;
}

void VSNode_WhileLoop::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateDataPin<bool>("Condition", PinDirection::Input);

	CreateExecPin("Loop", PinDirection::Output);
	CreateExecPin("Completed", PinDirection::Output);
}

size_t VSNode_WhileLoop::DoOperation()
{
	bool condition;

	if (GetPinData("Condition", condition))
	{
		while (condition)
		{
			ExitViaPin("Loop");
			GetPinData("Condition", condition);
		}

		return ExitViaPin("Completed");
	}

	return 0;
}

void VSNode_Sequence::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("First This", PinDirection::Output);
	CreateExecPin("Then This 1", PinDirection::Output);
	CreateExecPin("Then This 2", PinDirection::Output);
	CreateExecPin("Then This 3", PinDirection::Output);
	CreateExecPin("Lastly This", PinDirection::Output);
}

size_t VSNode_Sequence::DoOperation()
{
	ExitViaPin("First This");
	ExitViaPin("Then This 1");
	ExitViaPin("Then This 2");
	ExitViaPin("Then This 3");
	return ExitViaPin("Lastly This");
}