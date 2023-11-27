#pragma once
#include "SGNode_EventBase.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphDerivedNode(SGNode_EventBeginPlay, SGNode_EventBase)
{
public:

	void Init() override;

	std::string GetNodeTitle() const override { return "Begin Play"; }
	std::string GetDescription() const override { return "An event node that fires when the object that owns it spawns."; }

	//[[nodiscard]] ScriptGraphColor GetNodeHeaderColor() const override;

	//size_t DoOperation() override;

	//FORCEINLINE bool IsEntryNode() const override { return true; }
	//FORCEINLINE bool IsInternalOnly() const override { return true; }
	//FORCEINLINE unsigned MaxInstancesPerGraph() const override { return 1; }
};
