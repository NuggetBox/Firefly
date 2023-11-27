#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_GetEntityForward)
{
public:
	SetNodeTitle("Get Entity Forward");
	SetDesc("Gets the Entity Forward");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityBackward)
{
public:
	SetNodeTitle("Get Entity Backward");
	SetDesc("Gets the Entity Backward");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityRight)
{
public:
	SetNodeTitle("Get Entity Right");
	SetDesc("Gets the Entity Right");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityLeft)
{
public:
	SetNodeTitle("Get Entity Left");
	SetDesc("Gets the Entity Left");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityUp)
{
public:
	SetNodeTitle("Get Entity Up");
	SetDesc("Gets the Entity Up");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityDown)
{
public:
	SetNodeTitle("Get Entity Down");
	SetDesc("Gets the Entity Down");
	SetCategory("Transformation");

	void Init() override;
	size_t DoOperation() override;
};