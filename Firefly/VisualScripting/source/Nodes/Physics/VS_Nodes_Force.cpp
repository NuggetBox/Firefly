#include "VS_Nodes_Force.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Utils/Math/Vector3.hpp"

void VSNode_AddForce::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Force", PinDirection::Input);
}

size_t VSNode_AddForce::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f inForce;

	if (GetPinData("Entity", entityID) && GetPinData("Force", inForce))
	{
		const auto& entity = Firefly::GetEntityWithID(entityID);

		if (entity.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		auto rgBody = entity.lock()->GetComponent<Firefly::RigidbodyComponent>();
		if (rgBody.expired())
		{
			return ExitWithError("No RigidBody On Entity");
		}
		rgBody.lock()->AddForce(inForce);
		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_SetForce::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Force", PinDirection::Input);
}

size_t VSNode_SetForce::DoOperation()
{
	uint64_t entityID;
	Utils::Vector3f inForce;

	if (GetPinData("Entity", entityID) && GetPinData("Force", inForce))
	{
		const auto& entity = Firefly::GetEntityWithID(entityID);

		if (entity.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		auto rgBody = entity.lock()->GetComponent<Firefly::RigidbodyComponent>();
		if (rgBody.expired())
		{
			return ExitWithError("No RigidBody On Entity");
		}

		if (!rgBody.lock()->GetActor())
		{
			return ExitWithError("No Physics Actor On Entity");
		}

		rgBody.lock()->GetActor()->ResetForce(Firefly::Physics::ForceType::Impulse);
		rgBody.lock()->AddForce(inForce, Firefly::Physics::ForceType::Impulse);

		return ExitViaPin("Out");
	}
	return 0;
}

void VSNode_ResetForce::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
}

size_t VSNode_ResetForce::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		const auto& entity = Firefly::GetEntityWithID(entityID);

		if (entity.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		auto rgBody = entity.lock()->GetComponent<Firefly::RigidbodyComponent>();
		if (rgBody.expired())
		{
			return ExitWithError("No RigidBody On Entity");
		}
		if (!rgBody.lock()->GetActor())
		{
			return ExitWithError("Rigidbody Has No Actor");
		}
		rgBody.lock()->GetActor()->ResetForce();

		return ExitViaPin("Out");
	}
	return 0;
}

void VSNode_GetLinearVelocity::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Velocity", PinDirection::Output);
}

size_t VSNode_GetLinearVelocity::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		const auto& entity = Firefly::GetEntityWithID(entityID);

		if (entity.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		auto rgBody = entity.lock()->GetComponent<Firefly::RigidbodyComponent>();
		if (rgBody.expired())
		{
			return ExitWithError("No RigidBody On Entity");
		}
		if (!rgBody.lock()->GetActor())
		{
			return ExitWithError("Rigidbody Has No Actor");
		}
		SetPinData("Velocity", rgBody.lock()->GetActor()->GetLinearVelocity());
	}
	return 0;
}