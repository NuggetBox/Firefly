#pragma once
#include <algorithm>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

#include "NodeGraphPin.h"
#include "NodeGraph.h"

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

// Represents a connection between two nodes. Connections are always Forward
// which means that the StartPinUID is on the SOURCE node and the EndPinUID
// is on the TARGET node.

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
class NodeGraphNode
{
	friend GraphSchemaType;

	// Acceleration map to find pins based on UID;
	std::unordered_map<size_t, GraphPinType> myPins;
	std::unordered_map<std::string, size_t> myPinLabelToUID;

	std::shared_ptr<GraphType> myOwner = nullptr;

protected:

	void AddPin(GraphPinType&& aPin)
	{
		myPinLabelToUID.insert({ aPin.GetLabel(), aPin.GetUID() });
		myPins.insert({ aPin.GetUID(), std::move(aPin) });		
	}

	bool GetRawPinData(const std::string& aPinLabel, void* outData, size_t outDataSize) const;

	bool SetRawPinData(const std::string& aPinLabel, const void* inData, size_t aDataSize);

	template<typename T>
	void SetPinData(const std::string& aPinLabel, const T& aData)
	{
		const auto It = myPinLabelToUID.find(aPinLabel);
		assert(It != myPinLabelToUID.end() && "That pin could not be found!");
		GraphPinType& pin = myPins.find(It->second)->second;
		pin.SetData(aData);
	}

	virtual size_t ExitViaPin(const std::string& aPinLabel);
	virtual size_t Exit() { return 0; }

	bool RenamePin(const std::string& aPinLabel, const std::string& aNewName);
	bool RenamePin(size_t aPinUID, const std::string& aNewName);

	bool IsPinUIDLabel(size_t aPinUId, const std::string& aPinLabel) const;

public:

	const std::shared_ptr<GraphType> GetOwner() const { return myOwner;  }

	virtual ~NodeGraphNode();

	template<typename T>
	bool GetPinData(const std::string& aPinLabel, T& outData) const
	{
		const GraphPinType& myPin = GetPin(aPinLabel);
		if (myPin.IsPinConnected())
		{
			bool sourceErrored = false;
			const GraphPinType& dataPin = myOwner->GetDataSourcePin(myPin.GetUID(), sourceErrored);
			if (!sourceErrored)
			{
				return dataPin.GetData(outData);
			}

			return false;
		}
		return myPin.GetData(outData);
	}

	virtual FORCEINLINE std::string GetNodeTitle() const { return "Graph Node"; }

	const FORCEINLINE std::unordered_map<size_t, GraphPinType>& GetPins() const { return myPins; }
	const FORCEINLINE GraphPinType& GetPin(size_t aPinUID) const;
	const FORCEINLINE GraphPinType& GetPin(const std::string& aPinLabel) const;

	/**
	 * Called when this node is asked to perform whatever operation it's supposed to do.
	 * @returns The Pin Index of the pin we want to exit on after performing our operation or 0 if we don't have a pin to exit on.
	 */
	virtual size_t DoOperation() { return Exit(); }
};

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
bool NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::GetRawPinData(const std::string& aPinLabel,
	void* outData, size_t outDataSize) const
{
	const GraphPinType& myPin = GetPin(aPinLabel);
	if (myPin.IsPinConnected())
	{
		bool sourceErrored = false;
		const GraphPinType& dataPin = myOwner->GetDataSourcePin(myPin.GetUID(), sourceErrored);
		return dataPin.GetRawData(outData, outDataSize);
	}
	return myPin.GetRawData(outData, outDataSize);
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
bool NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::SetRawPinData(const std::string& aPinLabel,
	const void* inData, size_t aDataSize)
{
	const auto It = myPinLabelToUID.find(aPinLabel);
	assert(It != myPinLabelToUID.end() && "That pin could not be found!");
	GraphPinType& pin = myPins.find(It->second)->second;
	pin.SetRawData(inData, aDataSize);
	return true;
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
size_t NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::ExitViaPin(const std::string& aPinLabel)
{
	auto It = myPinLabelToUID.find(aPinLabel);
	assert(It != myPinLabelToUID.end() && "That pin could not be found!");
	return It->second;
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
bool NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::RenamePin(const std::string& aPinLabel,
	const std::string& aNewName)
{
	if(const auto pinNameIt = myPinLabelToUID.find(aPinLabel); pinNameIt != myPinLabelToUID.end())
	{
		if(auto pinIt = myPins.find(pinNameIt->second); pinIt != myPins.end())
		{
			myPinLabelToUID.erase(pinIt->second.GetLabel());
			pinIt->second.SetLabel(aNewName);
			myPinLabelToUID.insert({ aNewName, pinIt->second.GetUID() });
			return true;
		}
	}

	return false;
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
bool NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::RenamePin(size_t aPinUID, const std::string& aNewName)
{
	if (auto pinIt = myPins.find(aPinUID); pinIt != myPins.end())
	{
		myPinLabelToUID.erase(pinIt->second.GetLabel());
		pinIt->second.SetLabel(aNewName);
		myPinLabelToUID.insert({ aNewName, pinIt->second.GetUID() });
		return true;
	}

	return false;
}

template <typename GraphPinType, typename GraphType, typename GraphSchemaType>
bool NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::IsPinUIDLabel(size_t aPinUId,
	const std::string& aPinLabel) const
{
	if(const auto pinId = myPinLabelToUID.find(aPinLabel); pinId != myPinLabelToUID.end())
	{
		return pinId->second == aPinUId;
	}

	return false;
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
inline NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::~NodeGraphNode()
{
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
const GraphPinType& NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::GetPin(size_t aPinUID) const
{
	auto It = myPins.find(aPinUID);
	assert(It != myPins.end() && "That pin could not be found!");
	return It->second;
}

template<typename GraphPinType, typename GraphType, typename GraphSchemaType>
const GraphPinType& NodeGraphNode<GraphPinType, GraphType, GraphSchemaType>::GetPin(const std::string& aPinLabel) const
{
	const auto It = myPinLabelToUID.find(aPinLabel);
	assert(It != myPinLabelToUID.end() && "That pin could not be found!");
	return myPins.find(It->second)->second;
}

#define DeclareGraphNodeClass(Name) class GraphNode_##Name : public GraphNode, public UniversalUID<GraphNode_##Name>
