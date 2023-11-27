#include "VSNodes_FloatArithmetic.h"

#include "Utils/UtilityFunctions.hpp"

void VSNode_FloatAdd::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatAdd::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const float result = a + b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatSub::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatSub::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const float result = a - b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatMul::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatMul::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const float result = a * b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatDiv::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatDiv::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const float result = a / b;
		SetPinData("Result", result);
	}

	return 0;
}

//void VSNode_FloatLerp::Init()
//{
//	CreateDataPin<float>("A", PinDirection::Input);
//	CreateDataPin<float>("B", PinDirection::Input);
//	CreateDataPin<float>("Alpha", PinDirection::Input);
//	CreateDataPin<float>("Result", PinDirection::Output, true);
//}
//
//size_t VSNode_FloatLerp::DoOperation()
//{
//	float a, b, alpha;
//
//	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
//	{
//		const float result = Utils::Lerp(a, b, alpha);
//		SetPinData("Result", result);
//	}
//
//	return 0;
//}