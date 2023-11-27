#include "FFpch.h"
#include "VisualScriptComponent.h"

#include "MuninScriptGraph.h"
#include "ScriptEventManager.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/VisualScriptAsset.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Event/Event.h"

#include "Nodes/Events/VSNodes_Event.h"

#include "Utils/Timer.h"

namespace Firefly
{
	REGISTER_COMPONENT(VisualScriptComponent);

	VisualScriptComponent::VisualScriptComponent() : Component("VisualScriptComponent")
	{
		EditorVariable("Visual Script", ParameterType::File, &myVisualScriptPath, ".flow");

		EditorListVariable("VisualScriptVariableNames", ParameterType::String, &myHiddenVariableNames);
		EditorHideVariable();

		EditorListVariable("VisualScriptVariableTypes", ParameterType::String, &myHiddenVariableTypes);
		EditorHideVariable();

		EditorListVariable("VisualScriptVariableData", ParameterType::Int, &myHiddenVariableData);
		EditorHideVariable();

		EditorListVariable("VisualScriptVariableStrings", ParameterType::String, &myHiddenStrings);
		EditorHideVariable();

		EditorListVariable("VisualScriptVariableEntities", ParameterType::Entity, &myHiddenEntityPtrs);
		EditorHideVariable();

		myEditorOnlyVariableCount = mySerializedVariables.size();

		//Add additional visible editor variables below this
	}

	void VisualScriptComponent::EarlyInitialize()
	{
		LoadVisualScript();
	}

	void VisualScriptComponent::Initialize()
	{
		if (Application::Get().GetIsInPlayMode())
		{
			RunBeginPlay();
		}
	}

	void VisualScriptComponent::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>(BIND_EVENT_FN(VisualScriptComponent::OnPropertyUpdatedEvent));
		dispatcher.Dispatch<VisualScriptUpdatedEvent>(BIND_EVENT_FN(VisualScriptComponent::OnVisualScriptUpdated));

		if (begunPlay)
		{
			dispatcher.Dispatch<EntityOnCollisionEnterEvent>(BIND_EVENT_FN(VisualScriptComponent::OnCollisionEnter));
			dispatcher.Dispatch<EntityOnCollisionExitEvent>(BIND_EVENT_FN(VisualScriptComponent::OnCollisionExit));

			dispatcher.Dispatch<EntityOnTriggerEnterEvent>(BIND_EVENT_FN(VisualScriptComponent::OnTriggerEnter));
			dispatcher.Dispatch<EntityOnTriggerExitEvent>(BIND_EVENT_FN(VisualScriptComponent::OnTriggerExit));

			dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(VisualScriptComponent::OnUpdate));
			dispatcher.Dispatch<AppFixedUpdateEvent>(BIND_EVENT_FN(VisualScriptComponent::OnFixedUpdate));
		}
	}

	void VisualScriptComponent::RunBeginPlay()
	{
		if (!Application::Get().GetIsInPlayMode())
		{
			LOGERROR("Tried to run begin play on visual script {} but it was not in Play Mode", myVisualScriptPath);
			return;
		}

		if (!begunPlay)
		{
			if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
			{
				myVisualScriptAsset->VisualScriptGraph->Run("BeginPlay");
				myVisualScriptAsset->VisualScriptGraph->SetTicking(true);
				begunPlay = true;
			}
			else
			{
				LOGERROR("Tried to run begin play on visual script {} but it was not loaded correctly", myVisualScriptPath);
			}
		}
	}

	bool VisualScriptComponent::OnUpdate(AppUpdateEvent& aEvent)
	{
		if (aEvent.GetIsInPlayMode())
		{
			if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
			{
				myVisualScriptAsset->VisualScriptGraph->Tick(Utils::Timer::GetDeltaTime());
			}
		}

		return false;
	}

	bool VisualScriptComponent::OnFixedUpdate(AppFixedUpdateEvent& aEvent)
	{
		if (aEvent.GetIsInPlayMode())
		{
			if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
			{
				myVisualScriptAsset->VisualScriptGraph->Tick(Utils::Timer::GetFixedDeltaTime(), true);
			}
		}

		return false;
	}

	bool VisualScriptComponent::OnPropertyUpdatedEvent(EntityPropertyUpdatedEvent& aEvent)
	{
		if (aEvent.GetParamName() == "Visual Script")
		{
			if (!Application::Get().GetIsInPlayMode())
			{
				LoadVisualScript();
			}
		}
		//else //crash
		//{
		//	SetScriptVariables();
		//}

		return false;
	}

	bool VisualScriptComponent::OnVisualScriptUpdated(VisualScriptUpdatedEvent& aEvent)
	{
		if (aEvent.GetPath() == myVisualScriptPath)
		{
			LoadVisualScript();
		}
		return false;
	}

	bool VisualScriptComponent::OnCollisionEnter(EntityOnCollisionEnterEvent& aEvent)
	{
		if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
		{
			ScriptGraphNodePayload collisionPayload;
			collisionPayload.SetVariable("Other Entity", aEvent.GetCollidedEntity().lock()->GetID());
			collisionPayload.SetVariable("Contact Point", aEvent.GetContactPoint());
			collisionPayload.SetVariable("Contact Impulse", aEvent.GetImpulse());
			collisionPayload.SetVariable("Contact Normal", aEvent.GetNormal());
			myVisualScriptAsset->VisualScriptGraph->RunWithPayload("On Begin Overlap", collisionPayload);
		}
		return false;
	}

	bool VisualScriptComponent::OnCollisionExit(EntityOnCollisionExitEvent& aEvent)
	{
		if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
		{
			ScriptGraphNodePayload collisionPayload;
			collisionPayload.SetVariable("Other Entity", aEvent.GetCollidedEntity().lock()->GetID());
			myVisualScriptAsset->VisualScriptGraph->RunWithPayload("On End Overlap", collisionPayload);
		}
		return false;
	}

	bool VisualScriptComponent::OnTriggerEnter(EntityOnTriggerEnterEvent& aEvent)
	{
		if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
		{
			ScriptGraphNodePayload collisionPayload;
			collisionPayload.SetVariable("Other Entity", aEvent.GetCollidedEntity().lock()->GetID());
			collisionPayload.SetVariable("Contact Point", Utils::Vec3::Zero());
			collisionPayload.SetVariable("Contact Impulse", Utils::Vec3::Zero());
			collisionPayload.SetVariable("Contact Normal", Utils::Vec3::Zero());
			myVisualScriptAsset->VisualScriptGraph->RunWithPayload("On Begin Overlap", collisionPayload);
		}
		return false;
	}

	bool VisualScriptComponent::OnTriggerExit(EntityOnTriggerExitEvent& aEvent)
	{
		if (myVisualScriptAsset && myVisualScriptAsset->IsLoaded())
		{
			ScriptGraphNodePayload collisionPayload;
			collisionPayload.SetVariable("Other Entity", aEvent.GetCollidedEntity().lock()->GetID());
			myVisualScriptAsset->VisualScriptGraph->RunWithPayload("On End Overlap", collisionPayload);
		}
		return false;
	}

	ParameterType VisualScriptComponent::MuninGraphFriendlyNameToParameterType(const std::string& aMuninGraphFriendlyName) const
	{
		if (aMuninGraphFriendlyName == "String") return ParameterType::String;
		if (aMuninGraphFriendlyName == "Bool") return ParameterType::Bool;
		if (aMuninGraphFriendlyName == "Float") return ParameterType::Float;
		if (aMuninGraphFriendlyName == "Integer") return ParameterType::Int;
		if (aMuninGraphFriendlyName == "Entity") return ParameterType::Entity;
		if (aMuninGraphFriendlyName == "Vector3") return ParameterType::Vec3;
		if (aMuninGraphFriendlyName == "Quaternion") return ParameterType::Vec4;
		if (aMuninGraphFriendlyName == "Transform") FF_ASSERT(false && L"Transform doesn't have a ParameterType and can't be in the inspector");

		LOGERROR("No ParameterType matched the friendly name {}", aMuninGraphFriendlyName);
		return ParameterType::Int;
	}

	void VisualScriptComponent::LoadScriptVariables()
	{
		while (mySerializedVariables.size() > myEditorOnlyVariableCount)
		{
			mySerializedVariables.pop_back();
		}

		auto& dataInBytes = *reinterpret_cast<std::vector<uint8_t>*>(&myHiddenVariableData);

		auto scriptVariables = myVisualScriptAsset->VisualScriptGraph->GetGraphSchema()->GetVariables();

		int currentSize = 0;
		int currentEntity = 0;
		int currentString = 0;

		for (int i = 0; i < myHiddenVariableNames.size(); ++i)
		{
			const std::string& name = myHiddenVariableNames[i];

			auto it = std::find_if(scriptVariables.begin(), scriptVariables.end(), [name](const auto& keyVal)
				{
					return keyVal.first == name;
				});

			std::string type;
			bool shouldRemove = false;

			//Found a variable with same name in script
			if (it != scriptVariables.end())
			{
				type = (*it).second->GetTypeData()->GetFriendlyName();

				//The variable is not the same type anymore
				if (type != myHiddenVariableTypes[i])
				{
					shouldRemove = true;
				}
				//The variable is still the same type
				else
				{
					type = myHiddenVariableTypes[i];
					shouldRemove = false;
				}
			}
			//Variable with the name is not in the script anymore
			else
			{
				type = myHiddenVariableTypes[i];
				shouldRemove = true;
			}

			if (shouldRemove)
			{
				myHiddenVariableNames.erase(myHiddenVariableNames.begin() + i);
				myHiddenVariableTypes.erase(myHiddenVariableTypes.begin() + i);

				if (type == "String")
				{
					myHiddenStrings.erase(myHiddenStrings.begin() + currentString);
				}
				else if (type == "Entity")
				{
					myHiddenEntityPtrs.erase(myHiddenEntityPtrs.begin() + currentEntity);
				}
				else
				{
					if (currentSize >= dataInBytes.size())
					{
						LOGERROR("VisualScriptComponent::LoadScriptVariables: Error Removing Variable {}", myHiddenVariableNames[i]);
					}
					else
					{
						int size = dataInBytes[currentSize];
						dataInBytes.erase(dataInBytes.begin() + currentSize, dataInBytes.begin() + currentSize + size + 1);
					}
				}

				i--;
			}
			else
			{
				if (type == "String")
				{
					currentString++;
				}
				else if (type == "Entity")
				{
					currentEntity++;
				}
				else
				{
					currentSize += dataInBytes[currentSize];
					currentSize++;
				}
			}
		}

		bool skippedMyEntity = false;
		for (auto& [key, value] : scriptVariables)
		{
			std::string name = key;
			if (!skippedMyEntity)
			{
				if (name == "My Entity")
				{
					skippedMyEntity = true;
					continue;
				}
			}

			std::string type = value->GetTypeData()->GetFriendlyName();
			int size = value->GetTypeData()->GetTypeSize();

			bool shouldAdd = false;

			auto it = std::find(myHiddenVariableNames.begin(), myHiddenVariableNames.end(), key);
			if (it != myHiddenVariableNames.end())
			{
				shouldAdd = false;
			}
			else
			{
				shouldAdd = true;
			}

			if (shouldAdd)
			{
				myHiddenVariableNames.push_back(name);
				myHiddenVariableTypes.push_back(type);

				if (type == "String")
				{
					auto& string = *reinterpret_cast<std::string*>(value->Data.Ptr);
					myHiddenStrings.push_back(string);
					//bytes.push_back(string.size());
				}
				else if (type == "Entity")
				{
					uint64_t entityID = *reinterpret_cast<uint64_t*>(value->Data.Ptr);
					auto entity = Firefly::GetEntityWithID(entityID);
					myHiddenEntityPtrs.push_back(entity);
				}
				else
				{
					dataInBytes.push_back(size);
					uint8_t* data = reinterpret_cast<uint8_t*>(value->Data.Ptr);

					for (int i = 0; i < size; ++i)
					{
						dataInBytes.push_back(data[i]);
					}
				}
			}
		}

		currentSize = 0;
		currentString = 0;
		currentEntity = 0;

		for (int i = 0; i < myHiddenVariableNames.size(); ++i)
		{
			if (myHiddenVariableTypes[i] == "Transform")
			{
			}
			else if (myHiddenVariableTypes[i] == "Entity")
			{
				if (currentEntity >= myHiddenEntityPtrs.size())
				{
					LOGERROR("VisualScriptComponent::LoadScriptVariables: Out Of Entity Range {}", myHiddenVariableNames[i]);
					continue;
				}
				EditorVariable(myHiddenVariableNames[i], ParameterType::Entity, &myHiddenEntityPtrs[currentEntity]);
				currentEntity++;
				continue;
			}
			else if (myHiddenVariableTypes[i] == "String")
			{
				if (currentString >= myHiddenStrings.size())
				{
					LOGERROR("VisualScriptComponent::LoadScriptVariables: Out Of String Range {}", myHiddenVariableNames[i]);
					continue;
				}
				EditorVariable(myHiddenVariableNames[i], ParameterType::String, &myHiddenStrings[currentString]);
				currentString++;
				continue;
			}
			else
			{
				if (currentSize >= dataInBytes.size())
				{
					LOGERROR("VisualScriptComponent::LoadScriptVariables: Out Of Type Byte Range {}", myHiddenVariableNames[i]);
				}
				else
				{
					EditorVariable(myHiddenVariableNames[i], MuninGraphFriendlyNameToParameterType(myHiddenVariableTypes[i]), &dataInBytes[currentSize + 1]);
				}
			}

			if (currentSize >= dataInBytes.size())
			{
				LOGERROR("VisualScriptComponent::LoadScriptVariables: Out Of Regular Byte Range {}", myHiddenVariableNames[i]);
				continue;
			}
			currentSize += dataInBytes[currentSize];
			currentSize++;
		}
	}

	void VisualScriptComponent::SetScriptVariables()
	{
		const auto& scriptVariables = myVisualScriptAsset->VisualScriptGraph->GetGraphSchema()->GetVariables();

		if (!scriptVariables.contains("My Entity"))
		{
			LOGERROR("Visual Script {} has no My Entity variable", myVisualScriptPath);
		}
		else
		{
			ScriptGraphDataObject entityVar = ScriptGraphDataObject::Create<uint64_t>(myEntity->GetID());
			scriptVariables.at("My Entity")->Data = entityVar;
		}

		int currentSize = 0;
		int currentString = 0;
		int currentEntity = 0;

		auto& dataInBytes = *reinterpret_cast<std::vector<uint8_t>*>(&myHiddenVariableData);

		for (int i = 0; i < myHiddenVariableNames.size(); ++i)
		{
			const std::string& name = myHiddenVariableNames[i];
			const std::string& type = myHiddenVariableTypes[i];

			if (type == "String")
			{
				if (!scriptVariables.contains(name))
				{
					LOGERROR("VisualScriptComponent::SetScriptVariables: Couldn't find String script variable {} in script even after loading all", name);
					continue;
				}
				if (currentString >= myHiddenStrings.size())
				{
					LOGERROR("VisualScriptComponent::SetScriptVariables: Out Of String Range {}", name);
					continue;
				}
				const std::string& data = myHiddenStrings[currentString];
				scriptVariables.at(name)->Data.SetData(data);

				currentString++;
			}
			else if (type == "Entity")
			{
				if (currentEntity >= myHiddenEntityPtrs.size())
				{
					LOGERROR("VisualScriptComponent::SetScriptVariables: Out Of Entity Range {}", name);
					continue;
				}
				const auto& entity = myHiddenEntityPtrs[currentEntity];

				if (!entity.expired())
				{
					if (!scriptVariables.contains(name))
					{
						LOGERROR("VisualScriptComponent::SetScriptVariables: Couldn't find Entity script variable {} in script even after loading all", name);
						continue;
					}

					const uint64_t id = entity.lock()->GetID();
					scriptVariables.at(name)->Data.SetData(id);
				}

				currentEntity++;
			}
			else
			{
				if (!scriptVariables.contains(name))
				{
					LOGERROR("VisualScriptComponent::SetScriptVariables: Couldn't find script variable {} in script even after loading all", name);
					continue;
				}

				if (currentSize >= dataInBytes.size())
				{
					LOGERROR("VisualScriptComponent::SetScriptVariables: Out Of Byte Range {}", name);
					continue;
				}

				const int size = dataInBytes[currentSize];
				void* data = &dataInBytes[currentSize + 1];
				scriptVariables.at(name)->Data.SetDataRaw(data, size);

				currentSize += size;
				currentSize++;
			}
		}
	}

	void VisualScriptComponent::SubscribeToEvents()
	{
		const auto& schema = myVisualScriptAsset->VisualScriptGraph->GetGraphSchema();

		for (const auto& entryPoint : schema->GetEntryPoints())
		{
			const auto& entryPointNode = schema->GetEntryPoint(entryPoint);

			if (!entryPointNode)
			{
				LOGERROR("VisualScriptComponent::SubscribeToEvents: Unknown error 1 while looking for On Event node");
			}

			const auto& uuid = AsUUIDAwareSharedPtr(entryPointNode);
			const auto& eventNode = schema->GetGraph()->GetNodes().at(uuid->GetUID());

			if (!eventNode)
			{
				LOGERROR("VisualScriptComponent::SubscribeToEvents: Unknown error 2 while looking for On Event node");
			}

			if (eventNode->GetNodeTitle() == "On Event")
			{
				std::string eventName;

				if (eventNode->GetPinData<std::string>("Event Name", eventName))
				{
					ScriptEventManager::SubscribeToEvent(eventName, myVisualScriptAsset->VisualScriptGraph, myEntity->GetID(), AsUUIDAwareSharedPtr(eventNode)->GetUID());
				}
				else
				{
					LOGERROR("VisualScriptComponent::SubscribeToEvents: Couldn't fetch the name of the Event in an OnEvent node on Entity: {}", myEntity->GetName());
				}
			}
		}
	}

	void VisualScriptComponent::LoadVisualScript()
	{
		if (myVisualScriptPath.empty())
		{
			LOGWARNING("Tried to intialize a visual script component without a path");
			return;
		}

		if (!std::filesystem::exists(myVisualScriptPath))
		{
			LOGERROR("Tried to intialize a visual script component with an invalid or old path: {}", myVisualScriptPath);
			return;
		}

		myVisualScriptAsset = CreateRef<VisualScriptAsset>();
		myVisualScriptAsset->SetPath(myVisualScriptPath);
		VisualScriptImporter yo;
		yo.ImportVisualScript(myVisualScriptAsset);
		myVisualScriptAsset->SetLoaded();

		//myVisualScriptAsset = ResourceCache::GetAsset<VisualScriptAsset>(myVisualScriptPath, true);

		if (myVisualScriptAsset)
		{
			LoadScriptVariables();
			SetScriptVariables();
			SubscribeToEvents();
		}
	}
}
