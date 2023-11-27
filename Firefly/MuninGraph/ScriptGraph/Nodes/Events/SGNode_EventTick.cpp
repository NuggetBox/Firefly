#include "MuninGraph.pch.h"
#include "SGNode_EventTick.h"

void SGNode_EventTick::Init()
{
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<float>("Delta Time", PinDirection::Output);
}