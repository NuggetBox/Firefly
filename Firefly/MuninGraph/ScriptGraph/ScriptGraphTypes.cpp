#include "MuninGraph.pch.h"
#include "ScriptGraphTypes.h"

const ScriptGraphColor ScriptGraphColor::White = { 255, 255, 255, 255 };
const ScriptGraphColor  ScriptGraphColor::Black = { 0, 0, 0, 255 };

const ScriptGraphType ScriptGraphType::NullType;

std::shared_ptr<const ScriptGraphType> ScriptGraphDataTypeRegistry::GetType(const std::type_index& aType)
{
	if (const auto typeIt = MyTypesMap().find(aType); typeIt != MyTypesMap().end())
	{
		return typeIt->second;
	}

	return nullptr;
}

std::shared_ptr<const ScriptGraphType> ScriptGraphDataTypeRegistry::GetType(const std::string& aType)
{
	if (const auto typeIt = myStringToType.find(aType); typeIt != myStringToType.end())
	{
		return GetType(typeIt->second);
	}

	return nullptr;
}

std::shared_ptr<const ScriptGraphType> ScriptGraphDataTypeRegistry::GetTypeFromFriendlyName(
	const std::string& aFriendlyName)
{
	if(const auto typeIt = myFriendlyNameToType.find(aFriendlyName); typeIt != myFriendlyNameToType.end())
	{
		return GetType(typeIt->second);
	}

	return nullptr;
}

void ScriptGraphDataTypeRegistry::RenderEditInPlaceWidget(const std::type_index& aType, const std::string& aContainerUUID, void* const aDataPtr)
{
	if (const auto typeIt = MyTypesMap().find(aType); typeIt != MyTypesMap().end())
	{
		const std::shared_ptr<ScriptGraphType> type = typeIt->second;
		if (type->CanConstructInPlace)
		{
			type->RenderConstructWidget(aContainerUUID, aDataPtr, *type);
		}
	}
}

std::string ScriptGraphDataTypeRegistry::GetString(const std::type_index& aType, void* aDataPtr)
{
	if(auto type = GetType(aType))
	{
		return type->ToString(aDataPtr, *type);
	}

	return "";
}

void ScriptGraphDataTypeRegistry::Serialize(const std::type_index& aType, const void* aDataPtr,
	std::vector<uint8_t>& outData)
{
	const auto type = GetType(aType);
	type->SerializeData(aDataPtr, *type, outData);
}

void ScriptGraphDataTypeRegistry::Deserialize(const std::type_index& aType, void* outDataPtr,
	const std::vector<uint8_t>& inData)
{
	const auto type = GetType(aType);
	type->DeserializeData(inData, *type, outDataPtr);
}

std::string ScriptGraphDataTypeRegistry::GetFriendlyName(const std::type_index& aType)
{
	if(auto type = GetType(aType))
	{
		return type->FriendlyName;
	}

	return std::string();
}

std::type_index ScriptGraphDataTypeRegistry::GetTypeFromString(const std::string& aType)
{
	if (const auto typeIt = myStringToType.find(aType); typeIt != myStringToType.end())
	{
		return typeIt->second;
	}

	return typeid(std::nullptr_t);
}

ScriptGraphDataObject ScriptGraphDataTypeRegistry::GetDataObjectOfType(const std::type_index& aType)
{
	if (const auto typeIt = MyTypesMap().find(aType); typeIt != MyTypesMap().end())
	{
		return typeIt->second->MakeDataObject();
	}

	assert(false && "That data type is not registered!");
	return ScriptGraphDataObject();
}

ScriptGraphDataObject ScriptGraphDataTypeRegistry::GetDataObjectOfType(const std::string& aType)
{
	if (const auto typeIt = myStringToType.find(aType); typeIt != myStringToType.end())
	{
		return GetDataObjectOfType(typeIt->second);
	}

	assert(false && "That data type is not registered!");
	return ScriptGraphDataObject();
}

ScriptGraphColor ScriptGraphDataTypeRegistry::GetDataTypeColor(const std::type_index aType)
{
	if(auto type = GetType(aType))
	{
		return type->Color;
	}

	return ScriptGraphColor::White;
}

ScriptGraphColor ScriptGraphDataTypeRegistry::GetDataTypeColorNormalized(const std::type_index aType)
{
	return GetDataTypeColor(aType).AsNormalized();
}

bool ScriptGraphDataTypeRegistry::IsTypeInPlaceConstructible(const std::type_index& aType)
{
	if(auto type = GetType(aType))
	{
		return type->CanConstructInPlace;
	}

	return false;
}

std::string ScriptGraphType::FetchTypeName(const std::type_index& aType) const
{
	const std::string MSVCTypeName = aType.name();
	const size_t fromPos = MSVCTypeName.find_first_of(' ');

	size_t toPos = MSVCTypeName.find_first_of(' ', fromPos);
	if (toPos == std::string::npos)
	{
		toPos = MSVCTypeName.size() - fromPos;
	}
	else
	{
		toPos = MSVCTypeName.size() - toPos;
	}

	std::string result = MSVCTypeName.substr(fromPos + 1, toPos);

	result = std::regex_replace(result, std::regex(R"(((\bclass\b)|(\bstruct\b))\s*)"), "");

	return result;
}

std::string ScriptGraphType::FetchSimpleTypeName(const std::string& aFullName) const
{
	const size_t toPos = aFullName.find_first_of('<');
	if (toPos != std::string::npos)
	{
		std::string result = aFullName.substr(0, toPos);
		return result;
	}

	return aFullName;
}

void ScriptGraphType::SerializeData(const void* aDataPtr, const ScriptGraphType& aTypeInfo,
                                    std::vector<uint8_t>& outData) const
{
	// By default we just serialize the data to a vector.
	outData.resize(aTypeInfo.GetTypeSize());
	memcpy_s(outData.data(), outData.capacity(), aDataPtr, aTypeInfo.GetTypeSize());
}

void ScriptGraphType::DeserializeData(const std::vector<uint8_t>& inData, const ScriptGraphType& aTypeInfo,
	void* outDataPtr) const
{
	// And by default we just copy data back to the pointer with nothing fancy.
	memcpy_s(outDataPtr, aTypeInfo.GetTypeSize(), inData.data(), inData.size());
}
