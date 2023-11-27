#include "FFpch.h"
#include "ForceFieldManager.h"

#include "ForceFieldComponent.h"

#include "Firefly/ComponentSystem/Entity.h"

Firefly::ForceFieldManager::~ForceFieldManager()
{
	delete myInstance;
}

Firefly::ForceFieldManager& Firefly::ForceFieldManager::Get()
{
	if (!myInstance)
	{
		myInstance = new ForceFieldManager();
	}

	return *myInstance;
}

void Firefly::ForceFieldManager::AddForceField(const Ptr<Entity>& aForceFieldEntity)
{
	if (!aForceFieldEntity.expired())
	{
		const auto& entity = aForceFieldEntity.lock();

		if (entity->HasComponent<ForceFieldComponent>())
		{
			myForceFieldEntities.push_back(aForceFieldEntity);
		}
		else
		{
			LOGWARNING("Tried to add a force field without force field component");
		}
	}
	else
	{
		LOGWARNING("Tried to add invalid force field entity to manager");
	}
}

void Firefly::ForceFieldManager::AddForceField(uint64_t aForceFieldEntityID)
{
	AddForceField(GetEntityWithID(aForceFieldEntityID));
}

void Firefly::ForceFieldManager::ClearForceFields()
{
	myForceFieldEntities.clear();
	myForceFields.clear();
}

const std::vector<ForceField>& Firefly::ForceFieldManager::GetForceFields()
{
	return myForceFields;
}

void Firefly::ForceFieldManager::UpdateForceFields()
{
	myForceFields.clear();

	for (int i = 0; i < myForceFieldEntities.size(); ++i)
	{
		if (myForceFieldEntities[i].expired() || GetEntityWithID(myForceFieldEntities[i].lock()->GetID()).expired() || !myForceFieldEntities[i].lock()->HasComponent<ForceFieldComponent>())
		{
			myForceFieldEntities.erase(myForceFieldEntities.begin() + i);
			i--;
		}
		else
		{
			myForceFields.push_back(myForceFieldEntities[i].lock()->GetComponent<ForceFieldComponent>().lock()->GetForceField());
		}
	}
}