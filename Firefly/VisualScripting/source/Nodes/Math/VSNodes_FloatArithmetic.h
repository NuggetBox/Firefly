#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_FloatAdd)
{
public:
	SetNodeTitle("Add (Float)")
	SetDesc( "Adds two float values, A + B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatSub)
{
public:
	SetNodeTitle("Subtract (Float)")
	SetDesc("Subtracts two float values, A - B")
	SetCategory( "Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatMul)
{
public:
	SetNodeTitle("Multiply (Float)")
	SetDesc("Multiplies two float values, A * B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatDiv)
{
public:
	SetNodeTitle("Divide (Float)")
	SetDesc("Divides two float values, A / B")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

//BeginScriptGraphNode(VSNode_FloatLerp)
//{
//public:
//	SetNodeTitle("Lerp (Float)")
//	SetDesc("Linearly interpolates between A & B, by Alpha")
//	SetCategory("Math")
//
//	void Init() override;
//	size_t DoOperation() override;
//};