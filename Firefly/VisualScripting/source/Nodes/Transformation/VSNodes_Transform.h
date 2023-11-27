#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_GetEntityTransform)
{
public:
	SetNodeTitle("Get Entity Transform")
	SetDesc("Gets the Entity World Transform")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetEntityTransform)
{
public:
	SetNodeTitle("Set Entity Transform")
	SetDesc("Sets the Entity World Transform")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_GetEntityLocalTransform)
{
public:
	SetNodeTitle("Get Entity Local Transform")
	SetDesc("Gets the Entity Transform relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_SetEntityLocalTransform)
{
public:
	SetNodeTitle("Set Entity Local Transform")
	SetDesc("Sets the Entity Transform relative to its parent")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_MakeTransform)
{
public:
	SetNodeTitle("Make Transform")
	SetDesc("Constructs a Transform from Position, Rotation, Scale")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_BreakTransform)
{
public:
	SetNodeTitle("Break Transform")
	SetDesc("Returns Position, Rotation, Scale from a Transform")
	SetCategory("Transformation")

	void Init() override;
	size_t DoOperation() override;
};

//BeginScriptGraphNode(VSNode_LerpTransform)
//{
//public:
//	SetNodeTitle("Lerp Transform")
//	SetDesc("Linearly interpolats between two transforms A & B, lerping the position, rotation and scale by Alpha")
//	SetCategory("Transformation")
//
//	void Init() override;
//	size_t DoOperation() override;
//};