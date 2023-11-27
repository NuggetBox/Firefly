#include "MuninGraph.pch.h"
#include "SGNode_EventBase.h"

ScriptGraphColor SGNode_EventBase::GetNodeHeaderColor() const
{
	return ScriptGraphColor(255, 0, 0, 128);
}

size_t SGNode_EventBase::DoOperation()
{
	return ExitViaPin("Out");
}