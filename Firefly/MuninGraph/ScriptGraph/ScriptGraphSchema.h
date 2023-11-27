#pragma once
#include <functional>

#include "../Graph/NodeGraphSchema.h"
#include "ScriptGraph.h"
#include "ScriptGraphTypes.h"
#include "ScriptGraphVariable.h"

class SGNode_SetVariable;
class SGNode_GetVariable;
class ScriptGraph;
class ScriptGraphNode;
class ScriptGraphPin;

struct ScriptGraphNodeClass
{
	std::type_index Type = typeid(std::nullptr_t);
	std::string TypeName;
	std::function<std::shared_ptr<ScriptGraphNode>()> New;
	std::shared_ptr<ScriptGraphNode> DefaultObject;

	bool InternalOnly = false;
	std::string NodeTitle;

	std::type_index BaseType = typeid(std::nullptr_t);

private:
	static void SetupNodeType( ScriptGraphNodeClass& aNodeType, std::shared_ptr<ScriptGraphNode>&& aTempNode);

public:

	template<typename T>
	static ScriptGraphNodeClass Create()
	{
		ScriptGraphNodeClass result;
		result.Type = typeid(T);
		result.New = []() { auto ptr = std::make_shared<T>(); ptr->Init(); return ptr; };
		SetupNodeType(result, std::make_shared<T>());
		return result;
	}

	template<typename T, typename B>
	static ScriptGraphNodeClass Create()
	{
		ScriptGraphNodeClass result = Create<T>();
		result.BaseType = typeid(B);
		return result;
	}

	[[nodiscard]] bool IsA(const ScriptGraphNodeClass& aType) const
	{
		return Type == aType.Type || (BaseType != typeid(std::nullptr_t) && BaseType == aType.BaseType);
	}

	[[nodiscard]] bool IsA(const std::type_info& aType) const
	{
		return Type == aType || (BaseType != typeid(std::nullptr_t) && BaseType == aType);
	}

	bool operator==(const ScriptGraphNodeClass& aType) const
	{
		return Type == aType.Type;
	}

	bool operator!=(const ScriptGraphNodeClass& aType) const
	{
		return Type != aType.Type;
	}
};

template<>
struct std::hash<ScriptGraphNodeClass>
{
	auto operator()(const ScriptGraphNodeClass& aType) const noexcept -> size_t
	{
		return std::hash<std::type_index>{}(aType.Type);
	}
};

class ScriptGraphSchema : public NodeGraphSchema
{
	std::shared_ptr<ScriptGraph> myGraph;
	std::unordered_map<std::string, unsigned> myNodeTypeCounts;
	std::vector<std::string> myGraphEntryPoints;

	// Prevents static init order fiasco from happening.
	// NOTE: This could potentially happen with registered types as well.
	static std::unordered_map<std::string, ScriptGraphNodeClass>& MyNodeTypesMap()
	{
		static std::unordered_map<std::string, ScriptGraphNodeClass> myMap;
		return myMap;
	}

	static std::unordered_map<std::type_index, std::string >& MyNodeTypeIdToName()
	{
		static std::unordered_map<std::type_index, std::string> myMap;
		return myMap;
	}

	static std::unordered_map<std::string, std::string>& MyNodeNameToTypeNameMap()
	{
		static std::unordered_map<std::string, std::string> myMap;
		return myMap;
	}

	static std::unordered_map<std::string, const ScriptGraphNodeClass> RegisterNodeTypes();
	static std::unordered_map<std::string, std::string> GetNodeNames();
	static std::shared_ptr<ScriptGraph> CreateScriptGraphInternal(bool createEmpty);

	static void CreateNodeCDOs();

public:

	template<typename N>
	static bool RegisterNodeType()
	{
		ScriptGraphNodeClass type = ScriptGraphNodeClass::Create<N>();
		if(!type.InternalOnly)
			MyNodeNameToTypeNameMap().insert({ type.NodeTitle, type.TypeName });
		MyNodeTypeIdToName().insert({ typeid(N), type.TypeName });
		auto It = MyNodeTypesMap().insert({ type.TypeName, std::move(type) });		
		return true;
	}

	template<typename N, typename B>
	static bool RegisterNodeTypeWithBase()
	{
		ScriptGraphNodeClass type = ScriptGraphNodeClass::Create<N, B>();
		if (!type.InternalOnly)
			MyNodeNameToTypeNameMap().insert({ type.NodeTitle, type.TypeName });
		MyNodeTypeIdToName().insert({ typeid(N), type.TypeName });
		auto It = MyNodeTypesMap().insert({ type.TypeName, std::move(type) });
		return true;
	}

	static std::shared_ptr<ScriptGraph> CreateScriptGraph();
	static bool SerializeScriptGraph(const std::shared_ptr<ScriptGraph>& aGraph, std::string& outResult);
	static bool DeserializeScriptGraph(std::shared_ptr<ScriptGraph>& outGraph, const std::string& inData);

	#pragma warning(push)
	#pragma warning(disable: 4172)
	template<typename C>
	static const ScriptGraphNodeClass& GetNodeTypeByClass()
	{
		const std::type_index classType = typeid(C);
		for(const auto& [nodeTypeName, nodeType] : MyNodeTypesMap())
		{
			if(nodeType.Type == classType)
			{
				return nodeType;
			}
		}

		assert(false && "Type not found!");
		return ScriptGraphNodeClass();
	}
	#pragma warning(pop)

private:

	ScriptGraphPin& GetMutablePin(size_t aPinUID);
	bool RegisterNode(std::shared_ptr<ScriptGraphNode> aNode);
	void RegisterEntryPointNode(std::shared_ptr<ScriptGraphNode> aNode, const std::string& aEntryHandle);
	void CreateEdgeInternal(ScriptGraphPin& aSourcePin, ScriptGraphPin& aTargetPin) const;

	void RegenerateEntryPointList();

	void CheckCyclicLink(const std::shared_ptr<ScriptGraphNode>& aNode, const std::shared_ptr<ScriptGraphNode>& aBaseNode, bool& outResult) const;

public:

	ScriptGraphSchema(const std::shared_ptr<void>& aGraph);

	template<typename T>
	std::shared_ptr<T> AddNode()
	{
		if(const auto it = MyNodeTypeIdToName().find(typeid(T)); it != MyNodeTypeIdToName().end())
		{
			return std::dynamic_pointer_cast<T>(AddNode(it->second));
		}
		
		return nullptr;
	}

	std::shared_ptr<ScriptGraphNode> AddNode(const std::string& aType);
	std::shared_ptr<ScriptGraphNode> AddNode(const ScriptGraphNodeClass& aType);


	template<typename T>
	std::shared_ptr<T> AddEntryNode(const std::string& aEntryHandle)
	{
		if (const auto it = MyNodeTypeIdToName().find(typeid(T)); it != MyNodeTypeIdToName().end())
		{
			return std::dynamic_pointer_cast<T>(AddEntryNode(it->second, aEntryHandle));
		}

		return nullptr;
	}

	std::shared_ptr<ScriptGraphNode> AddEntryNode(const std::string& aType, const std::string& aEntryHandle);
	std::shared_ptr<ScriptGraphNode> AddEntryNode(const ScriptGraphNodeClass& aType, const std::string& aEntryHandle);

	template<typename T>
	void AddVariable(const std::string& aVariableName, const T& aDefaultValue = T())
	{
		std::shared_ptr<ScriptGraphVariable> newVariable = std::make_shared<ScriptGraphVariable>();
		newVariable->Data = ScriptGraphDataObject::Create<T>();
		newVariable->DefaultData = ScriptGraphDataObject::Create<T>();
		memcpy_s(newVariable->DefaultData.Ptr, newVariable->DefaultData.TypeData->GetTypeSize(), &aDefaultValue, sizeof(T));
		memcpy_s(newVariable->Data.Ptr, newVariable->DefaultData.TypeData->GetTypeSize(), &aDefaultValue, sizeof(T));
		newVariable->Name = aVariableName;
		myGraph->myVariables.insert({ aVariableName, newVariable });
	}

	void AddVariable(const std::string& aVariableName, const ScriptGraphDataObject& aDataObject)
	{
		std::shared_ptr<ScriptGraphVariable> newVariable = std::make_shared<ScriptGraphVariable>();
		newVariable->Data = aDataObject;
		newVariable->DefaultData = aDataObject;
		newVariable->Name = aVariableName;
		myGraph->myVariables.insert({ aVariableName, newVariable });
	}

	void RemoveNode(size_t aNodeUID);
	void RemoveVariable(const std::string& aVariableName);

	std::shared_ptr<SGNode_GetVariable> AddGetVariableNode(const std::string& aVariableName);
	std::shared_ptr<SGNode_SetVariable> AddSetVariableNode(const std::string& aVariableName);

	//~ Begin GraphEditorSchema Interface
	bool CanCreateEdge(size_t aSourcePinUID, size_t aTargetPinUID, std::string& outMesssage) const override;
	bool CreateEdge(size_t aSourcePinUID, size_t aTargetPinUID) override;
	bool DisconnectPin(size_t aPinUID) override;
	bool RemoveEdge(size_t aEdgeUID) override;
	//~ End GraphEditorSchema Interface

	FORCEINLINE std::shared_ptr<ScriptGraph> GetGraph() const { return myGraph; }
	FORCEINLINE const std::vector<std::string>& GetEntryPoints() const { return myGraphEntryPoints; }

	//BENNE FUNCTION
	FORCEINLINE const std::shared_ptr<ScriptGraphNode>& GetEntryPoint(const std::string& aEntryPointHandle) const
	{
		if (myGraph->myEntryPoints.contains(aEntryPointHandle))
		{
			return myGraph->myEntryPoints.at(aEntryPointHandle);
		}

		return nullptr;
	}

	FORCEINLINE const std::unordered_map<std::string, std::shared_ptr<ScriptGraphVariable>>& GetVariables() const { return myGraph->myVariables; }

	FORCEINLINE static const std::unordered_map<std::string, ScriptGraphNodeClass>& GetSupportedNodeTypes()
	{
		CreateNodeCDOs();
		return MyNodeTypesMap();
	}
};
