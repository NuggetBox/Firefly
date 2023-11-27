#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Sin)
{
public:
	SetNodeTitle("Sin")
	SetDesc("Returns the sine of Angle, degrees")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Cos)
{
public:
	SetNodeTitle("Cos")
	SetDesc("Returns the cosine of Angle, degrees")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Tan)
{
public:
	SetNodeTitle("Tan")
	SetDesc("Returns the tan of Angle, degrees")

	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_ASin)
{
public:
	SetNodeTitle("ASin")
	SetDesc("Returns the arcsin of Angle, degrees")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_ACos)
{
public:
	SetNodeTitle("ACos")
	SetDesc("Returns the arccos of Angle, degrees")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Atan)
{
public:
	SetNodeTitle("Atan")
	SetDesc("Returns the arctan of Angle, degrees")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};