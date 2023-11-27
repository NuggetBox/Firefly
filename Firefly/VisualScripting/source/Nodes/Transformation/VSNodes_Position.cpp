#include "VSNodes_Position.h"

#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Utils/Math/Vector3.hpp"

void VSNode_SetPosition::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("New Position", PinDirection::Input);
}

size_t VSNode_SetPosition::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f inPosition;

	if (GetPinData("Entity", entityID) && GetPinData("New Position", inPosition))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		entity->GetTransform().SetPosition(inPosition);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_GetPosition::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Position", PinDirection::Output);
}

size_t VSNode_GetPosition::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		SetPinData("Position", entity->GetTransform().GetPosition());
	}

	return 0;
}

void VSNode_SetLocalPosition::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("New Position", PinDirection::Input);
}

size_t VSNode_SetLocalPosition::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f inPosition;

	if (GetPinData("Entity", entityID) && GetPinData("New Position", inPosition))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		entity->GetTransform().SetLocalPosition(inPosition);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_GetLocalPosition::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Position", PinDirection::Output);
}

size_t VSNode_GetLocalPosition::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		SetPinData("Position", entity->GetTransform().GetLocalPosition());
	}

	return 0;
}

void VSNode_AddPosition::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Position Delta", PinDirection::Input);
}

size_t VSNode_AddPosition::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f positionDelta;

	if (GetPinData("Entity", entityID) && GetPinData("Position Delta", positionDelta))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		entity->GetTransform().AddPosition(positionDelta);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_AddLocalPosition::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Position Delta", PinDirection::Input);
}

size_t VSNode_AddLocalPosition::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f positionDelta;

	if (GetPinData("Entity", entityID) && GetPinData("Position Delta", positionDelta))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		entity->GetTransform().AddLocalPosition(positionDelta);

		return ExitViaPin("Out");
	}

	return 0;
}