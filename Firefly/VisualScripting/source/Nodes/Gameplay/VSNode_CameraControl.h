#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_CameraShake)
{
public:
	SetNodeTitle("Camera Shake")
		SetDesc("Shakes The Camera")
		SetCategory("Gameplay")

		void Init() override;
	size_t DoOperation() override;
};