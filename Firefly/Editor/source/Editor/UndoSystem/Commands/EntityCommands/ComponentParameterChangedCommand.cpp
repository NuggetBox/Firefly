#include "EditorPch.h"
#include "ComponentParameterChangedCommand.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Event/EntityEvents.h"


ComponentParameterChangedCommand::ComponentParameterChangedCommand(void* aOldValue, void* aTargetParameter, size_t aByteCount,
	Ptr<Firefly::Component> aComponent, const std::string& aParamName, Firefly::ParameterType aParamType)
{
	myTargetParameter = aTargetParameter;
	myOldValue = aOldValue;
	myByteCount = aByteCount;
	myNewValue = new char[aByteCount];
	myComponent = aComponent;
	myParamName = aParamName;
	myParamType = aParamType;
	memcpy_s(myNewValue, myByteCount, aTargetParameter, myByteCount);
}

ComponentParameterChangedCommand::~ComponentParameterChangedCommand()
{
	delete[] static_cast<char*>(myOldValue);
	delete[] static_cast<char*>(myNewValue);
}

void ComponentParameterChangedCommand::Execute()
{
	memcpy(myTargetParameter, myNewValue, myByteCount);
	if (myComponent.lock())
	{
		Firefly::EntityPropertyUpdatedEvent ev(myParamName, myParamType);
		myComponent.lock()->OnEvent(ev);
	}
}

void ComponentParameterChangedCommand::Undo()
{
	memcpy(myTargetParameter, myOldValue, myByteCount);
	if (myComponent.lock())
	{
		Firefly::EntityPropertyUpdatedEvent ev(myParamName, myParamType);
		myComponent.lock()->OnEvent(ev);
	}
}
