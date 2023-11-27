#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Print)
{
public:
	SetNodeTitle("Print");
	SetDesc("Prints a message to the console");

	void Init() override;
	size_t DoOperation() override;
};