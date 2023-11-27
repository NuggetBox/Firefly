#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"
#include "ScriptGraph/Nodes/Events/SGNode_EventBase.h"

BeginScriptGraphDerivedNode(VSNode_PhysicsTick, SGNode_EventBase)
{
public:
	SetNodeTitle("Physics Tick");
	SetDesc("Tick node where ALL physics code should be run in scripts");
	SetCategory("Physics");

	void Init() override;
	
	bool IsInternalOnly() const override { return false; }
	bool IsEntryNode() const override { return true; }
};