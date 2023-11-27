#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_FloatToString)
{
public:
	SetNodeTitle("Float to String")
	SetDesc("Converts a Float to a String")
	SetCategory("Casts")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntToFloat)
{
public:
	SetNodeTitle("Int to Float")
	SetDesc("Converts an Int to a Float")
	SetCategory("Casts")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_FloatToInt)
{
public:
	SetNodeTitle("Float to Int")
	SetDesc("Converts a Float to an Int")
	SetCategory("Casts")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_IntToString)
{
public:
	SetNodeTitle("Int to String")
	SetDesc("Converts an Int to a String")
	SetCategory("Casts")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_BoolToString)
{
public:
	SetNodeTitle("Bool to String")
		SetDesc("Converts an Bool to a String")
		SetCategory("Casts")

		void Init() override;
	size_t DoOperation() override;
};