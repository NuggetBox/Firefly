#include "MuninGraph.pch.h"
#include "ScriptGraphNode.h"

#include "ScriptGraphTypes.h"

size_t ScriptGraphNode::ExitViaPin(const std::string& aPinLabel)
{
	const size_t pinUID = NodeGraphNode<ScriptGraphPin, ScriptGraph, ScriptGraphSchema>::ExitViaPin(aPinLabel);
	const ScriptGraphPin& pin = GetPin(pinUID);
	assert(pin.GetType() == ScriptGraphPinType::Exec && "Only Exec pins can be used in ExitViaPin!");

	if(pin.IsPinConnected())
	{
		const size_t exitEdgeUID = pin.GetEdges()[0];
		const NodeGraphEdge& exitEdge = GetOwner()->GetEdgeFromUID(exitEdgeUID);
		const ScriptGraphPin& targetPin = GetOwner()->GetPinFromUID(exitEdge.ToUID);

		GetOwner()->ReportEdgeFlow(exitEdgeUID);

		return targetPin.GetOwner()->Exec(targetPin.GetUID());
	}

	return pinUID;
}

size_t ScriptGraphNode::ExitWithError(const std::string& anErrorMessage)
{
	hasErrored = true;
	myErrorMessage = anErrorMessage;
	const auto uuidPtr = AsUUIDAwarePtr(this);
	GetOwner()->ReportFlowError(uuidPtr->GetUID(), anErrorMessage);
	return 0;
}

size_t ScriptGraphNode::Exit()
{
	hasErrored = false;
	return ParentClass::Exit();
}

size_t ScriptGraphNode::Exec(size_t anEntryPinUID)
{
	hasErrored = false;
	return DoOperation();
}

void ScriptGraphNode::DeliverPayload(const ScriptGraphNodePayload& aPayload)
{
	for (const auto& [pinUid, pin] : GetPins())
	{
		if (const auto dataIt = aPayload.Data.find(pin.GetLabel()); dataIt != aPayload.Data.end())
		{
			SetRawPinData(pin.GetLabel(), dataIt->second.Ptr, dataIt->second.TypeData->GetTypeSize());
		}
	}
}
