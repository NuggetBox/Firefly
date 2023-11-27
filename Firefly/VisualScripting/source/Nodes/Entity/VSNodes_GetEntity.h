#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_GetEntityWithName)
{
public:
	SetNodeTitle("Get Entity With Name")
	SetDesc("Gets the entity with the given name")
	SetCategory("Entity")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IsEntityValid)
{
public:
	SetNodeTitle("Is Valid")
	SetDesc("Returns true if the Entity exists and is valid")
	SetCategory("Entity")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetEntityActive)
{
public:
	SetNodeTitle("Set Active")
		SetDesc("Sets the entity to be active or inactive")
		SetCategory("Entity")

		void Init() override;
	size_t DoOperation() override;
};