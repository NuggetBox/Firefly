#include "FFpch.h"
#include "SerializationUtils.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"

//Preloading includes
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Animations/Animator.h"
#include "Firefly/Components/Mesh/MeshComponent.h"
#include "Firefly/Components/Mesh/AnimatedMeshComponent.h"
#include "Firefly/Components/Animation/AnimatorComponent.h"
//

#include "Firefly/Event/EntityEvents.h"
#include "imnotify/imgui_notify.h" 
#include <Firefly/Asset/ResourceCache.h>
#include <random>
#include <Utils/Timer.h>

namespace Firefly
{
	void SerializationUtils::SerializeScene(nlohmann::json& aJson, Ptr<Scene> aScene)
	{
		aJson["Version"] = "1.0";
		auto& jsonEntArr = aJson["Entities"];
		auto entities = aScene.lock()->GetEntities();
		for (int entIndex = 0; entIndex < entities.size(); entIndex++)
		{
			const auto& entWeak = entities[entIndex];
			if (entWeak.expired())
			{
				LOGERROR("FAILED TO SERIALIZE EXPIRED ENTITY IN SCENE: {}", aScene.lock()->GetPath().filename().string());
				continue;
			}

			const auto& ent = entWeak.lock();
			if (ent->IsPrefab())
			{
				//if is entity root
				if (ent->GetPrefabRootEntityID() == ent->GetID())
				{
					SerializePrefabRoot(jsonEntArr[entIndex], ent);
				}
				else
				{
					SerializePrefabChild(jsonEntArr[entIndex], ent);
				}
			}
			else
			{
				SerializeEntity(jsonEntArr[entIndex], entities[entIndex]);
			}
		}
	}
	void Firefly::SerializationUtils::SerializeEntity(nlohmann::json& aJson, Ptr< Entity> aEntity, bool aIsPrefab)
	{
		auto entity = aEntity.lock();
		aJson["Name"] = entity->GetName();
		aJson["Tag"] = entity->GetTag();
		aJson["ID"] = entity->GetID();
		aJson["ParentID"] = entity->HasParent() ? entity->GetParentID() : 0;

		aJson["Position"] = { entity->GetTransform().GetLocalPosition().x, entity->GetTransform().GetLocalPosition().y, entity->GetTransform().GetLocalPosition().z };
		aJson["Rotation"] = { entity->GetTransform().GetLocalRotation().x, entity->GetTransform().GetLocalRotation().y, entity->GetTransform().GetLocalRotation().z };
		aJson["Scale"] = { entity->GetTransform().GetLocalScale().x, entity->GetTransform().GetLocalScale().y, entity->GetTransform().GetLocalScale().z };


		auto& jsonCompArr = aJson["Components"];
		auto components = entity->GetComponents();
		for (int compIndex = 0; compIndex < components.size(); compIndex++)
		{
			auto component = components[compIndex].lock();
			auto& jsonComp = jsonCompArr[compIndex];
			jsonComp["Name"] = component->GetName();

			auto& jsonParamArray = jsonComp["Parameters"];
			auto& params = component->GetSerializedVariables();
			for (int paramIndex = 0; paramIndex < params.size(); paramIndex++)
			{
				auto& param = params[paramIndex];
				if (param.Type != Firefly::ParameterType::Button) // no need to save button parameter
				{
					SerializeParameter(jsonParamArray[paramIndex], param, aIsPrefab);
				}
			}
		}
	}
	void SerializationUtils::SerializePrefabRoot(nlohmann::json& aJson, Ptr<Entity> aEntity)
	{
		auto entity = aEntity.lock();
		aJson["ID"] = entity->GetID();
		aJson["PrefabID"] = entity->GetPrefabID();
		aJson["CorrespondingSourceID"] = entity->GetCorrespondingSourceID();
		aJson["ParentID"] = entity->HasParent() ? entity->GetParentID() : 0;
		aJson["PrefabRootEntityID"] = entity->GetPrefabRootEntityID();

		auto& jsonModArr = aJson["Modifications"];
		auto& modifications = entity->GetModifications();
		for (int i = 0; i < modifications.size(); i++)
		{
			auto& mod = modifications[i];
			auto& jsonMod = jsonModArr[i];
			jsonMod["ID"] = mod.ID;
			jsonMod["Key"] = mod.Key;
			if (!mod.IntValues.empty())
			{
				jsonMod["IntValues"] = mod.IntValues;
			}
			if (!mod.FloatValues.empty())
			{
				jsonMod["FloatValues"] = mod.FloatValues;
			}
			if (!mod.StringValues.empty())
			{
				for (int i = 0; i < mod.StringValues.size(); i++)
				{
					jsonMod["StringValues"][i] = mod.StringValues[i];
				}
			}
			if (!mod.EntityIDValues.empty())
			{
				jsonMod["EntityIDValues"] = mod.EntityIDValues;
			}
			if (!mod.UintValues.empty())
			{
				jsonMod["UintValues"] = mod.UintValues;
			}
		}
	}
	void SerializationUtils::SerializePrefabChild(nlohmann::json& aJson, Ptr<Entity> aEntity)
	{
		auto entity = aEntity.lock();

		aJson["ID"] = entity->GetID();
		aJson["CorrespondingSourceID"] = entity->GetCorrespondingSourceID();
		aJson["PrefabRootEntityID"] = entity->GetPrefabRootEntityID();
		if (entity->GetParentID() == 0)
		{
			LOGERROR("Tried to serialize entity as a prefab child without a parent! ID: {}", entity->GetID());
		}
		aJson["ParentID"] = entity->GetParent().lock()->GetID();
	}
	void SerializationUtils::SerializeEntityAsPrefab(nlohmann::json& aJson, Ptr<Entity> aEntity)
	{
		auto entity = aEntity.lock();

		//generate random uint64_t value
		if (entity->GetPrefabID() == 0)
		{
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<uint64_t> dis;
			uint64_t randomID = dis(gen);
			aJson["PrefabID"] = randomID;
		}
		else
		{
			aJson["PrefabID"] = entity->GetPrefabID();
		}


		auto& jsonEntArray = aJson["Entities"];
		auto parentID = entity->HasParent() ? entity->GetParent().lock()->GetID() : 0;
		entity->SetParentIDUnsafe(0);
		SerializeEntity(jsonEntArray[0], entity, true);
		entity->SetParentIDUnsafe(parentID);

		//loop children
		auto children = entity->GetChildrenRecursive();
		for (int i = 0; i < children.size(); i++)
		{
			SerializeEntity(jsonEntArray[i + 1], children[i], true);
		}

	}

	void SerializationUtils::SerializeParameter(nlohmann::json& aJson, const Variable& aParameter, bool aIsPrefab)
	{
		aJson["Name"] = aParameter.Name;

		aJson["Type"] = ParameterTypeToString(aParameter.Type);

		switch (aParameter.Type)
		{
			case Firefly::ParameterType::Int:
				aJson["Value"] = *static_cast<int*>(aParameter.Pointer);
				break;

			case Firefly::ParameterType::Float:
				aJson["Value"] = *static_cast<float*>(aParameter.Pointer);
				break;

			case Firefly::ParameterType::String:
				aJson["Value"] = *static_cast<std::string*>(aParameter.Pointer);
				break;

			case Firefly::ParameterType::Bool:
				aJson["Value"] = *static_cast<bool*>(aParameter.Pointer);
				break;

			case Firefly::ParameterType::Vec2:
			{
				Utils::Vector2f vec2 = *static_cast<Utils::Vector2f*>(aParameter.Pointer);
				aJson["Value"] = { vec2.x, vec2.y };
			}
			break;

			case Firefly::ParameterType::Vec3:
			{
				Utils::Vector3f vec3 = *static_cast<Utils::Vector3f*>(aParameter.Pointer);
				aJson["Value"] = { vec3.x, vec3.y, vec3.z };
			}
			break;

			case Firefly::ParameterType::Vec4:
			{
				Utils::Vector4f vec4 = *static_cast<Utils::Vector4f*>(aParameter.Pointer);
				aJson["Value"] = { vec4.x, vec4.y, vec4.z, vec4.w };
			}
			break;

			case Firefly::ParameterType::Color:
			{
				Utils::Vector4f color = *static_cast<Utils::Vector4f*>(aParameter.Pointer);
				aJson["Value"] = { color.x, color.y, color.z, color.w };
			}
			break;

			case Firefly::ParameterType::Button:
				//Do Not Serialize
				break;

			case Firefly::ParameterType::File:
				aJson["Value"] = *static_cast<std::string*>(aParameter.Pointer);
				break;

			case Firefly::ParameterType::Entity:
			{
				Ptr<Entity> entity = *static_cast<Ptr<Entity>*>(aParameter.Pointer);
				if (entity.lock())
				{
					if (aIsPrefab)
					{
						aJson["Value"] = entity.lock()->GetCorrespondingSourceID();
					}
					else
					{
						aJson["Value"] = entity.lock()->GetID();
					}
				}
				else
				{
					aJson["Value"] = nullptr;
				}
				break;
			}
			case Firefly::ParameterType::Enum:
				aJson["Value"] = *static_cast<int*>(aParameter.Pointer);
				break;
			case Firefly::ParameterType::List:
			{
				auto& jsonList = aJson["Value"];
				if (aParameter.ListType == ParameterType::Entity)
				{
					auto& entityList = *static_cast<std::vector<Ptr<Entity>>*>(aParameter.Pointer);
					for (int i = 0; i < entityList.size(); i++)
					{
						if (entityList[i].lock())
						{
							jsonList[i] = entityList[i].lock()->GetID();
						}
						else
						{
							jsonList[i] = 0;
						}
					}
				}
				else if (aParameter.ListType == ParameterType::String || aParameter.ListType == ParameterType::File)
				{
					auto& stringList = *static_cast<std::vector<std::string>*>(aParameter.Pointer);
					for (int i = 0; i < stringList.size(); i++)
					{
						jsonList[i] = stringList[i];
					}
				}
				else
				{
					//we can do this sicne all other types are pod
					std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(aParameter.Pointer);

					const auto byteSize = Firefly::GetByteSize(aParameter.ListType);
					const auto size = vec.size() / byteSize;

					for (int i = 0; i < vec.size(); i++)
					{
						jsonList[i] = vec[i];
					}


				}
				break;
			}
		}
	}
	void DeserialzieParameter(simdjson::simdjson_result<simdjson::ondemand::value>& aJson, Variable& aParameter)
	{
		std::string typeString = std::string(aJson["Type"].get_string().value());
		ParameterType type = StringToParameterType(typeString);

		switch (type)
		{
			case Firefly::ParameterType::Int:
				*static_cast<int*>(aParameter.Pointer) = static_cast<int>(aJson["Value"].get_int64());
				break;

			case Firefly::ParameterType::Float:
				*static_cast<float*>(aParameter.Pointer) = static_cast<float>(aJson["Value"].get_double());
				break;

			case Firefly::ParameterType::Bool:
				*static_cast<bool*>(aParameter.Pointer) = aJson["Value"];
				break;

			case Firefly::ParameterType::Vec2:
			{
				auto vec2 = aJson["Value"].get_array();
				Utils::Vector2f vector;
				int index = 0;
				for (auto val : vec2)
				{
					vector[index] = static_cast<float>(val.get_double());
					index++;
				}
				*static_cast<Utils::Vector2f*>(aParameter.Pointer) = vector;
				break;

			}

			case Firefly::ParameterType::Vec3:
			{
				auto vec3 = aJson["Value"].get_array();
				Utils::Vector3f vector;
				int index = 0;
				for (auto val : vec3)
				{
					vector[index] = static_cast<float>(val.get_double());
					index++;
				}
				*static_cast<Utils::Vector3f*>(aParameter.Pointer) = vector;
			}
			break;

			case Firefly::ParameterType::Vec4:
			{
				auto vec4 = aJson["Value"].get_array();
				Utils::Vector4f vector;
				int index = 0;
				for (auto val : vec4)
				{
					vector[index] = static_cast<float>(val.get_double());
					index++;
				}
				*static_cast<Utils::Vector4f*>(aParameter.Pointer) = vector;
			}
			break;

			case Firefly::ParameterType::Color:
			{
				auto color = aJson["Value"].get_array();
				Utils::Vector4f vector;
				int index = 0;
				for (auto val : color)
				{
					vector[index] = static_cast<float>(val.get_double());
					index++;
				}
				*static_cast<Utils::Vector4f*>(aParameter.Pointer) = vector;
			}
			break;

			case Firefly::ParameterType::Button:
				//will never be called
				break;

			case Firefly::ParameterType::String:
			case Firefly::ParameterType::File:
				*static_cast<std::string*>(aParameter.Pointer) = std::string(aJson["Value"].get_string().value());
				break;


			case Firefly::ParameterType::Entity:
			{
				if (!aJson["Value"].is_null() && aJson["Value"].error() != simdjson::NO_SUCH_FIELD)
				{
					aParameter.EntityID = aJson["Value"].get_uint64();
				}
				else
				{
					aParameter.EntityID = 0;
				}
				break;
			}

			case Firefly::ParameterType::Enum:
			{
				if (aJson["Value"].error() != simdjson::NO_SUCH_FIELD)
				{
					*static_cast<int*>(aParameter.Pointer) = static_cast<int>(aJson["Value"].get_int64());
				}
				break;

			}
			case Firefly::ParameterType::List:
			{
				if (!aJson["Value"].is_null())
				{

					auto jsonList = aJson["Value"];
					if (aParameter.ListType == ParameterType::Entity)
					{
						auto& entityList = *static_cast<std::vector<Ptr<Entity>>*>(aParameter.Pointer);
						entityList.clear();
						for (auto jsonEnt : jsonList)
						{
							uint64_t id = jsonEnt.get_uint64();
							aParameter.EntityIDVector.push_back(id);
						}
					}
					else if (aParameter.ListType == ParameterType::String || aParameter.ListType == ParameterType::File)
					{
						auto& stringList = *static_cast<std::vector<std::string>*>(aParameter.Pointer);
						stringList.clear();
						for (auto jsonStr : jsonList)
						{
							stringList.push_back(std::string(jsonStr.get_string().value()));
						}
					}
					else
					{
						//we can do this sicne all other types are pod
						std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(aParameter.Pointer);
						vec.clear();
						for (auto byte : jsonList)
						{
							uint64_t byteNum = byte.get_uint64();
							vec.push_back(static_cast<uint8_t>(byteNum));
						}
					}
				}
				break;
			}


		}
	}


	std::shared_ptr<Entity> SerializationUtils::DeserializePrefabRoot(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt)
	{
		auto entity = CreateRef<Firefly::Entity>();
		entity->SetIDUnsafe(aJsonEnt["ID"].get_uint64());
		entity->SetCorrespondingSourceID(aJsonEnt["CorrespondingSourceID"].get_uint64());
		entity->SetParentIDUnsafe(aJsonEnt["ParentID"].get_uint64());
		entity->SetPrefabID(aJsonEnt["PrefabID"].get_uint64());
		entity->SetPrefabRootEntityID(aJsonEnt["PrefabRootEntityID"].get_uint64());

		//Collect Modifications
		auto jsonModArr = aJsonEnt["Modifications"];
		if (!jsonModArr.is_null())
		{
			for (auto jsonMod : jsonModArr)
			{
				EntityModification mod;
				mod.ID = jsonMod["ID"].get_uint64();
				mod.Key = jsonMod["Key"].get_string().take_value();
				if (jsonMod["IntValues"].error() != simdjson::NO_SUCH_FIELD)
				{
					auto arr = jsonMod["IntValues"];
					for (auto val : arr)
					{
						mod.IntValues.push_back(val.get_uint64());
					}
				}
				if (jsonMod["FloatValues"].error() != simdjson::NO_SUCH_FIELD)
				{
					auto arr = jsonMod["FloatValues"];
					for (auto val : arr)
					{
						mod.FloatValues.push_back(val.get_double());
					}
				}
				if (jsonMod["StringValues"].error() != simdjson::NO_SUCH_FIELD)
				{
					auto arr = jsonMod["StringValues"];
					for (auto val : arr)
					{
						mod.StringValues.push_back(std::string(val.get_string().take_value()));
					}
				}
				if (jsonMod["UintValues"].error() != simdjson::NO_SUCH_FIELD)
				{
					auto arr = jsonMod["UintValues"];
					for (auto val : arr)
					{
						mod.UintValues.push_back(val.get_uint64());
					}
				}
				if (jsonMod["EntityIDValues"].error() != simdjson::NO_SUCH_FIELD)
				{
					auto arr = jsonMod["EntityIDValues"];
					for (auto val : arr)
					{
						mod.EntityIDValues.push_back(val.get_uint64());
					}
				}


				entity->AddModification(mod, false);
			}
		}

		return entity;
	}
	std::shared_ptr<Entity> SerializationUtils::DeserializePrefabChild(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt)
	{
		auto entity = CreateRef<Firefly::Entity>();
		entity->SetIDUnsafe(aJsonEnt["ID"].get_uint64());
		entity->SetCorrespondingSourceID(aJsonEnt["CorrespondingSourceID"].get_uint64());
		entity->SetParentIDUnsafe(aJsonEnt["ParentID"].get_uint64());
		entity->SetPrefabRootEntityID(aJsonEnt["PrefabRootEntityID"].get_uint64());
		return entity;
	}

	std::shared_ptr<Entity> SerializationUtils::DeserializeEntity(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt)
	{
		auto entity = CreateRef<Firefly::Entity>();
		entity->SetName(std::string(aJsonEnt["Name"].get_string().take_value()));
		entity->SetTag(std::string(aJsonEnt["Tag"].get_string().take_value()));
		uint64_t id = aJsonEnt["ID"].get_uint64();
		if (id == 0)
		{
			id = Entity::GenerateRandomID();
		}
		entity->SetIDUnsafe(id);
		uint64_t parentID = aJsonEnt["ParentID"].get_uint64();
		entity->SetParentIDUnsafe(parentID);

		if (aJsonEnt["PrefabID"].error() != simdjson::NO_SUCH_FIELD)
		{
			entity->SetPrefabID(static_cast<uint64_t>(aJsonEnt["PrefabID"].get_uint64()));
		}

		auto jsonPos = aJsonEnt["Position"].get_array();
		Utils::Vector3f position;
		int index = 0;
		for (auto pos : jsonPos)
		{
			position[index] = static_cast<float>(pos.get_double());
			index++;
		}
		entity->GetTransform().SetLocalPosition(position);

		auto jsonRot = aJsonEnt["Rotation"].get_array();
		Utils::Vector3f rotation;
		index = 0;
		for (auto rot : jsonRot)
		{
			rotation[index] = static_cast<float>(rot.get_double());
			index++;
		}
		entity->GetTransform().SetLocalRotation(rotation);

		auto jsonScale = aJsonEnt["Scale"].get_array();
		Utils::Vector3f scale;

		index = 0;
		for (auto sca : jsonScale)
		{
			scale[index] = static_cast<float>(sca.get_double());
			index++;
		}
		entity->GetTransform().SetLocalScale(scale);

		entity->GetTransform().SetAsModified();

		auto jsonComponents = aJsonEnt["Components"];
		if (!jsonComponents.is_null())
		{
			auto jsonCompArr = jsonComponents.get_array();

			for (auto jsonComp : jsonCompArr)
			{
				std::string name = std::string(jsonComp["Name"].get_string().take_value());
				auto component = ComponentRegistry::Create(name);
				if (!component)
				{
					LOGERROR(__FUNCTION__" Component with name: \"{}\" could not be created.", name);
					continue;
				}
				entity->AddComponent(component, false);

				auto& params = component->GetSerializedVariablesMutable();
				auto jsonParamArr = jsonComp["Parameters"];

				if (jsonParamArr.is_null() || jsonParamArr.error() == simdjson::NO_SUCH_FIELD)
				{
					continue;
				}

				for (auto jsonParam : jsonParamArr)
				{
					if (jsonParam.is_null() || jsonParam.error() == simdjson::NO_SUCH_FIELD)
					{
						continue;
					}
					for (auto& param : params)
					{
						if (param.Type == ParameterType::Button || param.Type == ParameterType::Header || param.Type == ParameterType::HeaderEnd)
						{
							continue;
						}
						auto name = jsonParam["Name"];
						if (name.error() == simdjson::NO_SUCH_FIELD)
						{
							continue;
						}
						if (param.Name == jsonParam["Name"])
						{
							DeserialzieParameter(jsonParam, const_cast<Variable&>(param));
						}

						//EntityPropertyUpdatedEvent ev(param.Name, param.Type);
						//component->OnEvent(ev);
					}
				}
			}
		}
		return entity;
	}

	Ref<Scene> SerializationUtils::DeserializeScene(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonScene)
	{
		Ref<Scene> scene = CreateRef<Scene>();

		if (aJsonScene["Version"].error() != simdjson::NO_SUCH_FIELD)
		{
			Utils::Timer::Update();
			auto deserializeBeginTime = Utils::Timer::GetTotalTime();

			auto jsonEntities = aJsonScene["Entities"];
			if (!jsonEntities.is_null())
			{
				std::vector<int> prefabEntityIDs;
				auto jsonEntArr = aJsonScene["Entities"].get_array();
				for (auto jsonEnt : jsonEntArr)
				{
					Ref<Entity> ent;
					if (jsonEnt["PrefabID"].error() != simdjson::NO_SUCH_FIELD)
					{
						ent = SerializationUtils::DeserializePrefabRoot(jsonEnt);
					}
					else if (jsonEnt["PrefabRootEntityID"].error() != simdjson::NO_SUCH_FIELD)
					{
						//Load Prefab Child
						ent = SerializationUtils::DeserializePrefabChild(jsonEnt);
					}
					else
					{
						//Load normal entity
						ent = SerializationUtils::DeserializeEntity(jsonEnt);
					}
					scene->AddEntity(ent);
				}
			}

			Utils::Timer::Update();
			auto deserializeTime = Utils::Timer::GetTotalTime();
			LOGINFO("Deserializing: Deserializing entities took {} seconds", deserializeTime - deserializeBeginTime);

			auto sceneEntities = scene->GetEntities();



			////check if any ids are the same and if so, generate new ids (SUPER UNSAFE FOR PARAMETER LINKS)
			//for (int i = 0; i < sceneEntities.size(); i++)
			//{
			//	for (int j = i + 1; j < sceneEntities.size(); j++)
			//	{
			//		if (sceneEntities[i]->GetID() == sceneEntities[j]->GetID())
			//		{
			//			auto oldID = sceneEntities[i]->GetID();
			//			sceneEntities[i]->SetIDUnsafe(Entity::GenerateRandomID());
			//			for (int k = 0; k < sceneEntities.size(); k++)
			//			{
			//				if (sceneEntities[k]->GetParentID() == oldID)
			//				{
			//					sceneEntities[k]->SetParentIDUnsafe(sceneEntities[i]->GetID());
			//				}
			//			}
			//		}
			//	}
			//}

			//setup parent - child relationship
			for (int i = 0; i < sceneEntities.size(); i++)
			{
				auto ent = sceneEntities[i].lock();
				uint64_t parentID = ent->GetParentID();
				ent->SetParentIDUnsafe(0);
				ent->SetParent(parentID, -1, true, false);
			}

			Utils::Timer::Update();
			auto childTime = Utils::Timer::GetTotalTime();
			LOGINFO("Deserializing: Setting up parent-child relationships took {} seconds", childTime - deserializeTime);

			//find all mesh and animation components and preload the assets
			std::vector<std::string> meshPaths;
			std::vector<std::string> animatedMeshPaths;
			std::vector<std::string> animationPaths;

			for (int i = 0; i < sceneEntities.size(); i++)
			{
				auto components = sceneEntities[i].lock()->GetComponents();
				for (int j = 0; j < components.size(); j++)
				{
					auto& comp = components[j];
					CollectMeshesAndAnimationsPaths(comp, meshPaths, animatedMeshPaths, animationPaths);
				}
			}

			//find all prefabs and start preload
			for (int i = 0; i < sceneEntities.size(); i++)
			{
				auto ent = sceneEntities[i].lock();
				//if is prefab root make sure it has all of it's children
				if (ent->IsPrefabRoot())
				{
					auto prefabID = ent->GetPrefabID();
					ResourceCache::GetAsset<Prefab>(prefabID);
				}
			}

			//find all prefabs and collect meshes
			for (int i = 0; i < sceneEntities.size(); i++)
			{
				auto ent = sceneEntities[i].lock();
				//if is prefab root make sure it has all of it's children
				if (ent->IsPrefabRoot())
				{
					auto prefabID = ent->GetPrefabID();
					auto prefab = ResourceCache::GetAsset<Prefab>(prefabID, true);
					if (prefab)
					{
						auto prefabEntities = prefab->GetEntities();
						for (int j = 0; j < prefabEntities.size(); j++)
						{
							auto components = prefabEntities[j]->GetComponents();
							for (int k = 0; k < components.size(); k++)
							{
								auto& comp = components[k];
								CollectMeshesAndAnimationsPaths(comp, meshPaths, animatedMeshPaths, animationPaths);
							}
						}
					}
				}
			}

			//remove all duplicates
			std::sort(meshPaths.begin(), meshPaths.end());
			meshPaths.erase(std::unique(meshPaths.begin(), meshPaths.end()), meshPaths.end());

			std::sort(animatedMeshPaths.begin(), animatedMeshPaths.end());
			animatedMeshPaths.erase(std::unique(animatedMeshPaths.begin(), animatedMeshPaths.end()), animatedMeshPaths.end());

			std::sort(animationPaths.begin(), animationPaths.end());
			animationPaths.erase(std::unique(animationPaths.begin(), animationPaths.end()), animationPaths.end());



			Utils::Timer::Update();
			auto collectPreloadAssetsTime = Utils::Timer::GetTotalTime();
			LOGINFO("Preloading: Collecting all assets to be preloaded took {} seconds", collectPreloadAssetsTime - childTime);

			//Preload all collected assets
			auto futureThing1 = std::async([meshPaths]()
				{
					for (int i = 0; i < meshPaths.size(); i++)
					{
						ResourceCache::GetAsset<Mesh>(meshPaths[i]);
					}
				});

			auto futureThing2 = std::async([animatedMeshPaths]()
				{
					for (int i = 0; i < animatedMeshPaths.size(); i++)
					{
						ResourceCache::GetAsset<AnimatedMesh>(animatedMeshPaths[i]);
					}
				});

			auto futureThing3 = std::async([animationPaths]()
				{
					for (int i = 0; i < animationPaths.size(); i++)
					{
						ResourceCache::GetAsset<Animation>(animationPaths[i]);
					}
				});
			futureThing1.wait();
			futureThing2.wait();
			futureThing3.wait();

			Utils::Timer::Update();
			auto loadPreloadAssetsTime = Utils::Timer::GetTotalTime();
			LOGINFO("Preloading: Loading all preload assets took {} seconds", loadPreloadAssetsTime - collectPreloadAssetsTime);

			//find all prefabs and set base values
			auto lastSize = sceneEntities.size();
			for (int i = 0; i < sceneEntities.size(); i++)
			{
				auto ent = sceneEntities[i].lock();
				if (ent)
				{
					//if is prefab root make sure it has all of it's children
					if (ent->IsPrefabRoot())
					{
						Entity::UpdatePrefabValues(ent);
						//Have to account for possibly added or removed entities
						//i += sceneEntities.size() - lastSize;
						lastSize = sceneEntities.size();
					}
				}
			}

			Utils::Timer::Update();
			auto prefabBaseValueTime = Utils::Timer::GetTotalTime();
			LOGINFO("Deserializing: Finding all prefabs and setting base values took {} seconds", prefabBaseValueTime - loadPreloadAssetsTime);

			sceneEntities = scene->GetEntities();
			//set all prefabs modified values
			for (auto entWeak : sceneEntities)
			{
				auto ent = entWeak.lock();
				if (ent->IsPrefab() && ent->GetPrefabRootEntityID() == ent->GetID())
				{
					auto& modifications = ent->GetModifications();
					std::vector<EntityModification> modificationsToRemove;
					for (auto& mod : modifications)
					{
						const auto& key = mod.Key;
						auto componentNameOffset = key.find_first_of('_');
						auto componentName = key.substr(0, componentNameOffset);
						auto paramName = key.substr(componentNameOffset + 1, key.length() - componentNameOffset - 1);
						auto targetEntity = scene->GetEntityByID(mod.ID).lock();
						if (!targetEntity)
						{
							LOGWARNING("Prefab \"{}\" has a modification for entity with ID \"{}\" but that entity does not exist, Removing Modification", ent->GetName(), mod.ID);
							modificationsToRemove.push_back(mod);
							continue;
						}
						if (componentName == "Transform")
						{
							const float x = *reinterpret_cast<const float*>(&mod.FloatValues[0]);
							const float y = *reinterpret_cast<const float*>(&mod.FloatValues[1]);
							const float z = *reinterpret_cast<const float*>(&mod.FloatValues[2]);

							if (paramName == "Pos")
							{
								targetEntity->GetTransform().SetLocalPosition(x, y, z);
							}
							else if (paramName == "Rot")
							{
								const float w = *reinterpret_cast<const float*>(&mod.FloatValues[3]);
								targetEntity->GetTransform().SetLocalRotation(Utils::Quaternion(x, y, z, w));
							}
							else if (paramName == "Scale")
							{
								targetEntity->GetTransform().SetLocalScale(x, y, z);
							}
						}
						else
						{
							auto targetComponent = targetEntity->GetComponent(componentName).lock();
							if (!targetComponent)
							{
								LOGWARNING("When setting prefab modified values, Target component \"{}\", could not be found on the target entity (ID: {}). Removing modification.", componentName, targetEntity->GetID());
								modificationsToRemove.push_back(mod);
								continue;
							}
							for (auto& param : targetComponent->GetSerializedVariablesMutable())
							{
								if (param.Name == paramName)
								{

									switch (param.Type)
									{
										case Firefly::ParameterType::Int:
											*reinterpret_cast<int*>(param.Pointer) = mod.IntValues.front();
											break;

										case Firefly::ParameterType::Float:
											*reinterpret_cast<float*>(param.Pointer) = mod.FloatValues.front();
											break;

										case Firefly::ParameterType::Vec2:
											*reinterpret_cast<Utils::Vector2f*>(param.Pointer) = Utils::Vector2(mod.FloatValues[0], mod.FloatValues[1]);
											break;

										case Firefly::ParameterType::Vec3:
											*reinterpret_cast<Utils::Vector3f*>(param.Pointer) = Utils::Vector3(mod.FloatValues[0], mod.FloatValues[1], mod.FloatValues[2]);
											break;

										case Firefly::ParameterType::Color:
										case Firefly::ParameterType::Vec4:
											*reinterpret_cast<Utils::Vector4f*>(param.Pointer) = Utils::Vector4(mod.FloatValues[0], mod.FloatValues[1], mod.FloatValues[2], mod.FloatValues[3]);
											break;

										case Firefly::ParameterType::Bool:
											*reinterpret_cast<bool*>(param.Pointer) = mod.UintValues.front();
											break;

										case Firefly::ParameterType::File:
										case Firefly::ParameterType::String:
											*static_cast<std::string*>(param.Pointer) = mod.StringValues.front();
											break;

										case Firefly::ParameterType::Button:
											//Do Nothing
											break;


										case Firefly::ParameterType::Entity:
										{
											param.EntityID = mod.EntityIDValues.front();
											break;
										}
										case Firefly::ParameterType::Enum:
										{
											//surely
											*static_cast<uint32_t*>(param.Pointer) = mod.UintValues.front();
											break;
										}
										case Firefly::ParameterType::List:
										{
											switch (param.ListType)
											{
												case Firefly::ParameterType::Int:
												{
													auto& list = *reinterpret_cast<std::vector<int>*>(param.Pointer);
													list.clear();
													for (auto& val : mod.IntValues)
													{
														list.push_back(val);
													}
													break;
												}

												case Firefly::ParameterType::Float:
												{
													auto& list = *reinterpret_cast<std::vector<float>*>(param.Pointer);
													list.clear();
													for (auto& val : mod.FloatValues)
													{
														list.push_back(val);
													}
													break;
												}

												case Firefly::ParameterType::Vec2:
												{
													auto& list = *reinterpret_cast<std::vector<Utils::Vector2f>*>(param.Pointer);
													list.clear();
													for (size_t i = 0; i < mod.FloatValues.size(); i += 2)
													{
														list.push_back(Utils::Vector2(mod.FloatValues[i], mod.FloatValues[i + 1]));
													}
													break;
												}

												case Firefly::ParameterType::Vec3:
												{
													auto& list = *reinterpret_cast<std::vector<Utils::Vector3f>*>(param.Pointer);
													list.clear();
													for (size_t i = 0; i < mod.FloatValues.size(); i += 3)
													{
														list.push_back(Utils::Vector3(mod.FloatValues[i], mod.FloatValues[i + 1], mod.FloatValues[i + 2]));
													}
													break;
												}

												case Firefly::ParameterType::Color:
												case Firefly::ParameterType::Vec4:
												{
													auto& list = *reinterpret_cast<std::vector<Utils::Vector4f>*>(param.Pointer);
													list.clear();
													for (size_t i = 0; i < mod.FloatValues.size(); i += 4)
													{
														list.push_back(Utils::Vector4(mod.FloatValues[i], mod.FloatValues[i + 1], mod.FloatValues[i + 2], mod.FloatValues[i + 3]));
													}
													break;
												}

												case Firefly::ParameterType::Bool:
												{
													assert(false && "Bool lists are not supported");
													break;
												}

												case Firefly::ParameterType::File:
												case Firefly::ParameterType::String:
												{
													auto& list = *reinterpret_cast<std::vector<std::string>*>(param.Pointer);
													list.clear();
													for (auto& val : mod.StringValues)
													{
														list.push_back(val);
													}
													break;
												}

												case Firefly::ParameterType::Enum:
												{
													auto& list = *reinterpret_cast<std::vector<uint32_t>*>(param.Pointer);
													list.clear();
													for (auto& val : mod.UintValues)
													{
														list.push_back(val);
													}
													break;
												}

												case Firefly::ParameterType::Entity:
												{
													auto& list = *reinterpret_cast<std::vector<uint32_t>*>(param.Pointer);
													list.clear();
													for (auto& val : mod.UintValues)
													{
														list.push_back(val);
													}
													break;
												}


												default:
													assert(false && "Unknown parameter type");
													break;

											}
											break;
										}
									}
									EntityPropertyUpdatedEvent ev(param.Name, param.Type);
									targetComponent->OnEvent(ev);
								}
							}
						}

					}
					for (auto& mod : modificationsToRemove)
					{
						ent->RemoveModification(mod.Key, mod.ID);
					}
				}
			}


			Utils::Timer::Update();
			auto modifiedPrefabValueTime = Utils::Timer::GetTotalTime();
			LOGINFO("Deserializing: Setting all modified prefab values took {} seconds", modifiedPrefabValueTime - prefabBaseValueTime);

			//loop through parameters and set the entity pointers after all entities are in the scene
			for (int entIndex = 0; entIndex < sceneEntities.size(); entIndex++)
			{
				auto entity = sceneEntities[entIndex].lock();
				auto components = entity->GetComponents();
				for (int compIndex = 0; compIndex < components.size(); compIndex++)
				{
					auto component = components[compIndex].lock();
					for (auto& param : component->GetSerializedVariablesMutable())
					{
						if (param.Type == ParameterType::Entity)
						{
							auto ent = scene->GetEntityByID(param.EntityID).lock();
							if (ent)
							{
								*reinterpret_cast<Ptr<Entity>*>(param.Pointer) = ent;
							}
							else
							{
								param.EntityID = 0;
							}
						}
						else if (param.Type == ParameterType::List)
						{
							if (param.ListType == ParameterType::Entity)
							{
								auto& vec = *reinterpret_cast<std::vector<Ptr<Entity>>*>(param.Pointer);
								vec.clear();
								for (auto& id : param.EntityIDVector)
								{
									auto ent = scene->GetEntityByID(id);
									vec.push_back(ent);
								}
							}
						}
					}
				}
			}

			Utils::Timer::Update();
			auto entityPointerTime = Utils::Timer::GetTotalTime();
			LOGINFO("Deserializing: Setting all entity pointers in params took {} seconds", entityPointerTime - modifiedPrefabValueTime);
		}
		return scene;
	}

	void SerializationUtils::SaveEntityAsPrefab(Ptr<Entity> aEntity, const std::filesystem::path& aPath)
	{
		std::ofstream file(aPath);

		if (!file.is_open())
		{
			ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Could not save prefab at path:\n\"%s\"\nMake sure the file is checked out in perforce!", aPath.string().c_str() });
			return;
		}
		nlohmann::json json;
		Firefly::SerializationUtils::SerializeEntityAsPrefab(json, aEntity);
		if (file.bad())
		{
			LOGERROR("Could not save prefab {} to path: {}", aEntity.lock()->GetName(), aPath.string());
		}
		else if (!file.is_open())
		{
			LOGERROR("Could not open file {} to save prefab {}, make sure it is checked out", aPath.string(), aEntity.lock()->GetName());
		}
		else
		{
			file << json;
			file.close();
		}

		Firefly::ResourceCache::UnloadAsset(aPath);
		ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Prefab successfully saved to path:\n\"%s\"", aPath.string().c_str() });
	}


	void SerializationUtils::CollectMeshesAndAnimationsPaths(Ptr<Component> aComponent, std::vector<std::string>& aMeshes, std::vector<std::string>& aAnimatedMeshes, std::vector<std::string>& aAnimations)
	{
		if (aComponent.lock()->GetName() == MeshComponent::GetFactoryName())
		{
			for (auto& param : aComponent.lock()->GetSerializedVariables())
			{
				if (param.Name == "Mesh Path")
				{
					auto path = *reinterpret_cast<std::string*>(param.Pointer);
					if (!path.empty())
					{
						aMeshes.push_back(path);
					}
				}
			}
		}
		else if (aComponent.lock()->GetName() == AnimatedMeshComponent::GetFactoryName())
		{
			for (auto& param : aComponent.lock()->GetSerializedVariables())
			{
				if (param.Name == "Mesh Path")
				{
					auto path = *reinterpret_cast<std::string*>(param.Pointer);
					if (!path.empty())
					{
						aAnimatedMeshes.push_back(path);
					}
				}
			}
		}
		else if (aComponent.lock()->GetName() == AnimatorComponent::GetFactoryName())
		{
			for (auto& param : aComponent.lock()->GetSerializedVariables())
			{
				if (param.Name == "Animator")
				{
					auto animatorPath = *reinterpret_cast<std::string*>(param.Pointer);
					if (!animatorPath.empty())
					{
						auto animator = ResourceCache::GetAsset<Animator>(animatorPath, true);
						if (animator)
						{
							auto& layers = animator->GetLayers();
							for (auto& layer : layers)
							{
								auto& states = layer.GetStates();
								for (auto& state : states)
								{
									auto animationPath = state.second.AnimationPath;
									if (!animationPath.empty())
									{
										aAnimations.push_back(animationPath);
									}
								}
							}
						}
					}
				}
			}
		}
	}

}
