#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

enum ParamTypes
{
	Bool,
	Float,
	Int,
	Count
};

struct ParamsInfo
{
	ParamTypes Type = ParamTypes::Count;
	std::string Name = "";
	uint64_t Id = -1;
	std::tuple<float, int, bool> Value = { 0,0,0 };
	uint32_t Condition = 0;
};

struct LayerInfo
{
	std::vector<ParamsInfo> Params;
};

struct NodeInfo
{
	uint64_t nodeId = -1;
	std::string nodeName = "";
	bool myIsStart = false;
	bool IsAnyState = false;
};

struct LinkInfo
{
	uint64_t InputNode = -1;
	uint64_t OutputNode = -1;
	std::vector<LayerInfo> Layers;
};

struct State;
struct StateTransitions
{
	Ref<State> myNextState;
	std::vector<ParamsInfo> myParams;
	bool CanTransition(std::unordered_map<std::string, ParamsInfo> aParamMap)
	{
		if (myParams.size() == 0)
			return false;
		for (size_t i = 0; i < myParams.size(); i++)
		{
			ParamsInfo p = aParamMap[myParams[i].Name];
			switch (myParams[i].Type)
			{
			case ParamTypes::Bool:
				if (std::get<bool>(p.Value) != myParams[i].Condition)
					return false;
				break;
			case ParamTypes::Float:
				if (!CheckValue<float>(std::get<float>(p.Value), std::get<float>(myParams[i].Value), myParams[i].Condition))
					return false;
			case ParamTypes::Int:
				if (!CheckValue<int>(std::get<int>(p.Value), std::get<int>(myParams[i].Value), myParams[i].Condition))
					return false;
			}
		}
		return true;
	}
	template <typename T>
	bool CheckValue(T& aFirstVal, T& aSecondVal, int aCondition)
	{
		if (aCondition == 0)
		{
			if (aFirstVal < aSecondVal)
				return true;
		}
		if (aCondition == 1)
		{
			if (aFirstVal == aSecondVal)
				return true;
		}
		if (aCondition == 2)
		{
			if (aFirstVal > aSecondVal)
				return true;
		}
		return false;
	}
};

struct State
{
	std::vector<Ref<StateTransitions>> myTransitions;
	std::string Name = "";
	uint64_t ID = -1;

	std::function<void()> Enter = [&]() {};
	std::function<void()> Update = [&]() {};
	std::function<void()> FixedUpdate = [&]() {};
	std::function<void()> Exit = [&]() {};
};

class StateMachine : public Firefly::Component
{
public:
	StateMachine();
	~StateMachine() = default;

	void EarlyInitialize() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::string GetFactoryName() { return "StateMachine"; }
	static Ref<Component> Create() { return CreateRef<StateMachine>(); }

	template<typename T>
	inline void SetValue(const std::string& aParamName, const T& aValue) { std::get<T>(myParamMap[aParamName].Value) = aValue; }
	template<typename T>
	inline T GetValue(const std::string& aParamName) { return std::get<T>(myParamMap[aParamName].Value); }

	void SetState(const std::string& aState);
	std::string GetCurrentState() { return myCurrentState->Name; }

	void LockStateMachine(bool aLock = true);

	void SetEnterFunction(const std::string& aName, std::function<void()> aFunc);
	void SetUpdateFunction(const std::string& aName, std::function<void()> aFunc);
	void SetFixedUpdate(const std::string& aName, std::function<void()> aFunc);
	void SetExitFunction(const std::string& aName, std::function<void()> aFunc);
	void SetFunctions(const std::string& aName, std::function<void()> aEnterFunc, std::function<void()> aUpdateFunc, std::function<void()> aExitFunc);

private:
	std::string myPath = "";
	std::unordered_map<std::string, ParamsInfo> myParamMap;
	std::vector<NodeInfo> myNodes;
	std::vector<LinkInfo> myLinks;

	Ref<State> myCurrentState = nullptr;
	std::vector<Ref<State>> myStates;

	void LoadStateMachine();
	void CreateTransitions();
	Ref<State> GetStateFromID(const uint64_t& aID);
	NodeInfo GetNodeFromID(const uint64_t& aID);

	bool myLockedFlag;
};