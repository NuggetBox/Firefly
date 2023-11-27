#include "MuninGraph.pch.h"
#include "SGNode_EventBeginPlay.h"

void SGNode_EventBeginPlay::Init()
{
	CreateExecPin("Out", PinDirection::Output, true);
}

//ScriptGraphColor SGNode_EventBeginPlay::GetNodeHeaderColor() const
//{
//	return ScriptGraphColor(255, 0, 0, 128);
//}
//
//size_t SGNode_EventBeginPlay::DoOperation()
//{
//	return ExitViaPin("Out");
//}
