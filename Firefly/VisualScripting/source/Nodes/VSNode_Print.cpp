#include "VSNode_Print.h"

#include "Firefly/Core/Log/DebugLogger.h"

void VSNode_Print::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<std::string>("Message", PinDirection::Input);
}

size_t VSNode_Print::DoOperation()
{
	std::string message;
	if (GetPinData("Message", message))
	{
		LOGINFO(message);
		return ExitViaPin("Out");
	}

	return 0;
}