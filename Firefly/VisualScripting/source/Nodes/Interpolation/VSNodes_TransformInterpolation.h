#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_TransformLerp)
{
public:
	SetNodeTitle("Lerp (Transform)");
	SetDesc("Linearly interpolates between A & B, by Alpha");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_TransformEaseIn)
{
public:
	SetNodeTitle("Ease In (Transform)");
	SetDesc("Interpolates between A & B, by Alpha, slowly at first");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_TransformEaseOut)
{
public:
	SetNodeTitle("Ease Out (Transform)");
	SetDesc("Interpolates, fast at first, between A & B, by Alpha");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_TransformEaseInOut)
{
public:
	SetNodeTitle("Ease In Out (Transform)");
	SetDesc("Interpolates, slowly at first and slowly in the end, between A & B, by Alpha");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_TransformBounce)
{
public:
	SetNodeTitle("Bounce (Transform)");
	SetDesc("Interpolates in a bouncing shape, between A & B, by Alpha");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_TransformParabola)
{
public:
	SetNodeTitle("Parabola (Transform)");
	SetDesc("Interpolates in a parabola shape, between A & B, by Alpha, start at 0 reach 1 at 0.5 and end at 0");
	SetCategory("Transform Interpolation");

	void Init() override;
	size_t DoOperation() override;
};