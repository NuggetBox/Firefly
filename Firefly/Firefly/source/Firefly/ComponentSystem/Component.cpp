#include "FFpch.h"
#include "Component.h"
#include <Firefly/ComponentSystem/ComponentRegistry.hpp>
#include <Firefly/Event/EntityEvents.h>
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"


namespace Firefly
{
	int GetByteSize(const Firefly::ParameterType& aType)
	{
		switch (aType)
		{
		case Firefly::ParameterType::Int:
			return sizeof(int);
		case Firefly::ParameterType::Float:
			return sizeof(float);
		case Firefly::ParameterType::Bool:
			return sizeof(bool);
		case Firefly::ParameterType::String:
			return sizeof(std::string);
		case Firefly::ParameterType::Vec2:
			return sizeof(Utils::Vector2f);
		case Firefly::ParameterType::Vec3:
			return sizeof(Utils::Vector3f);
		case Firefly::ParameterType::Vec4:
			return sizeof(Utils::Vector4f);
		case Firefly::ParameterType::Color:
			return sizeof(Utils::Vector4f);
		case Firefly::ParameterType::Entity:
			return sizeof(Ref<Firefly::Entity>);
		case Firefly::ParameterType::File:
			return sizeof(std::string);
		case Firefly::ParameterType::Enum:
			return sizeof(uint32_t);
		}
		return -1;
	}
	std::string ParameterTypeToString(ParameterType aType)
	{
		std::string result = "";
		switch (aType)
		{
		case ParameterType::Int:
			result = "Int";
			break;
		case ParameterType::Float:
			result = "Float";
			break;
		case ParameterType::String:
			result = "String";
			break;
		case ParameterType::Bool:
			result = "Bool";
			break;
		case ParameterType::Vec2:
			result = "Vec2";
			break;
		case ParameterType::Vec3:
			result = "Vec3";
			break;
		case ParameterType::Vec4:
			result = "Vec4";
			break;
		case ParameterType::Color:
			result = "Color";
			break;
		case ParameterType::Button:
			result = "Button";
			break;
		case ParameterType::Entity:
			result = "Entity";
			break;
		case ParameterType::File:
			result = "File";
			break;
		case ParameterType::Enum:
			result = "Enum";
			break;
		case ParameterType::List:
			result = "List";
			break;
		}

		return result;
	}

	ParameterType StringToParameterType(const std::string& aType)
	{
		ParameterType result = ParameterType::Int;
		if (aType == "Int")
		{
			result = ParameterType::Int;
		}
		else if (aType == "Float")
		{
			result = ParameterType::Float;
		}
		else if (aType == "String")
		{
			result = ParameterType::String;
		}
		else if (aType == "Bool")
		{
			result = ParameterType::Bool;
		}
		else if (aType == "Vec2")
		{
			result = ParameterType::Vec2;
		}
		else if (aType == "Vec3")
		{
			result = ParameterType::Vec3;
		}
		else if (aType == "Vec4")
		{
			result = ParameterType::Vec4;
		}
		else if (aType == "Color")
		{
			result = ParameterType::Color;
		}
		else if (aType == "Button")
		{
			result = ParameterType::Button;
		}
		else if (aType == "Entity")
		{
			result = ParameterType::Entity;
		}
		else if (aType == "File")
		{
			result = ParameterType::File;
		}
		else if (aType == "Enum")
		{
			result = ParameterType::Enum;
		}
		else if (aType == "List")
		{
			result = ParameterType::List;
		}
		return result;
	}

	//void SetParameterFromModificationas(Variable& aParam, EntityModification& aModification, Scene* aScene, bool aSetEntityFlag)
	//{
	//	switch (aParam.Type)
	//	{
	//	case Firefly::ParameterType::Int:
	//		*static_cast<int*>(aParam.Pointer) = *reinterpret_cast<int*>(&(aModification.Values[0]));
	//		break;

	//	case Firefly::ParameterType::Float:
	//		*static_cast<float*>(aParam.Pointer) = *reinterpret_cast<float*>(&(aModification.Values[0]));
	//		break;

	//	case Firefly::ParameterType::File:
	//	case Firefly::ParameterType::String:
	//		*static_cast<std::string*>(aParam.Pointer) = aModification.ModifiedString;
	//		break;

	//	case Firefly::ParameterType::Bool:
	//		*static_cast<bool*>(aParam.Pointer) = *reinterpret_cast<bool*>(&(aModification.Values[0]));
	//		break;

	//	case Firefly::ParameterType::Vec2:
	//	{

	//		auto x = *reinterpret_cast<float*>(&(aModification.Values[0]));
	//		auto y = *reinterpret_cast<float*>(&(aModification.Values[1]));
	//		*static_cast<Utils::Vector2f*>(aParam.Pointer) = { x,y };
	//		break;
	//	}

	//	case Firefly::ParameterType::Vec3:
	//	{
	//		auto x = *reinterpret_cast<float*>(&(aModification.Values[0]));
	//		auto y = *reinterpret_cast<float*>(&(aModification.Values[1]));
	//		auto z = *reinterpret_cast<float*>(&(aModification.Values[2]));
	//		*static_cast<Utils::Vector3f*>(aParam.Pointer) = { x, y, z };
	//		break;
	//	}

	//	case Firefly::ParameterType::Color:
	//	case Firefly::ParameterType::Vec4:
	//	{
	//		auto x = *reinterpret_cast<float*>(&(aModification.Values[0]));
	//		auto y = *reinterpret_cast<float*>(&(aModification.Values[1]));
	//		auto z = *reinterpret_cast<float*>(&(aModification.Values[2]));
	//		auto w = *reinterpret_cast<float*>(&(aModification.Values[3]));
	//		*static_cast<Utils::Vector4f*>(aParam.Pointer) = { x, y, z, w };
	//		break;
	//	}

	//	case Firefly::ParameterType::Button:
	//	case Firefly::ParameterType::Header:
	//	case Firefly::ParameterType::HeaderEnd:
	//		// do nothing
	//		break;

	//	case Firefly::ParameterType::Entity:
	//		aParam.EntityID = aModification.Values[0];
	//		if (aSetEntityFlag)
	//		{
	//			*reinterpret_cast<Ref<Firefly::Entity>*>(aParam.Pointer) = aScene->GetEntityByID(aModification.Values[0]);
	//		}
	//		break;
	//	case Firefly::ParameterType::Enum:
	//		*static_cast<int*>(aParam.Pointer) = *reinterpret_cast<int*>(&aModification.Values[0]);
	//		break;

	//	case Firefly::ParameterType::List:
	//	{
	//		if (aParam.ListType == Firefly::ParameterType::Entity)
	//		{
	//			if ()
	//				std::vector<Ref<Entity>>& vec = *reinterpret_cast<std::vector<Ref<Entity>>*>(aParam.Pointer);
	//			vec.clear();
	//			for (int i = 0; i < aModification.Values.size(); i++)
	//			{
	//				vec.push_back(aScene->GetEntityByID(aModification.Values[i]));
	//			}
	//		}
	//		else
	//		{
	//			//TODO: MAKE SURE THIS WORKS
	//			std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(aParam.Pointer);
	//			vec.resize(aModification.Values.size() * sizeof(uint64_t));
	//			const auto end = vec.end()._Ptr;
	//			const auto begin = vec.begin()._Ptr;
	//			int stepSize = 1;

	//			if (aParam.ListType == Firefly::ParameterType::Vec2)
	//			{
	//				stepSize = 2;
	//			}
	//			else if (aParam.ListType == Firefly::ParameterType::Vec3)
	//			{
	//				stepSize = 3;
	//			}
	//			else if (aParam.ListType == Firefly::ParameterType::Vec4 || aParam.ListType == Firefly::ParameterType::Color)
	//			{
	//				stepSize = 4;
	//			}

	//			for (int i = 0; i < aModification.Values.size(); i += stepSize)
	//			{
	//				auto& value = aModification.Values[i];
	//				memcpy(begin + i * sizeof(uint64_t), &value, sizeof(uint64_t) * stepSize);
	//			}
	//		}
	//		break;
	//	}
	//	}
	//}

	void CopyParameterValue(Variable& aParam, const Variable& aParamToCopy, Scene* aScene, bool aSetEntityFlag)
	{
		switch (aParam.Type)
		{
		case Firefly::ParameterType::Int:
			*static_cast<int*>(aParam.Pointer) = *reinterpret_cast<const int*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Float:
			*static_cast<float*>(aParam.Pointer) = *reinterpret_cast<const float*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::File:
		case Firefly::ParameterType::String:
			*static_cast<std::string*>(aParam.Pointer) = *reinterpret_cast<const std::string*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Bool:
			*static_cast<bool*>(aParam.Pointer) = *reinterpret_cast<const bool*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Vec2:
			*static_cast<Utils::Vector2f*>(aParam.Pointer) = *reinterpret_cast<const Utils::Vector2f*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Vec3:
			*static_cast<Utils::Vector3f*>(aParam.Pointer) = *reinterpret_cast<const Utils::Vector3f*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Color:
		case Firefly::ParameterType::Vec4:
			*static_cast<Utils::Vector4f*>(aParam.Pointer) = *reinterpret_cast<const Utils::Vector4f*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::Button:
			// do nothing
			break;
		case Firefly::ParameterType::Entity:
			aParam.EntityID = aParamToCopy.EntityID;
			if (aSetEntityFlag)
			{
				*reinterpret_cast<Ptr<Firefly::Entity>*>(aParam.Pointer) = aScene->GetEntityByID(aParam.EntityID);
			}
			break;
		case Firefly::ParameterType::Enum:
			*static_cast<int*>(aParam.Pointer) = *reinterpret_cast<const int*>(aParamToCopy.Pointer);
			break;
		case Firefly::ParameterType::List:
		{
			if (aParam.ListType == Firefly::ParameterType::Entity)
			{
				std::vector<Ref<Entity>>& vec = *reinterpret_cast<std::vector<Ref<Entity>>*>(aParam.Pointer);
				vec.clear();
				for (auto& entity : *reinterpret_cast<const std::vector<Ref<Entity>>*>(aParamToCopy.Pointer))
				{
					vec.push_back(entity);
				}
			}
			else if (aParam.ListType == Firefly::ParameterType::String || aParam.ListType == Firefly::ParameterType::File)
			{
				std::vector<std::string>& vec = *reinterpret_cast<std::vector<std::string>*>(aParam.Pointer);
				std::vector<std::string>& vecToCopy = *reinterpret_cast<std::vector<std::string>*>(aParamToCopy.Pointer);
				vec.clear();
				for (auto& str : vecToCopy)
				{
					vec.push_back(str);
				}
			}
			else
			{
				std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(aParam.Pointer);
				vec = *reinterpret_cast<const std::vector<uint8_t>*>(aParamToCopy.Pointer);
			}
			break;
		}
		}
	}

	Firefly::Component::Component(const std::string& aName)
		: myName(aName)
	{
		myEntity = nullptr;
		myIsActive = true;
	}

	Component::Component(Component& aComp)
	{
		myEntity = aComp.myEntity;
		myName = aComp.myName;
		//myParameters = aComp.myParameters;
		myIsActive = true;
	}

	Ref<Component> Component::Duplicate(Ptr<Component> aComponentToCopy, Ptr<Entity> aTargetEnt)
	{
		if (aComponentToCopy.expired())
		{
			LOGERROR("Component::Duplicate: Component to copy is expired");
			return nullptr;
		}
		if (aTargetEnt.expired())
		{
			LOGERROR("Component::Duplicate: Target entity is expired");
			return nullptr;
		}
		auto targetEnt = aTargetEnt.lock();
		auto componentToCopy = aComponentToCopy.lock();

		auto comp = Firefly::ComponentRegistry::Create(componentToCopy->GetName());
		targetEnt->AddComponent(comp, false);
		comp->myIsActive = componentToCopy->myIsActive;
		for (int paramIndex = 0; paramIndex < Utils::Min(comp->mySerializedVariables.size(), componentToCopy->mySerializedVariables.size()); paramIndex++)
		{
			auto& param = comp->mySerializedVariables[paramIndex];
			auto& paramToCopy = componentToCopy->mySerializedVariables[paramIndex];

			if (param.Type != paramToCopy.Type)
			{
				LOGWARNING("Something went wrong while duplication component {} parameter {}", componentToCopy->GetName(), paramToCopy.Name);
				continue;
			}

			switch (param.Type)
			{
			case Firefly::ParameterType::Int:
				*static_cast<int*>(param.Pointer) = *static_cast<int*>(paramToCopy.Pointer);
				break;

			case Firefly::ParameterType::Float:
				*static_cast<float*>(param.Pointer) = *static_cast<float*>(paramToCopy.Pointer);
				break;

			case Firefly::ParameterType::File:
			case Firefly::ParameterType::String:
				*static_cast<std::string*>(param.Pointer) = *static_cast<std::string*>(paramToCopy.Pointer);
				break;

			case Firefly::ParameterType::Bool:
				*static_cast<bool*>(param.Pointer) = *static_cast<bool*>(paramToCopy.Pointer);
				break;

			case Firefly::ParameterType::Vec2:
			{
				Utils::Vector2f vec2 = *static_cast<Utils::Vector2f*>(paramToCopy.Pointer);
				*static_cast<Utils::Vector2f*>(param.Pointer) = { vec2.x, vec2.y };
			}
			break;

			case Firefly::ParameterType::Vec3:
			{
				Utils::Vector3f vec3 = *static_cast<Utils::Vector3f*>(paramToCopy.Pointer);
				*static_cast<Utils::Vector3f*>(param.Pointer) = { vec3.x, vec3.y, vec3.z };
			}
			break;

			case Firefly::ParameterType::Vec4:
			{
				Utils::Vector4f vec4 = *static_cast<Utils::Vector4f*>(paramToCopy.Pointer);
				*static_cast<Utils::Vector4f*>(param.Pointer) = { vec4.x, vec4.y, vec4.z, vec4.w };
			}
			break;

			case Firefly::ParameterType::Color:
			{
				Utils::Vector4f color = *static_cast<Utils::Vector4f*>(paramToCopy.Pointer);
				*static_cast<Utils::Vector4f*>(param.Pointer) = { color.x, color.y, color.z, color.w };
			}
			break;

			case Firefly::ParameterType::Button:
				//Do Nothing
				break;


			case Firefly::ParameterType::Entity:
			{
				Ptr<Entity> entity = *static_cast<Ptr<Entity>*>(paramToCopy.Pointer);
				if (!entity.expired())
				{
					param.EntityID = entity.lock()->GetID();
				}
				else
				{
					param.EntityID = paramToCopy.EntityID;
				}
				break;
			}
			case Firefly::ParameterType::Enum:
			{
				//surely
				*static_cast<int*>(param.Pointer) = *static_cast<int*>(paramToCopy.Pointer);
				break;
			}

			case Firefly::ParameterType::List:
			{
				if (param.ListType == Firefly::ParameterType::Entity)
				{
					auto& vectToCopy = *reinterpret_cast<const std::vector<Ptr<Entity>>*>(paramToCopy.Pointer);
					if (vectToCopy.size() == 0)
					{
						param.EntityIDVector = paramToCopy.EntityIDVector;
					}
					else
					{
						param.EntityIDVector.resize(vectToCopy.size());
						for (int i = 0; i < vectToCopy.size(); i++)
						{
							if (!vectToCopy[i].expired())
							{
								param.EntityIDVector[i] = vectToCopy[i].lock()->GetID();
							}
							else
							{
								param.EntityIDVector[i] = 0;
							}
							//param.EntityIDVector[i] = paramToCopy.EntityIDVector[i];	
						}
					}
				}
				else if (param.ListType == Firefly::ParameterType::String || param.ListType == Firefly::ParameterType::File)
				{
					auto& vec = *reinterpret_cast<std::vector<std::string>*>(param.Pointer);
					auto& vecToCopy = *reinterpret_cast<std::vector<std::string>*>(paramToCopy.Pointer);
					vec.clear();
					vec.resize(vecToCopy.size());
					for (int i = 0; i < vecToCopy.size(); i++)
					{
						vec[i] = vecToCopy[i];
					}
				}
				else
				{
					//the rest is pod
					std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(param.Pointer);
					vec = *reinterpret_cast<const std::vector<uint8_t>*>(paramToCopy.Pointer);
				}
				break;
			}
			}
			EntityPropertyUpdatedEvent ev(param.Name, param.Type);
			comp->OnEvent(ev);
		}
		return comp;
	}

	void Component::SetEntity(Entity* aEntity)
	{
		myEntity = aEntity;
	}


	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue)
	{
		assert(aType != ParameterType::Enum && "You need to use the enum overload function to be able to use enum types");
		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue, bool aUseAlpha)
	{
		//assert(aType == ParameterType::Color && "You can only use Color when chosing to use alpha or not");
		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		var.UseAlpha = aUseAlpha;
		mySerializedVariables.push_back(var);
	}


	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue, std::function<std::string(uint32_t)> aToStringFunction, int aEnumCount)
	{
		assert(aType == ParameterType::Enum && "You can only use this function with enums");
		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		var.EnumToStringFunction = aToStringFunction;
		var.EnumCount = aEnumCount;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue, const std::vector<std::string>& someEnumNames)
	{
		assert(aType == ParameterType::Enum && "You can only use this function with enums");
		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		var.EnumNames = someEnumNames;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue, float aIncrement, float aMin, float aMax)
	{
		assert(aType == ParameterType::Int || aType == ParameterType::Float || aType == ParameterType::Vec2 || aType == ParameterType::Vec3 || aType == ParameterType::Vec4 && "Cannont use this EditorVariable Function for types other than these");

		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		var.Increment = aIncrement;
		var.Min = aMin;
		var.Max = aMax;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorVariable(const std::string& aName, ParameterType aType, void* aValue, const char* someExtensions)
	{
		assert(aType == ParameterType::File && "You can only use this overload with files");
		Variable var;
		var.Name = aName;
		var.Type = aType;
		var.Pointer = aValue;
		var.FileExtensions = someExtensions;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorListVariable(const std::string& aName, ParameterType aType, void* aValue)
	{
		assert(aType != ParameterType::Bool);
		Variable var;
		var.Name = aName;
		var.Type = ParameterType::List;
		var.Pointer = aValue;
		var.ListType = aType;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, std::function<std::string(uint32_t)> aToStringFunction, int aEnumCount)
	{
		assert(aType == ParameterType::Enum && "You can only use this function with enums");
		Variable var;
		var.Name = aName;
		var.Type = ParameterType::List;
		var.Pointer = aValue;
		var.ListType = aType;
		var.EnumToStringFunction = aToStringFunction;
		var.EnumCount = aEnumCount;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, const std::vector<std::string>& someEnumNames)
	{
		assert(aType == ParameterType::Enum && "You can only use this function with enums");
		Variable var;
		var.Name = aName;
		var.Type = ParameterType::List;
		var.ListType = aType;
		var.Pointer = aValue;
		var.EnumNames = someEnumNames;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, float aIncrement, float aMin, float aMax)
	{
		assert(aType == ParameterType::Int || aType == ParameterType::Float || aType == ParameterType::Vec2 || aType == ParameterType::Vec3 || aType == ParameterType::Vec4 && "Cannont use this EditorVariable Function for types other than these");

		Variable var;
		var.Name = aName;
		var.Type = ParameterType::List;
		var.ListType = aType;
		var.Pointer = aValue;
		var.Increment = aIncrement;
		var.Min = aMin;
		var.Max = aMax;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, const std::string& someExtensions)
	{
		assert(aType == ParameterType::File && "You can only use this overload with files");

		Variable var;
		var.Name = aName;
		var.Type = ParameterType::List;
		var.Pointer = aValue;
		var.ListType = aType;
		var.FileExtensions = someExtensions;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorTooltip(const std::string& aToolTip)
	{
		if (mySerializedVariables.empty())
		{
			LOGERROR("No variable to add tooltip to!");
			return;
		}
		mySerializedVariables.back().Tooltip = aToolTip;
	}

	void Component::EditorListDefaultOpen()
	{
		mySerializedVariables.back().DefaultOpen = true;
	}

	void Component::EditorHideVariable()
	{
		if (mySerializedVariables.empty())
		{
			LOGERROR("No variable to hide!");
			return;
		}
		mySerializedVariables.back().Hidden = true;
	}

	void Component::EditorHeader(const std::string& aHeaderText)
	{
		Variable var;
		var.Name = aHeaderText;
		var.Type = ParameterType::Header;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorEndHeader()
	{
		Variable var;
		var.Name = "";
		var.Type = ParameterType::HeaderEnd;

		//count how many headers and end headers are in the vector
		int headerCount = 0;
		int headerEndCount = 0;
		for (auto& variable : mySerializedVariables)
		{
			if (variable.Type == ParameterType::Header)
			{
				headerCount++;
			}
			else if (variable.Type == ParameterType::HeaderEnd)
			{
				headerEndCount++;
			}
		}
		//calculate depth 
		int depth = headerCount - headerEndCount;
		assert(depth >= 0 && "Header end without header");
		var.HeaderDepth = depth;
		mySerializedVariables.push_back(var);
	}

	void Component::EditorButton(const std::string& aButtonText, std::function<void()> aFunctionToCall)
	{
		Variable var;
		var.Name = aButtonText;
		var.Type = ParameterType::Button;
		var.ButtonFunction = aFunctionToCall;
		mySerializedVariables.push_back(var);
	}

}
