#pragma once
#include "MuninGraph.h"
#include "ScriptGraph/ScriptGraphNode.h"

// Example file for how to register nodes in another project, i.e. not inside MuninGraph.lib.
// To get all the nodes inside MuninGraph in your project to auto-register you need to link
// with /WHOLEARCHIVE:MuninGraph.lib or VS will optimize the auto reg away. Similarly if you
// have this file in a lib of your own you need to /WHOLEARCHIVE that file too. If this lives
// in an EXE project then it'll work as intended on its own.

// NOTE THE INCLUDES!

BeginScriptGraphNode(VSNode_ExampleNode)
{
public:
	SetNodeTitle("Example Node");
	SetDesc("Insert a descriptive and relevant description here");
	SetCategory("Default");

	void Init() override;
	size_t DoOperation() override;

	//Overridable functions & their default values

	//virtual bool IsEntryNode() const { return false; }
	//virtual bool IsInternalOnly() const { return false; }
	//virtual bool IsDebugOnly() const { return false; }
	//virtual bool IsSimpleNode() const { return false; }  // False: Regular node. True: The node has no header or name
};