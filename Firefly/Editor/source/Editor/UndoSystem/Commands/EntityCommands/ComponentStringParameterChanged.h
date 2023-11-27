#pragma once
#include "Editor/UndoSystem/UndoCommand.h"

#include "ComponentParameterChangedCommand.h"

namespace Firefly
{
	class Component;
	enum class ParameterType;
}

class ComponentStringParameterChangedCommand : public UndoCommand
{
public:
	ComponentStringParameterChangedCommand(std::string& aTargetParameter, const std::string& aOldValue, 
		Ptr<Firefly::Component> aComponent = Ptr<Firefly::Component>(), const std::string& aParamName = "",
		Firefly::ParameterType aParamType = Firefly::ParameterType::Float);
	~ComponentStringParameterChangedCommand();
	void Execute() override;
	void Undo() override;

private:
	std::string& myTargetParameter; // this is the parameter to change

	std::string myOldValue;

	std::string myNewValue;

	Ptr<Firefly::Component> myComponent;

	std::string myParamName;
	Firefly::ParameterType myParamType;
};
