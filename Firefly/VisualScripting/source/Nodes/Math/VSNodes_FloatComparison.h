#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_FloatNearlyEqual)
{
public:
	SetNodeTitle("Equal (Float)")
	SetDesc("Returns true if A and B are equal within the error tolerance |A - B| < tolerance")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatNotEqual)
{
public:
	SetNodeTitle("Not Equal (Float)")
	SetDesc("Returns true if A and B are not equal, A != B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatGreater)
{
public:
	SetNodeTitle("Greater (Float)")
	SetDesc("Returns true if A is greater than B, A > B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatGreaterEqual)
{
public:
	SetNodeTitle("Greater Equal (Float)")
	SetDesc("Returns true if A is greater than or equal to B, A >= B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatLess)
{
public:
	SetNodeTitle("Less (Float)")
	SetDesc("Returns true if A is less than B, A < B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatLessEqual)
{
public:
	SetNodeTitle("Less Equal (Float)")
	SetDesc("Returns true if A is less than or equal to B, A <= B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};