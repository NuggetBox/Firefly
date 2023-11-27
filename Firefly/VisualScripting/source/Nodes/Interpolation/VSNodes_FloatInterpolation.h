#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_FloatLerp)
{
public:
	SetNodeTitle("Lerp (Float)");
	SetDesc("Linearly interpolates between A & B, by Alpha");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatEaseIn)
{
public:
	SetNodeTitle("Ease In (Float)");
	SetDesc("Interpolates, slowly at first, between A & B, by Alpha");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatEaseOut)
{
public:
	SetNodeTitle("Ease Out (Float)");
	SetDesc("Interpolates, fast at first, between A & B, by Alpha");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatEaseInOut)
{
public:
	SetNodeTitle("Ease In Out (Float)");
	SetDesc("Interpolates, slowly at first and slowly in the end, between A & B, by Alpha");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatBounce)
{
public:
	SetNodeTitle("Bounce (Float)");
	SetDesc("Interpolates in a bouncing shape, between A & B, by Alpha");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatParabola)
{
public:
	SetNodeTitle("Parabola (Float)");
	SetDesc("Interpolates in a parabola shape, between A & B, by Alpha, start at 0 reach 1 at 0.5 and end at 0");
	SetCategory("Float Interpolation");

	void Init() override;
	size_t DoOperation() override;
};