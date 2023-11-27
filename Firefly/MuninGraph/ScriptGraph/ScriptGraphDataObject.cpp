#include "MuninGraph.pch.h"
#include "ScriptGraphDataObject.h"

#include "ScriptGraphTypes.h"

void ScriptGraphDataObject::SetDataInternal(const void* aValue, size_t aSize)
{
	// FY!
	//memcpy_s(Ptr, TypeData->GetTypeSize(), &aValue, aSize);
	memcpy_s(Ptr, TypeData->GetTypeSize(), aValue, aSize);
}

void ScriptGraphDataObject::GetDataInternal(void* aValue, size_t aSize) const
{
	memcpy_s(aValue, aSize, Ptr, TypeData->GetTypeSize());
}

ScriptGraphDataObject::ScriptGraphDataObject(const std::type_index& aType): Ptr(nullptr)
{
	TypeData = ScriptGraphDataTypeRegistry::GetType(aType);
}

ScriptGraphDataObject::ScriptGraphDataObject(ScriptGraphDataObject&& other) noexcept
{
	std::swap(Ptr, other.Ptr);
	std::swap(TypeData, other.TypeData);
}

ScriptGraphDataObject::~ScriptGraphDataObject()
{
	free(Ptr);
	TypeData = nullptr;
}

ScriptGraphDataObject& ScriptGraphDataObject::operator=(const ScriptGraphDataObject& other)
{
	if (this != &other)
	{
		if (Ptr)
		{
			free(Ptr);
		}

		if (other.TypeData)
		{
			TypeData = ScriptGraphDataTypeRegistry::GetType(other.TypeData->GetType());
			if (other.Ptr)
			{
				const size_t ptrSize = TypeData->GetTypeSize();
				Ptr = malloc(ptrSize);
				memset(Ptr, 0, ptrSize);
				memcpy_s(Ptr, ptrSize, other.Ptr, other.TypeData->GetTypeSize());
			}
		}
		else
		{
			TypeData = nullptr;
			Ptr = nullptr;
		}
	}

	return *this;
}
