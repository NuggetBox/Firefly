#include "MuninGraph.pch.h"
#include "ScriptGraph.h"

#include <fstream>
#include <json.hpp>

#include "ScriptGraphNode.h"
#include "Nodes/SGNode_Variable.h"

//bool ScriptGraphInternal::GraphExec(const std::shared_ptr<ScriptGraphNode>& anEntryPoint)
//{
//	myLastExecutedPath.clear();
//
//	std::shared_ptr<ScriptGraphNode> currentNode = anEntryPoint;
//	size_t currentEntryPinUID = 0;
//	bool bIsRunning = true;
//	while(bIsRunning)
//	{
//		// TODO: This doesn't work if there are timer nodes or other flow control.
//		// I.e. it'll not respond to future ExitViaPin calls since we're outside
//		// this loop.
//
//		const size_t exitPinUID = currentNode->Exec(currentEntryPinUID);
//		if(currentNode->HasError())
//		{
//			//myErrorDelegate(*this, currentNode, currentNode->GetErrorMessage());
//			return false;
//		}
//
//		const ScriptGraphPin& exitPin = GetPinFromUID(exitPinUID);
//		if (exitPin.IsPinConnected())
//		{
//			const size_t edgeUID = exitPin.GetEdges()[0];
//
//			myLastExecutedPath.push_back(edgeUID);
//
//			const NodeGraphEdge& edge = GetEdgeFromUID(edgeUID);
//			const ScriptGraphPin& targetPin = GetPinFromUID(edge.ToUID);
//			currentNode = targetPin.GetOwner();
//			currentEntryPinUID = targetPin.GetUID();
//		}
//		else
//		{
//			bIsRunning = false;
//		}
//	}
//
//	return true;
//}

const ScriptGraphPin& ScriptGraphInternal::GetDataSourcePin(size_t aPinUID, bool& outErrored) const
{
	const ScriptGraphPin& dataPin = Super::GetDataSourcePin(aPinUID, outErrored);
	if(dataPin.GetUID() != aPinUID && !dataPin.GetOwner()->IsExecNode())
	{
		// If this node isn't an exec node we need to ask it to
		// evaluate itself so we can get the data we want.
		// Just like UE we force the node to recalculate and we
		// do not cache any result.

		// ScriptGraph design says that this node should not return
		// a pin to continue on since it's not an exec node.
		// Therefore we don't need to take care of that part.
		// This should recursively call GetDataSourcePin as needed.
		dataPin.GetOwner()->DoOperation();
		outErrored = dataPin.GetOwner()->HasError();
	}

	return dataPin;
}

void ScriptGraphInternal::ReportEdgeFlow(size_t anEdgeUID)
{
	myLastExecutedPath.push_back(anEdgeUID);
}

void ScriptGraphInternal::ReportFlowError(size_t aNodeUID, const std::string& anErrorMessage) const
{
	if(myErrorDelegate)
	{
		myErrorDelegate(*dynamic_cast<const ScriptGraph*>(this), aNodeUID, anErrorMessage);
	}
}

bool ScriptGraphInternal::Run(const std::string& anEntryPointHandle)
{
	if(const auto it = myEntryPoints.find(anEntryPointHandle); it != myEntryPoints.end())
	{
		myLastExecutedPath.clear();
		return it->second->Exec(0) != 0;
	}

	return false;
}

bool ScriptGraphInternal::RunWithPayload(const std::string& anEntryPointHandle, const ScriptGraphNodePayload& aPayload)
{
	if (const auto it = myEntryPoints.find(anEntryPointHandle); it != myEntryPoints.end())
	{
		it->second->DeliverPayload(aPayload);
		return Run(anEntryPointHandle);
	}

	return false;
}

void ScriptGraphInternal::Tick(float aDeltaTime, bool aPhysicsTick)
{
	if(bShouldTick)
	{
		const std::string tick = aPhysicsTick ? "Physics Tick" : "Tick";

		if (const auto it = myEntryPoints.find(tick); it != myEntryPoints.end())
		{
			static ScriptGraphNodePayload tickPayload;
			tickPayload.SetVariable("Delta Time", aDeltaTime);
			RunWithPayload(tick, tickPayload);
		}
	}
}

void ScriptGraphInternal::SetTicking(bool bTicking)
{
	bShouldTick = bTicking;
}

void ScriptGraphInternal::BindErrorHandler(ScriptGraphErrorHandlerSignature&& aErrorHandler)
{
	myErrorDelegate = aErrorHandler;
}

void ScriptGraphInternal::UnbindErrorHandler()
{
	myErrorDelegate = nullptr;
}
