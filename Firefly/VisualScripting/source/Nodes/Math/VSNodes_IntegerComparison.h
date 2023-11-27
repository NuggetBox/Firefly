#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_IntegerEqual)
{
public:
	SetNodeTitle("Equal (Integer)")
	SetDesc("Returns true if A and B are equal, A == B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerNotEqual)
{
public:
	SetNodeTitle("Not Equal (Integer)")
	SetDesc("Returns true if A and B are not equal, A != B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerGreater)
{
public:
	SetNodeTitle("Greater (Integer)")
	SetDesc("Returns true if A is greater than B, A > B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerGreaterEqual)
{
public:
	SetNodeTitle("Greater Equal (Integer)")
	SetDesc("Returns true if A is greater than or equal to B, A >= B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerLess)
{
public:
	SetNodeTitle("Less (Integer)")
	SetDesc("Returns true if A is less than B, A < B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntegerLessEqual)
{
public:
	SetNodeTitle("Less Equal (Integer)")
	SetDesc("Returns true if A is less than or equal to B, A <= B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};