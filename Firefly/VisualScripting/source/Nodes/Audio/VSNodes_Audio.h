#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

BeginScriptGraphNode(VSNode_Audio_PlayEvent)
{
public:
	SetNodeTitle("PlayEvent");
	SetDesc("Play Event");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_CreateEventInstance)
{
public:
	SetNodeTitle("CreateEventInstance");
	SetDesc("Create an Event Instance");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_GetMinMaxDistance)
{
public:
	SetNodeTitle("GetMinMaxDistance");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_GetPlaybackState)
{
public:
	SetNodeTitle("GetPlaybackState");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_PlayEventOneShot)
{
public:
	SetNodeTitle("PlayEventOneShot");
	SetDesc("Play Event as OneShot");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_PlayEventOneShotWithParameters)
{
public:
	SetNodeTitle("PlayEventOneShotWithParameters");
	SetDesc("Play Event as OneShot With Parameters");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_SetEvent3DParameters)
{
public:
	SetNodeTitle("SetEvent3DParameters");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_SetGlobalParameter)
{
public:
	SetNodeTitle("SetGlobalParameter");
	SetDesc("Sets Global Parameter for sound events");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_SetListenerAttributes)
{
public:
	SetNodeTitle("SetListenerAttributes");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_Audio_StopEvent)
{
public:
	SetNodeTitle("StopSoundEvent");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};

BeginScriptGraphNode(VSNode_AudioManagerInit)
{
public:
	SetNodeTitle("AudioManagerInit");
	SetDesc("Insert a descriptive and relevant descripting here");
	SetCategory("Audio");

	void Init() override;
	size_t DoOperation() override;
};