#pragma once
#include "Editor/UndoSystem/UndoCommand.h"
#include "Firefly/ComponentSystem/Component.h"

namespace Firefly
{
	class Component;
}

class ComponentParameterChangedCommand : public UndoCommand
{
public:
	ComponentParameterChangedCommand(void* aOldValue, void* aTargetParameter, size_t aOldByteCount
		, Ptr<Firefly::Component> aComponent = Ptr<Firefly::Component>(), const std::string& aParamName = "",
		Firefly::ParameterType aParamType = Firefly::ParameterType::Float);
	~ComponentParameterChangedCommand();
	void Execute() override;
	void Undo() override;

private:
	void* myTargetParameter = nullptr; // this is the parameter to change

	void* myOldValue = nullptr;
	size_t myByteCount;

	void* myNewValue = nullptr;

	Ptr<Firefly::Component> myComponent;

	std::string myParamName;
	Firefly::ParameterType myParamType;
};
