#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"
#include "ScriptGraph/Nodes/Events/SGNode_EventBase.h"

BeginScriptGraphDerivedNode(VSNode_OnEvent, SGNode_EventBase)
{
public:
	SetNodeTitle("On Event");
	SetDesc("On Event executes whenever the event with Event Name is triggered");
	SetCategory("Event");

	void Init() override;
	size_t DoOperation() override;
	
	bool IsInternalOnly() const override { return false; }
	bool IsEntryNode() const override { return true; }
	unsigned MaxInstancesPerGraph() const override { return 0; }
};

BeginScriptGraphNode(VSNode_TriggerEvent)
{
public:
	SetNodeTitle("Trigger Event");
	SetDesc("Trigger Event executes all OnEvent nodes with the given name");
	SetCategory("Event");

	void Init() override;
	size_t DoOperation() override;
};