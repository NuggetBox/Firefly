#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_MakeVector)
{
public:
	SetNodeTitle("Make Vector");
	SetDesc("Contructs a Vector3 from X, Y, Z");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_BreakVector)
{
public:
	SetNodeTitle("Break Vector");
	SetDesc("Returns X, Y, Z from a Vector3");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorLength)
{
public:
	SetNodeTitle("Vector Length");
	SetDesc("Returns the length of the vector");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorDistance)
{
public:
	SetNodeTitle("Vector Distance");
	SetDesc("Returns the distance between the vectors");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorLerp)
{
public:
	SetNodeTitle("Lerp (Vector)");
	SetDesc("Linearly interpolates between two vectors A & B, by alpha");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorNormalize)
{
public:
	SetNodeTitle("Normalize Vector");
	SetDesc("Normalizes a vector, makes it length one but still pointing in the same direction");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorMultiply)
{
public:
	SetNodeTitle("Multiply Vector by Float");
	SetDesc("Multiplies a vector by a float, making the vector longer by a factor of the float value");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorAdd)
{
public:
	SetNodeTitle("Add (Vector)");
	SetDesc("Adds two vectors together");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorSub)
{
public:
	SetNodeTitle("Subtract (Vector)");
	SetDesc("Subtracts two vectors");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_VectorDot)
{
public:
	SetNodeTitle("Dot Product");
	SetDesc("Calculates the dot product between two vectors");
	SetCategory("Math");

	void Init() override;
	size_t DoOperation() override;
};