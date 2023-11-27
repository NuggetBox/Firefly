#include "VSNode_PhysicsTick.h"

void VSNode_PhysicsTick::Init()
{
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<float>("Delta Time", PinDirection::Output);
}