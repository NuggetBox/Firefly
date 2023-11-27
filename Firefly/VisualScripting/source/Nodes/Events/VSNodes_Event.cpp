#include "VSNodes_Event.h"

#include "ScriptEventManager.h"

void VSNode_OnEvent::Init()
{
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<std::string>("Event Name", PinDirection::Input);
	CreateDataPin<uint64_t>("Entity", PinDirection::Output);
	CreateDataPin<int>("Index", PinDirection::Output);
}

size_t VSNode_OnEvent::DoOperation()
{
	return ExitViaPin("Out");
}

void VSNode_TriggerEvent::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<std::string>("Event Name", PinDirection::Input);
	CreateDataPin<uint64_t>("My Entity", PinDirection::Input);
	CreateDataPin<uint64_t>("Target Entity", PinDirection::Input);
	CreateDataPin<int>("Index", PinDirection::Input);
}

size_t VSNode_TriggerEvent::DoOperation()
{
	std::string eventName;
	uint64_t myEntityID = 0;
	uint64_t targetEntityID = 0;
	int index;

	if (GetPinData("Event Name", eventName) && GetPinData("My Entity", myEntityID) && GetPinData("Target Entity", targetEntityID) && GetPinData("Index", index))
	{
		ScriptEventManager::TriggerEvent(eventName, index, targetEntityID, myEntityID);
		return ExitViaPin("Out");
	}

	return 0;
}