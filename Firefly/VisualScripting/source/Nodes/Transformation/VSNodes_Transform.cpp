#include "VSNodes_Transform.h"
#include "Utils/Math/Transform.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"

void VSNode_GetEntityTransform::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input, false);
	CreateDataPin<Utils::Transform>("Out Transform", PinDirection::Output, false);
}

size_t VSNode_GetEntityTransform::DoOperation()
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

		SetPinData("Out Transform", entity->GetTransform());
	}

	return 0;
}

void VSNode_SetEntityTransform::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input, false);
	CreateDataPin<Utils::Transform>("Transform", PinDirection::Input, false);
}

size_t VSNode_SetEntityTransform::DoOperation()
{
	uint64_t entityID;
	Utils::Transform transform;

	if (GetPinData("Entity", entityID) && GetPinData("Transform", transform))
	{
		const auto& ent = Firefly::GetEntityWithID(entityID);

		if (ent.expired())
		{
			return ExitWithError("Invalid Entity");
		}

		const auto& entity = ent.lock();

		entity->GetTransform().SetTransform(transform);
		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_GetEntityLocalTransform::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input, false);
	CreateDataPin<Utils::Transform>("Out Transform", PinDirection::Output, false);
}

size_t VSNode_GetEntityLocalTransform::DoOperation()
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

		Utils::Vector3f position = entity->GetTransform().GetLocalPosition();
		Utils::Quaternion rotation = entity->GetTransform().GetLocalQuaternion();
		Utils::Vector3f scale = entity->GetTransform().GetLocalScale();
		Utils::Transform local(position, rotation, scale);
		SetPinData("Out Transform", local);
	}

	return 0;
}

void VSNode_SetEntityLocalTransform::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<uint64_t>("Entity", PinDirection::Input, false);
	CreateDataPin<Utils::Transform>("Transform", PinDirection::Input, false);
}

size_t VSNode_SetEntityLocalTransform::DoOperation()
{
	uint64_t entityID;
	Utils::Transform transform;

	if (GetPinData("Entity", entityID) && GetPinData("Transform", transform))
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
			dummy.SetLocalPosition(transform.GetPosition());
			dummy.SetLocalRotation(transform.GetQuaternion());
			dummy.SetLocalScale(transform.GetScale());

			const auto& comp = entity->GetComponent<Firefly::RigidbodyComponent>().lock();
			comp->Teleport(dummy.GetPosition());
			comp->SetRotation(dummy.GetQuaternion());
			//TODO: comp->SetScale(dummy.GetScale);
		}
		else
		{
			entity->GetTransform().SetLocalPosition(transform.GetPosition());
			entity->GetTransform().SetLocalRotation(transform.GetQuaternion());
			entity->GetTransform().SetLocalScale(transform.GetScale());
		}

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_MakeTransform::Init()
{
	CreateDataPin<Utils::Vector3f>("Position", PinDirection::Input, false);
	CreateDataPin<Utils::Quaternion>("Rotation", PinDirection::Input, false);
	CreateDataPin<Utils::Vector3f>("Scale", PinDirection::Input, false);
	CreateDataPin<Utils::Transform>("Out Transform", PinDirection::Output, false);

	//Default scale to 1, 1, 1
	SetPinData("Scale", Utils::Vector3f::One());
}

size_t VSNode_MakeTransform::DoOperation()
{
	Utils::Vector3f position, scale;
	Utils::Quaternion rotation;

	if (GetPinData("Position", position) && GetPinData("Rotation", rotation) && GetPinData("Scale", scale))
	{
		const Utils::Transform transform(position, rotation, scale);
		SetPinData("Out Transform", transform);
	}

	return 0;
}

void VSNode_BreakTransform::Init()
{
	CreateDataPin<Utils::Transform>("Transform", PinDirection::Input, false);
	CreateDataPin<Utils::Vector3f>("Position", PinDirection::Output, false);
	CreateDataPin<Utils::Quaternion>("Rotation", PinDirection::Output, false);
	CreateDataPin<Utils::Vector3f>("Scale", PinDirection::Output, false);
}

size_t VSNode_BreakTransform::DoOperation()
{
	Utils::Transform inTransform;

	if (GetPinData("Transform", inTransform))
	{
		SetPinData("Position", inTransform.GetPosition());
		SetPinData("Rotation", inTransform.GetQuaternion());
		SetPinData("Scale", inTransform.GetScale());
	}

	return 0;
}

//void VSNode_LerpTransform::Init()
//{
//	CreateExecPin("In", PinDirection::Input, true);
//	CreateExecPin("Out", PinDirection::Input, true);
//
//	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
//	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
//	CreateDataPin<float>("Alpha", PinDirection::Input);
//	CreateDataPin<Utils::Transform>("Result", PinDirection::Output);
//}
//
//size_t VSNode_LerpTransform::DoOperation()
//{
//	Utils::Transform a, b;
//	float alpha;
//
//	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
//	{
//		const Utils::Transform result = Utils::Transform::Lerp(a, b, alpha);
//		SetPinData("Result", result);
//		return ExitViaPin("Out");
//	}
//
//	return 0;
//}