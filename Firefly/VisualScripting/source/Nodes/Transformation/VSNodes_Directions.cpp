#include "VSNodes_Directions.h"

#include "Firefly/Components/Physics/RigidbodyComponent.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include "Firefly/ComponentSystem/Entity.h"

#include "Utils/Math/Transform.h"

void VSNode_GetEntityForward::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Forward", PinDirection::Output);
}

size_t VSNode_GetEntityForward::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Forward", entity->GetTransform().GetForward());
	}

	return 0;
}

void VSNode_GetEntityBackward::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Backward", PinDirection::Output);
}

size_t VSNode_GetEntityBackward::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Backward", entity->GetTransform().GetBackward());
	}

	return 0;
}

void VSNode_GetEntityRight::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Right", PinDirection::Output);
}

size_t VSNode_GetEntityRight::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Right", entity->GetTransform().GetRight());
	}

	return 0;
}

void VSNode_GetEntityLeft::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Left", PinDirection::Output);
}

size_t VSNode_GetEntityLeft::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Left", entity->GetTransform().GetLeft());
	}

	return 0;
}

void VSNode_GetEntityUp::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Up", PinDirection::Output);
}

size_t VSNode_GetEntityUp::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Up", entity->GetTransform().GetUp());
	}

	return 0;
}

void VSNode_GetEntityDown::Init()
{
	CreateDataPin<uint64_t>("Entity", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Down", PinDirection::Output);
}

size_t VSNode_GetEntityDown::DoOperation()
{
	uint64_t entityID;

	if (GetPinData("Entity", entityID))
	{
		FETCH_ENTITY;

		SetPinData("Down", entity->GetTransform().GetDown());
	}

	return 0;
}