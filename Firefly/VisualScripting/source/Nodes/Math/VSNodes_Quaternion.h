#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_MakeQuaternion)
{
public:
	SetNodeTitle("Make Quaternion")
	SetDesc("Constructs a Quaternion structure from Euler angles, can be used to rotate objects")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_BreakQuaternion)
{
public:
	SetNodeTitle("Break Quaternion")
	SetDesc("Returns the Euler angles that the quaternion represents")
	SetCategory("Math")

	void Init() override;
	size_t DoOperation() override;
};