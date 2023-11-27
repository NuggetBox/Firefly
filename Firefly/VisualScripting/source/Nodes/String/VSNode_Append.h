#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Append)
{
public:
	SetNodeTitle("Append");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("String");

	void Init() override;
	size_t DoOperation() override;
};