#include "MuninGraph.pch.h"
#include "SGNode_Variable.h"
#include "ScriptGraph/ScriptGraphVariable.h"

void SGNode_SetVariable::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateVariablePin("VAR", PinDirection::Input);
	CreateVariablePin("Get", PinDirection::Output, true);
}

void SGNode_SetVariable::SetNodeVariable(const std::shared_ptr<ScriptGraphVariable>& aVariable)
{
	myVariable = aVariable;
	for (const auto& [pinUID, pin] : GetPins())
	{
		if (pin.GetType() == ScriptGraphPinType::Variable && pin.GetPinDirection() == PinDirection::Input)
		{
			RenamePin(pin.GetUID(), myVariable->Name);
		}
	}
}

size_t SGNode_SetVariable::DoOperation()
{
	if(myVariable)
	{
		if (GetRawPinData(myVariable->Name, myVariable->Data.Ptr, myVariable->GetTypeData()->GetTypeSize()))
		{
			SetRawPinData("Get", myVariable->Data.Ptr, myVariable->GetTypeData()->GetTypeSize());
		}
	}
	else
	{
		return ExitWithError("Invalid Variable!");
	}

	return ExitViaPin("Out");
}

ScriptGraphColor SGNode_SetVariable::GetNodeHeaderColor() const
{
	ScriptGraphColor col = ScriptGraphDataTypeRegistry::GetDataTypeColor(myVariable->GetTypeData()->GetType());
	col.A = 128.0f;
	return col;
}

void SGNode_GetVariable::Init()
{
	CreateVariablePin("VAR", PinDirection::Output);
}

void SGNode_GetVariable::SetNodeVariable(const std::shared_ptr<ScriptGraphVariable>& aVariable)
{
	myVariable = aVariable;
	for(const auto& [pinUID, pin] : GetPins())
	{
		if(pin.GetType() == ScriptGraphPinType::Variable && pin.GetPinDirection() == PinDirection::Output)
		{
			RenamePin(pin.GetUID(), myVariable->Name);
		}
	}
}

size_t SGNode_GetVariable::DoOperation()
{
	if(myVariable)
	{
		SetRawPinData(myVariable->Name, myVariable->Data.Ptr, myVariable->GetTypeData()->GetTypeSize());
	}
	else
	{
		return ExitWithError("Invalid Variable!");
	}

	return Exit();
}

ScriptGraphColor SGNode_GetVariable::GetNodeHeaderColor() const
{
	ScriptGraphColor col = ScriptGraphDataTypeRegistry::GetDataTypeColor(myVariable->GetTypeData()->GetType());
	col.A = 128.0f;
	return col;
}
