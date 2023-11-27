#include "EditorPch.h"
#include "ComponentAddCommand.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"

ComponentAddCommand::ComponentAddCommand(Ptr<Firefly::Entity> aTargetEntity, const std::string& aComponentFactoryName)
{
	myTargetEntity = aTargetEntity;
	myComponentFactoryName = aComponentFactoryName;
}

void ComponentAddCommand::Execute()
{
	if (!myComponent)
	{
		myComponent = Firefly::ComponentRegistry::Create(myComponentFactoryName);
	}
	myTargetEntity.lock()->AddComponent(myComponent);
	
}

void ComponentAddCommand::Undo()
{
	myTargetEntity.lock()->RemoveComponent(myComponent);
}
