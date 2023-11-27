#pragma once
#include "ScriptGraph/ScriptGraphPin.h"

template<typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename GraphSchemaType>
class NodeGraph;

template<typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename GraphSchemaType>
class NodeGraphInternal : public std::enable_shared_from_this<NodeGraph<GraphNodeType, GraphPinType, GraphEdgeType, GraphSchemaType>>
{
	// Our Schema can do whatever it wants.
	friend GraphSchemaType;

	// And the NodeGraph base class can do whatever it wants.
	friend class NodeGraph<GraphNodeType, GraphPinType, GraphEdgeType, GraphSchemaType>;

	// Only friends can construct us.
	NodeGraphInternal() = default;

	// To make sure all edges in this Graph has unique IDs
	size_t myNextEdgeId = 1;

	// All the nodes that live in this graph.
	std::unordered_map<size_t, std::shared_ptr<GraphNodeType>> myNodes = {};

	// Acceleration map for reverse lookup of PinUID to Pin.
	std::unordered_map<size_t, GraphPinType*> myPins = {};

	// Edge lookup of EdgeUID to Edge.
	std::unordered_map<size_t, GraphEdgeType> myEdges = {};

protected: // These are functions that friends of NodeGraph can access.

	/**
	 * \brief Retrieves the Pin that should be used as the data source when reading from
	 *		  the provided PinUID by checking the EdgeMap.
	 * \param aPinUID The Pin UID to retrieve data for.
	 * \return The GraphPinType reference to the actual data source node.
	 */
	virtual const GraphPinType& GetDataSourcePin(size_t aPinUID, bool& outErrored) const;

public:
	/**
	 * \brief Finds the actual GraphPinType ref from the provided PinUID.
	 * \param aPinUID The UID of the pin to find.
	 * \return The GraphPinType ref of the requested pin.
	 */
	[[nodiscard]] const GraphPinType& GetPinFromUID(size_t aPinUID) const;
	//THIS WAS PREVIOSLY IN PROTECTED ^^
	[[nodiscard]] const GraphEdgeType& GetEdgeFromUID(size_t aEdgeUID) const;
	//THIS WAS PREVIOSLY IN PROTECTED ^^

	virtual ~NodeGraphInternal() = default;

	NodeGraphInternal(const NodeGraphInternal& other) = default;
	NodeGraphInternal& operator=(const NodeGraphInternal& other) = default;

	// TODO: Need a more intelligent way to manage lifetime of this object.
	// Should be requested whenever you want to CHANGE the graph and is unique
	// per graph. If you call this it'll create the object with this graph as
	// argument and return it.
	std::unique_ptr<GraphSchemaType> GetGraphSchema();

	[[nodiscard]] const std::unordered_map<size_t, std::shared_ptr<GraphNodeType>>& GetNodes() const { return myNodes; }
	[[nodiscard]] const std::unordered_map<size_t, GraphEdgeType>& GetEdges() const { return myEdges; }
};

/**
 * \brief Base class for Node Graphs.
 * \tparam GraphNodeType Base class for nodes that will exist in this graph.
 * \tparam GraphPinType The class that will be used for node Pins.
 * \tparam GraphEdgeType The struct that will be used for Edges.
 * \tparam GraphSchemaType The schema that governs how this graph functions.
 */
template<class GraphNodeType, class GraphPinType, class GraphEdgeType, class GraphSchemaType>
class NodeGraph : public NodeGraphInternal<GraphNodeType, GraphPinType, GraphEdgeType, GraphSchemaType>
{
	// These classes can access Protected items in NodeGraphInternal.
	friend GraphNodeType;
	friend GraphPinType;
	friend GraphEdgeType;

public:

	NodeGraph() = default;
	~NodeGraph() override = default;
};

template <typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename GraphSchemaType>
const GraphPinType& NodeGraphInternal<GraphNodeType, GraphPinType, GraphEdgeType, GraphSchemaType>::GetDataSourcePin(
	size_t aPinUID, bool& outErrored) const
{
	const GraphPinType& pin = GetPinFromUID(aPinUID);
	const PinDirection pinDir = pin.GetPinDirection();

	assert(pinDir == PinDirection::Input && "Pin Data can only be fetched from Input pins!");

	outErrored = false;

	if (pin.IsPinConnected())
	{
		// Inputs only have one connection.
		const NodeGraphEdge& myEdge = myEdges.find(pin.GetEdges()[0])->second;
		return GetPinFromUID(myEdge.FromUID);
	}

	return pin;
}

template <typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename SchemaType>
const GraphPinType& NodeGraphInternal<GraphNodeType, GraphPinType, GraphEdgeType, SchemaType>::GetPinFromUID(size_t aPinUID) const
{
	const auto it = myPins.find(aPinUID);
	assert(it != myPins.end() && "Invalid Pin ID!");
	return *it->second;
}

template <typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename GraphSchemaType>
const GraphEdgeType& NodeGraphInternal<GraphNodeType, GraphPinType, GraphEdgeType, GraphSchemaType>::GetEdgeFromUID(
	size_t aEdgeUID) const
{
	const auto it = myEdges.find(aEdgeUID);
	assert(it != myEdges.end() && "Invalid Edge ID!");
	return it->second;
}

template <typename GraphNodeType, typename GraphPinType, typename GraphEdgeType, typename SchemaType>
std::unique_ptr<SchemaType> NodeGraphInternal<GraphNodeType, GraphPinType, GraphEdgeType, SchemaType>::GetGraphSchema()
{
	return std::make_unique<SchemaType>(this->shared_from_this());
}
