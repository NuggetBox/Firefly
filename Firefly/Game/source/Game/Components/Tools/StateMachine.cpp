#include "Gamepch.h"
#include "StateMachine.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Utils/InputHandler.h"
#include "nlohmann/json.hpp"
#include <fstream>

REGISTER_COMPONENT(StateMachine);

StateMachine::StateMachine()
	: Component("StateMachine")
{
	myPath = "";
	myLockedFlag = false;
	EditorVariable("State", Firefly::ParameterType::File, &myPath, ".state");
}

void StateMachine::EarlyInitialize()
{
	LoadStateMachine();
}

void StateMachine::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<EditorPlayEvent>([&](EditorPlayEvent& e)
		{
			if (myPath != "")
			{
				if (myCurrentState)
					myCurrentState->Enter();
			}
			return false;
		});
	dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& e)
		{
			if (!e.GetIsInPlayMode() || !myCurrentState || myLockedFlag)
				return false;
			myCurrentState->Update();

			for (size_t i = 0; i < myCurrentState->myTransitions.size(); i++)
			{
				if (myCurrentState->myTransitions[i]->CanTransition(myParamMap))
				{
					myCurrentState->Exit();
					myCurrentState = myCurrentState->myTransitions[i]->myNextState;
					myCurrentState->Enter();
				}
			}
			return false;
		});
	dispatcher.Dispatch<Firefly::AppFixedUpdateEvent>([&](Firefly::AppFixedUpdateEvent& e)
		{
			if (!e.GetIsInPlayMode() || !myCurrentState || myLockedFlag)
				return false;
			myCurrentState->FixedUpdate();
			// Maybe check transitions here idk
			return false;
		});
}

void StateMachine::SetState(const std::string& aState)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aState)
		{
			myCurrentState->Exit();
			myCurrentState = myStates[i];
			myCurrentState->Enter();
			return;
		}
	}
}

void StateMachine::LockStateMachine(bool aLock)
{
	myLockedFlag = aLock;
}

void StateMachine::SetEnterFunction(const std::string& aName, std::function<void()> aFunc)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aName)
		{
			myStates[i]->Enter = aFunc;
		}
	}
}

void StateMachine::SetUpdateFunction(const std::string& aName, std::function<void()> aFunc)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aName)
		{
			myStates[i]->Update = aFunc;
		}
	}
}

void StateMachine::SetFixedUpdate(const std::string& aName, std::function<void()> aFunc)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aName)
		{
			myStates[i]->FixedUpdate = aFunc;
		}
	}
}

void StateMachine::SetExitFunction(const std::string& aName, std::function<void()> aFunc)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aName)
		{
			myStates[i]->Exit = aFunc;
		}
	}
}

void StateMachine::SetFunctions(const std::string& aName, std::function<void()> aEnterFunc, std::function<void()> aUpdateFunc, std::function<void()> aExitFunc)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->Name == aName)
		{
			myStates[i]->Enter = aEnterFunc;
			myStates[i]->Update = aUpdateFunc;
			myStates[i]->Exit = aExitFunc;
		}
	}
}

void StateMachine::LoadStateMachine()
{
	if (myPath == "")
		return;

	nlohmann::json json;
	std::ifstream ifStream(myPath);
	if (ifStream.fail())
		return;
	ifStream >> json;

	for (size_t i = 0; i < json["Params"].size(); i++)
	{
		ParamsInfo param;
		param.Id = json["Params"][i]["ID"];
		param.Name = json["Params"][i]["Name"];
		param.Type = json["Params"][i]["Type"];
		myParamMap.insert({ param.Name, param });
	}

	for (size_t i = 0; i < json["Nodes"].size(); i++)
	{
		NodeInfo node;
		int id = json["Nodes"][i]["ID"];
		node.nodeId = id;
		node.nodeName = json["Nodes"][i]["Name"];
		node.myIsStart = json["Nodes"][i]["IsStart"];
		node.IsAnyState = json["Nodes"][i]["IsAnyState"];
		myNodes.push_back(node);
	}

	for (size_t i = 0; i < json["Links"].size(); i++)
	{
		LinkInfo link;
		link.InputNode = (int)json["Links"][i]["InNode"];
		link.OutputNode = (int)json["Links"][i]["OutNode"];
		for (size_t j = 0; j < json["Links"][i]["Layers"].size(); j++)
		{
			LayerInfo layer;
			for (size_t k = 0; k < json["Links"][i]["Layers"][j]["Params"].size(); k++)
			{
				ParamsInfo param;
				param.Id = json["Links"][i]["Layers"][j]["Params"][k]["ID"];
				param.Name = json["Links"][i]["Layers"][j]["Params"][k]["Name"];
				param.Type = json["Links"][i]["Layers"][j]["Params"][k]["Type"];
				param.Condition = json["Links"][i]["Layers"][j]["Params"][k]["Condition"];

				switch (param.Type)
				{
				case ParamTypes::Bool:
					std::get<bool>(param.Value) = json["Links"][i]["Layers"][j]["Params"][k]["Value"];
					break;
				case ParamTypes::Float:
					std::get<float>(param.Value) = json["Links"][i]["Layers"][j]["Params"][k]["Value"];
					break;
				}
				layer.Params.push_back(param);
			}
			link.Layers.push_back(layer);
		}
		myLinks.push_back(link);
	}
	ifStream.close();

	CreateTransitions();
}

void StateMachine::CreateTransitions()
{
	for (auto node : myNodes)
	{
		if (node.IsAnyState)
			continue;

		Ref<State> state = CreateRef<State>();
		state->Name = node.nodeName;
		state->ID = node.nodeId;
		if (node.myIsStart)
			myCurrentState = state;
		myStates.push_back(state);
	}

	for (size_t i = 0; i < myLinks.size(); i++)
	{
		for (auto layer : myLinks[i].Layers)
		{
			Ref<StateTransitions> trans = CreateRef<StateTransitions>();
			trans->myParams = layer.Params;
			trans->myNextState = GetStateFromID(myLinks[i].OutputNode);
			if (!GetNodeFromID(myLinks[i].InputNode).IsAnyState)
			{
				GetStateFromID(myLinks[i].InputNode)->myTransitions.push_back(trans);
			}
			else
			{
				for (auto state : myStates)
				{
					if (state != trans->myNextState)
					{
						state->myTransitions.push_back(trans);
					}
				}
			}
		}
	}
}

Ref<State> StateMachine::GetStateFromID(const uint64_t& aID)
{
	for (size_t i = 0; i < myStates.size(); i++)
	{
		if (myStates[i]->ID == aID)
		{
			return myStates[i];
		}
	}
	return nullptr;
}

NodeInfo StateMachine::GetNodeFromID(const uint64_t& aID)
{
	for (size_t i = 0; i < myNodes.size(); i++)
	{
		if (myNodes[i].nodeId == aID)
		{
			return myNodes[i];
		}
	}
	return NodeInfo();
}
