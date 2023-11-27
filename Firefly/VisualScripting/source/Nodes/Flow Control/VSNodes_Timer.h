#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_StartTimer)
{
public:
	SetNodeTitle("Timer");
	SetCategory("Timer");

	void Init() override;
	size_t DoOperation() override;

	void OnTimer();
};

BeginScriptGraphNode(VSNode_GetTimerInfo)
{
public:
	SetNodeTitle("Get Timer Info");
	SetCategory("Timer");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_ResumeTimer)
{
public:
	SetNodeTitle("Resume Timer");
	SetCategory("Timer");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_PauseTimer)
{
public:
	SetNodeTitle("Pause Timer");
	SetCategory("Timer");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_RemoveTimer)
{
public:
	SetNodeTitle("Remove Timer");
	SetCategory("Timer");

	void Init() override;
	size_t DoOperation() override;
};