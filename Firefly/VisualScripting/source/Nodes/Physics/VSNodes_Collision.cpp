#include "VSNodes_Collision.h"

#include "Utils/Math/Vector3.hpp"

void VSNode_OnBeginOverlap::Init()
{
	CreateExecPin("Out", PinDirection::Output, false);
	CreateDataPin<uint64_t>("Other Entity", PinDirection::Output, false);
	CreateDataPin<Utils::Vector3f>("Contact Point", PinDirection::Output);
	CreateDataPin<Utils::Vector3f>("Contact Impulse", PinDirection::Output);
	CreateDataPin<Utils::Vector3f>("Contact Normal", PinDirection::Output);
}

//size_t VSNode_OnBeginOverlap::DoOperation()
//{
//	uint64_t entityID;
//
//	if (GetPinData("Other Entity", entityID))
//	{
//		FETCH_ENTITY;
//		SetPinData("Other Entity", entity->GetID());
//		return ExitViaPin("Out");
//	}
//
//	return 0;
//}

void VSNode_OnEndOverlap::Init()
{
	CreateExecPin("Out", PinDirection::Output, false);
	CreateDataPin<uint64_t>("Other Entity", PinDirection::Output, false);
}

//size_t VSNode_OnEndOverlap::DoOperation()
//{
//	uint64_t entityID;
//
//	if (GetPinData("Other Entity", entityID))
//	{
//		FETCH_ENTITY;
//		SetPinData("Other Entity", entity->GetID());
//		return ExitViaPin("Out");
//	}
//
//	return 0;
//}