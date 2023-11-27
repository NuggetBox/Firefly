#include "VSNodes_Rotation.h"

#include "Firefly/Components/Physics/RigidbodyComponent.h"
#include "Utils/Math/Quaternion.h"

void VSNode_SetRotation::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("New Rotation", PinDirection::Input);
}

size_t VSNode_SetRotation::DoOperation()
{
	uint64_t entityID;
	Utils::Quaternion quaternion;

	if (GetPinData("Entity", entityID) && GetPinData("New Rotation", quaternion))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		if (entity->HasComponent<Firefly::RigidbodyComponent>())
		{
			entity->GetComponent<Firefly::RigidbodyComponent>().lock()->SetRotation(quaternion);
		}
		else
		{
			entity->GetTransform().SetRotation(quaternion);
		}

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_GetRotation::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Rotation", PinDirection::Output);
}

size_t VSNode_GetRotation::DoOperation()
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

		const Utils::Quaternion quaternion = entity->GetTransform().GetQuaternion();
		SetPinData("Rotation", quaternion);
	}

	return 0;
}

void VSNode_SetLocalRotation::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("New Rotation", PinDirection::Input);
}

size_t VSNode_SetLocalRotation::DoOperation()
{
	uint64_t entityID;
	Utils::Quaternion inQuat;

	if (GetPinData("Entity", entityID) && GetPinData("New Rotation", inQuat))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		if (entity->HasComponent<Firefly::RigidbodyComponent>())
		{
			Utils::Transform dummy = entity->GetTransform();
			dummy.SetLocalRotation(inQuat);
			entity->GetComponent<Firefly::RigidbodyComponent>().lock()->SetRotation(dummy.GetQuaternion());
		}
		else
		{
			entity->GetTransform().SetLocalRotation(inQuat);
		}

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_GetLocalRotation::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Rotation", PinDirection::Output);
}

size_t VSNode_GetLocalRotation::DoOperation()
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

		const Utils::Quaternion quaternion = entity->GetTransform().GetLocalQuaternion();
		SetPinData("Rotation", quaternion);
	}

	return 0;
}

void VSNode_AddRotation::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Delta Rotation", PinDirection::Input);
}

size_t VSNode_AddRotation::DoOperation()
{
	uint64_t entityID;
	Utils::Quaternion quaternion;

	if (GetPinData("Entity", entityID) && GetPinData("Delta Rotation", quaternion))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		if (entity->HasComponent<Firefly::RigidbodyComponent>())
		{
			entity->GetComponent<Firefly::RigidbodyComponent>().lock()->SetRotation(quaternion * entity->GetTransform().GetLocalQuaternion());
		}
		else
		{
			entity->GetTransform().AddRotation(quaternion);
		}

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_AddLocalRotation::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Delta Rotation", PinDirection::Input);
}

size_t VSNode_AddLocalRotation::DoOperation()
{
	uint64_t entityID;
	Utils::Quaternion quaternion;

	if (GetPinData("Entity", entityID) && GetPinData("Delta Rotation", quaternion))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		if (entity->HasComponent<Firefly::RigidbodyComponent>())
		{
			entity->GetComponent<Firefly::RigidbodyComponent>().lock()->SetRotation(entity->GetTransform().GetLocalQuaternion() * quaternion);
		}
		else
		{
			entity->GetTransform().AddLocalRotation(quaternion);
		}

		return ExitViaPin("Out");
	}

	return 0;
}