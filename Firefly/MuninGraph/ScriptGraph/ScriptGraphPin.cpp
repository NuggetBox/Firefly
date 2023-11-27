#include "MuninGraph.pch.h"
#include "ScriptGraphPin.h"

#include "ScriptGraphTypes.h"
#include "../Graph/NodeGraphPin.h"

void ScriptGraphPin::InitVariableBlock(const std::type_index& aType)
{
	assert(myType == ScriptGraphPinType::Variable && "This is not a Variable pin!");
	SetDataObject(ScriptGraphDataTypeRegistry::GetDataObjectOfType(aType));
}

bool ScriptGraphPin::GetRawData(void* outData, size_t outDataSize) const
{
	if (outDataSize >= myData.TypeData->GetTypeSize())
	{
		memcpy_s(outData, outDataSize, myData.Ptr, myData.TypeData->GetTypeSize());
		return true;
	}

	return false;
}

void ScriptGraphPin::SetRawData(const void* inData, size_t inDataSize)
{
	assert(myData.TypeData->GetTypeSize() >= inDataSize);
	memcpy_s(myData.Ptr, myData.TypeData->GetTypeSize(), inData, inDataSize);
}
