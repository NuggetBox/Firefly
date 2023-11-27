#pragma once
#include "ScriptGraph/ScriptGraphNode.h"
#include "SGNode_EventBase.h"

BeginScriptGraphDerivedNode(SGNode_EventTick, SGNode_EventBase)
{
public:

	void Init() override;

	std::string GetNodeTitle() const override { return "Tick"; }
	std::string GetDescription() const override { return "An event node that fires every frame."; };
};
