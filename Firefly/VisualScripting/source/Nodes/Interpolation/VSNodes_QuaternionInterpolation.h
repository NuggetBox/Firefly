#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_QuaternionLerp)
{
public:
	SetNodeTitle("Lerp (Quaternion)");
	SetDesc("Linearly interpolates between A & B, by Alpha");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_QuaternionEaseIn)
{
public:
	SetNodeTitle("Ease In (Quaternion)");
	SetDesc("Interpolates between A & B, by Alpha, slowly at first");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_QuaternionEaseOut)
{
public:
	SetNodeTitle("Ease Out (Quaternion)");
	SetDesc("Interpolates, fast at first, between A & B, by Alpha");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_QuaternionEaseInOut)
{
public:
	SetNodeTitle("Ease In Out (Quaternion)");
	SetDesc("Interpolates, slowly at first and slowly in the end, between A & B, by Alpha");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_QuaternionBounce)
{
public:
	SetNodeTitle("Bounce (Quaternion)");
	SetDesc("Interpolates in a bouncing shape, between A & B, by Alpha");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_QuaternionParabola)
{
public:
	SetNodeTitle("Parabola (Quaternion)");
	SetDesc("Interpolates in a parabola shape, between A & B, by Alpha, start at 0 reach 1 at 0.5 and end at 0");
	SetCategory("Quaternion Interpolation");

	void Init() override;
	size_t DoOperation() override;
};