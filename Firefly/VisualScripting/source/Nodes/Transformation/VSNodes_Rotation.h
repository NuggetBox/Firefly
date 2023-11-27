#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_SetRotation)
{
public:
	SetNodeTitle("Set Entity World Rotation")
	SetDesc("Sets the Entity World Rotation")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetRotation)
{
public:
	SetNodeTitle("Get Entity World Rotation")
	SetDesc("Gets the Entity World Rotation")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetLocalRotation)
{
public:
	SetNodeTitle("Set Entity Local Rotation")
	SetDesc("Sets the Entity Rotation relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetLocalRotation)
{
public:
	SetNodeTitle("Get Entity Local Rotation")
	SetDesc("Gets the Entity Rotation relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_AddRotation)
{
public:
	SetNodeTitle("Add Entity World Rotation")
	SetDesc("Rotates Entity along the World Axes by Delta Rotation")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_AddLocalRotation)
{
public:
	SetNodeTitle("Add Entity Local Rotation")
	SetDesc("Rotates Entity along its own local axes by Delta Rotation")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};