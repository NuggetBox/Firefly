#include "VSNodes_Trigonometry.h"
#include "Utils/Math/MathDefines.h"

void VSNode_Sin::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_Sin::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(sinf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}

void VSNode_Cos::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_Cos::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(cosf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}

void VSNode_Tan::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_Tan::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(tanf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}

void VSNode_ASin::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_ASin::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(asinf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}

void VSNode_ACos::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_ACos::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(acosf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}

void VSNode_Atan::Init()
{
	CreateDataPin<float>("Angle", PinDirection::Input, true);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_Atan::DoOperation()
{
	float angle;

	if (GetPinData("Angle", angle))
	{
		const float result = RADTODEG(atanf(DEGTORAD(angle)));
		SetPinData("Result", result);
		return 1;
	}

	return 0;
}