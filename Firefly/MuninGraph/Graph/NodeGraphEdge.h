#pragma once
#include "ScriptGraph/ScriptGraphTypes.h"

struct NodeGraphEdge
{
	size_t EdgeId;
	size_t FromUID;
	size_t ToUID;

	ScriptGraphColor Color;
	float Thickness = 1.0f;
};
