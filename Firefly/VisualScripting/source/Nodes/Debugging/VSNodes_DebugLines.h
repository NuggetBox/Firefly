#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_DebugLine)
{
public:
	SetNodeTitle("Debug Line");
	SetDesc("Draws a debug line from start to end, does not show in final game");
	SetCategory("Debugging");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_DebugSphere)
{
public:
	SetNodeTitle("Debug Sphere");
	SetDesc("Draws a debug sphere with given radius, does not show in final game");
	SetCategory("Debugging");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_DebugCube)
{
public:
	SetNodeTitle("Debug Cube");
	SetDesc("Draws a dube with the sides being 2x size");
	SetCategory("Debugging");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_DebugArrow)
{
public:
	SetNodeTitle("Debug Arrow");
	SetDesc("Draws an arrow pointing from start to end");
	SetCategory("Debugging");

	void Init() override;
	size_t DoOperation() override;
};