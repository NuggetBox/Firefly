#pragma once
#include <functional>
#include <memory>

struct ScriptGraphNodePayload;
struct ScriptGraphVariable;
class ScriptGraphPin;
class ScriptGraphNode;
struct NodeGraphEdge;
class ScriptGraphSchema;
class ScriptGraph;

class ScriptGraphInternal : public NodeGraph<ScriptGraphNode, ScriptGraphPin, NodeGraphEdge, ScriptGraphSchema>
{
	// All ScriptGraph private things go in here.
	typedef NodeGraph<ScriptGraphNode, ScriptGraphPin, NodeGraphEdge, ScriptGraphSchema> Super;

	// Our Schema can do whatever it wants.
	friend ScriptGraphSchema;
	friend ScriptGraph;	
	ScriptGraphInternal() = default;

	// A map of all nodes that can start execution flow along with a node handle.
	// This allows you to call a specific start node at a specific point via this handle.
	std::unordered_map<std::string, std::shared_ptr<ScriptGraphNode>> myEntryPoints;
	// Reverse lookup to find the entry handle from the Node UId;
	std::unordered_map<size_t, std::string> myNodeUIDToEntryHandle;

	std::unordered_map<std::string, std::shared_ptr<ScriptGraphVariable>> myVariables;

	std::vector<size_t> myLastExecutedPath;

	bool bShouldTick = false;

	// TODO: Should this be a thing still?
	//bool GraphExec(const std::shared_ptr<ScriptGraphNode>& anEntryPoint);

public:
	typedef std::function<void(const class ScriptGraph&, size_t, const std::string&)> ScriptGraphErrorHandlerSignature;

private:
	ScriptGraphErrorHandlerSignature myErrorDelegate;

protected:

	// Node interface goes here
	virtual const ScriptGraphPin& GetDataSourcePin(size_t aPinUID, bool& outErrored) const override;

	void ReportEdgeFlow(size_t anEdgeUID);

	void ReportFlowError(size_t aNodeUID, const std::string& anErrorMessage) const;

public:

	const std::vector<size_t>& GetLastExecutedPath() const { return myLastExecutedPath; }
	void ResetLastExecutedPath() { myLastExecutedPath.clear(); }

	void Tick(float aDeltaTime, bool aPhysicsTick = false);
	void SetTicking(bool bTicking);

	void BindErrorHandler(ScriptGraphErrorHandlerSignature&& aErrorHandler);
	void UnbindErrorHandler();

	bool Run(const std::string& anEntryPointHandle);
	bool RunWithPayload(const std::string& anEntryPointHandle, const ScriptGraphNodePayload& aPayload);
};

class ScriptGraph : public ScriptGraphInternal
{
	// These classes can access Protected items in ScriptGraphInternal.
	friend class ScriptGraphNode;
	friend class NodeGraphNode<ScriptGraphPin, ScriptGraph, ScriptGraphSchema>;
};
