#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_SetPosition)
{
public:
	SetNodeTitle("Set Entity Position")
	SetDesc("Sets the Entity World Position")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetPosition)
{
public:
	SetNodeTitle("Get Entity Position")
	SetDesc("Gets the Entity World Position")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetLocalPosition)
{
public:
	SetNodeTitle("Set Entity Local Position")
	SetDesc("Sets the Entity Position relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetLocalPosition)
{
public:
	SetNodeTitle("Get Entity Local Position")
	SetDesc("Gets the Entity Position relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_AddPosition)
{
public:
	SetNodeTitle("Add Entity World Position")
	SetDesc("Adds Position Delta to the Entity World Position, moving the Entity along the world axes")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_AddLocalPosition)
{
public:
	SetNodeTitle("Add Entity Local Position")
	SetDesc("Adds Position Delta to the Entity Local Position, moving the Entity along its own local axes")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};