#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_IntegerAdd)
{
public:
	SetNodeTitle("Add (Integer)")
	SetDesc("Adds two integer values, A + B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerSub)
{
public:
	SetNodeTitle("Subtract (Integer)")
	SetDesc("Subtracts two integer values, A - B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerMul)
{
public:
	SetNodeTitle("Multiply (Integer)")
	SetDesc("Multiplies two integer values, A * B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerDiv)
{
public:
	SetNodeTitle("Divide (Integer)")
	SetDesc("Divides two integer values, A / B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};