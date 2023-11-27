#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Vector3Lerp)
{
public:
	SetNodeTitle("Lerp (Vector3)");
	SetDesc("Linearly interpolates between A & B, by Alpha");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Vector3EaseIn)
{
public:
	SetNodeTitle("Ease In (Vector3)");
	SetDesc("Interpolates between A & B, by Alpha, slowly at first");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Vector3EaseOut)
{
public:
	SetNodeTitle("Ease Out (Vector3)");
	SetDesc("Interpolates, fast at first, between A & B, by Alpha");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Vector3EaseInOut)
{
public:
	SetNodeTitle("Ease In Out (Vector3)");
	SetDesc("Interpolates, slowly at first and slowly in the end, between A & B, by Alpha");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Vector3Bounce)
{
public:
	SetNodeTitle("Bounce (Vector3)");
	SetDesc("Interpolates in a bouncing shape, between A & B, by Alpha");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Vector3Parabola)
{
public:
	SetNodeTitle("Parabola (Vector3)");
	SetDesc("Interpolates in a parabola shape, between A & B, by Alpha, start at 0 reach 1 at 0.5 and end at 0");
	SetCategory("Vector3 Interpolation");

	void Init() override;
	size_t DoOperation() override;
};