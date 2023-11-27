#include "EditorPch.h"
#include "ComponentStringParameterChanged.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/ComponentSystem/Component.h"

ComponentStringParameterChangedCommand::ComponentStringParameterChangedCommand(std::string& aTargetParameter, const std::string& aOldValue, 
	Ptr<Firefly::Component> aComponent, const std::string& aParamName, Firefly::ParameterType aParamType)
	: myTargetParameter(aTargetParameter)
{
	myNewValue = myTargetParameter;
	myOldValue = aOldValue;
	myComponent = aComponent;
	myParamName = aParamName;
	myParamType = aParamType;
}

ComponentStringParameterChangedCommand::~ComponentStringParameterChangedCommand()
{
}

void ComponentStringParameterChangedCommand::Execute()
{
	myTargetParameter = myNewValue;

	if (myComponent.lock())
	{
		Firefly::EntityPropertyUpdatedEvent ev(myParamName, myParamType);
		myComponent.lock()->OnEvent(ev);
	}
}

void ComponentStringParameterChangedCommand::Undo()
{
	myTargetParameter = myOldValue;

	if (myComponent.lock())
	{
		Firefly::EntityPropertyUpdatedEvent ev(myParamName, myParamType);
		myComponent.lock()->OnEvent(ev);
	}
}
