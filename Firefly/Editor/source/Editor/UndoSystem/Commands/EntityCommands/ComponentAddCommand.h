#pragma once

#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Component;
}
class ComponentAddCommand : public UndoCommand
{

public:
	ComponentAddCommand(Ptr<Firefly::Entity> aTargetEntity, const std::string& aComponentFactoryName );

	virtual void Execute() override;
	virtual void Undo() override;
private:
	Ptr<Firefly::Entity> myTargetEntity;
	Ref<Firefly::Component> myComponent; // needs to be a ref because it should save even when undo is called
	std::string myComponentFactoryName;
};