#include "VSNodes_FloatComparison.h"

#include "Utils/UtilityFunctions.hpp"

void VSNode_FloatNearlyEqual::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Tolerance", PinDirection::Input);
	CreateDataPin<bool>("Equal", PinDirection::Output);

	SetPinData<float>("Tolerance", 0.0001f);
}

size_t VSNode_FloatNearlyEqual::DoOperation()
{
	float a, b, tolerance;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Tolerance", tolerance))
	{
		bool equal = Utils::IsAlmostEqual(a, b, tolerance);
		SetPinData("Equal", equal);
	}

	return 0;
}

void VSNode_FloatNotEqual::Init()
{
	CreateDataPin<float>("A", PinDirection::Input, true);
	CreateDataPin<float>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Not Equal", PinDirection::Output, true);
}

size_t VSNode_FloatNotEqual::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool notEqual = a != b;
		SetPinData("Not Equal", notEqual);
	}

	return 0;
}

void VSNode_FloatGreater::Init()
{
	CreateDataPin<float>("A", PinDirection::Input, true);
	CreateDataPin<float>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Greater", PinDirection::Output, true);
}

size_t VSNode_FloatGreater::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool greater = a > b;
		SetPinData("Greater", greater);
	}

	return 0;
}

void VSNode_FloatGreaterEqual::Init()
{
	CreateDataPin<float>("A", PinDirection::Input, true);
	CreateDataPin<float>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Greater Equal", PinDirection::Output, true);
}

size_t VSNode_FloatGreaterEqual::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool greaterEqual = a >= b;
		SetPinData("Greater Equal", greaterEqual);
	}

	return 0;
}

void VSNode_FloatLess::Init()
{
	CreateDataPin<float>("A", PinDirection::Input, true);
	CreateDataPin<float>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Less", PinDirection::Output, true);
}

size_t VSNode_FloatLess::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool less = a < b;
		SetPinData("Less", less);
	}

	return 0;
}

void VSNode_FloatLessEqual::Init()
{
	CreateDataPin<float>("A", PinDirection::Input, true);
	CreateDataPin<float>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Less Equal", PinDirection::Output, true);
}

size_t VSNode_FloatLessEqual::DoOperation()
{
	float a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool lessEqual = a <= b;
		SetPinData("Less Equal", lessEqual);
	}

	return 0;
}