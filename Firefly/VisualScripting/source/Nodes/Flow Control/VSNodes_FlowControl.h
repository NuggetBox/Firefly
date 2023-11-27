#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Branch)
{
public:
	SetNodeTitle("Branch");
	SetCategory("Flow Control");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_DoOnce)
{
public:
	SetNodeTitle("Do Once");
	SetCategory("Flow Control");

	void Init() override;
	size_t DoOperation() override;
	size_t Exec(size_t anEntryPinUID) override;

private:
	bool myHasExecuted = false;
};

BeginScriptGraphNode(VSNode_ForLoop)
{
public:
	SetNodeTitle("For Loop");
	SetCategory("Flow Control");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_WhileLoop)
{
public:
	SetNodeTitle("While Loop");
	SetCategory("Flow Control");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Sequence)
{
public:
	SetNodeTitle("Sequence");
	SetCategory("Flow Control");

	void Init() override;
	size_t DoOperation() override;
};