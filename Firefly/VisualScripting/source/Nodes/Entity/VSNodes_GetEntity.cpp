#include "VSNodes_GetEntity.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"

void VSNode_GetEntityWithName::Init()
{
	CreateDataPin<std::string>("Name", PinDirection::Input);
	CreateDataPin<uint64_t>("Entity", PinDirection::Output);
}

size_t VSNode_GetEntityWithName::DoOperation()
{
	std::string name;

	if (GetPinData("Name", name))
	{
		const auto& ent = Firefly::GetEntityWithName(name);

		if (ent.expired())
		{
			SetPinData<uint64_t>("Entity", 0);
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		const uint64_t id = entity->GetID();
		SetPinData("Entity", id);
	}

	return 0;
}

void VSNode_IsEntityValid::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<bool>("Valid", PinDirection::Output);
}

size_t VSNode_IsEntityValid::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		bool valid = !ent.expired();
		SetPinData("Valid", valid);
	}

	return 0;
}

void VSNode_SetEntityActive::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<bool>("Flag", PinDirection::Input);
}

size_t VSNode_SetEntityActive::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		bool flag;
		if (GetPinData("Flag", flag))
		{
			const auto& ent = Firefly::GetEntityWithID(entityID).lock();
			if (ent)
			{
				ent->SetActive(flag);
			}
		}
	}

	return ExitViaPin("Out");
}