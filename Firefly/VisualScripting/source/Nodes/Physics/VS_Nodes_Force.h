#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_AddForce)
{
public:
	SetNodeTitle("Add Force");
	SetDesc("Add Force To The Entity");
	SetCategory("Physics");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetForce)
{
public:
	SetNodeTitle("Set Force");
	SetDesc("Set Force Of The Entity");
	SetCategory("Physics");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_ResetForce)
{
public:
	SetNodeTitle("Reset Force");
	SetDesc("Resets The Current Force On Entity");
	SetCategory("Physics");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetLinearVelocity)
{
public:
	SetNodeTitle("Get Entity Linear Velocity");
	SetDesc("Get Current Linear Velocity Of Entity");
	SetCategory("Physics");

	void Init() override;
	size_t DoOperation() override;
};