#pragma once

/**
 * Holds the rules for the underlying graph when it's being handled in the editor.
 * I.e. this is the class to enforce if two pins can link and other rules for how
 * the graph should be built. Only used by the editor, not needed in during runtime.
 */
class NodeGraphSchema
{

public:

	NodeGraphSchema() = default;

	virtual ~NodeGraphSchema() = default;

	/**
	 * \brief Checks if the two provided pins can be joined with an edge.
	 * \param aSourcePinUID The From pin UID.
	 * \param aTargetPinUID The Target pin UID.
	 * \param outMesssage Feedback providing helpful information regarding the return result.
	 * \return True if an edge can be created.
	 */
	virtual bool CanCreateEdge(size_t aSourcePinUID, size_t aTargetPinUID, std::string& outMesssage) const = 0;

	/**
	 * \brief Creates an edge between the two provided pins. This may disconnect existing edges
	 *		  depending on the rules in the schema.
	 * \param aSourcePinUID The From pin UID.
	 * \param aTargetPinUID The To pin UID.
	 * \return True if the edge was successfully created.
	 */
	virtual bool CreateEdge(size_t aSourcePinUID, size_t aTargetPinUID) = 0;

	/**
	 * \brief Removes an existing edge.
	 * \param aEdgeUID The UID of the edge to remove.
	 * \return True if the edge was successfully removed.
	 */
	virtual bool RemoveEdge(size_t aEdgeUID) = 0;

	/**
	 * \brief Removes all edges from the specified pin.
	 * \param aPinUID The UID of the pin to disconnect all links from.
	 * \return True if the pin was successfully disconnected.
	 */
	virtual bool DisconnectPin(size_t aPinUID) = 0;
};
