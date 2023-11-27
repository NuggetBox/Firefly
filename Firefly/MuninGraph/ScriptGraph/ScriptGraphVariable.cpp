#include "MuninGraph.pch.h"
#include "ScriptGraphVariable.h"

#include "ScriptGraphTypes.h"

void ScriptGraphVariable::ResetVariable()
{
	memcpy_s(Data.Ptr, GetTypeData()->GetTypeSize(), DefaultData.Ptr, DefaultData.TypeData->GetTypeSize());
}
