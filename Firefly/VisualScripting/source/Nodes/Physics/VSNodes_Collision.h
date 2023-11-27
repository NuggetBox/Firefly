#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"
#include "ScriptGraph/Nodes/Events/SGNode_EventBase.h"

BeginScriptGraphDerivedNode(VSNode_OnBeginOverlap, SGNode_EventBase)
{
public:
	SetNodeTitle("On Begin Overlap");
	SetDesc("On Begin Overlap is sent to 2 entities when their colliders start colliding/touching, (Collider - Collider) or (Collider - Trigger)");
	SetCategory("Physics");

	void Init() override;
	//size_t DoOperation() override;
	
	bool IsInternalOnly() const override { return false; }
	bool IsEntryNode() const override { return true; }
};

BeginScriptGraphDerivedNode(VSNode_OnEndOverlap, SGNode_EventBase)
{
public:
	SetNodeTitle("On End Overlap");
	SetDesc("On End Overlap is sent to 2 entities when their colliders stop colliding/touching, (Collider - Collider) or (Collider - Trigger)");
	SetCategory("Physics");

	void Init() override;
	//size_t DoOperation() override;

	bool IsInternalOnly() const override { return false; }
	bool IsEntryNode() const override { return true; }
};