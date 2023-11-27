#include "VSNodes_FloatInterpolation.h"
#include "Utils/Math/Lerps.hpp"

void VSNode_FloatLerp::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatLerp::DoOperation()
{
	float a, b, alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float result = Utils::Lerp(a, b, alpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatEaseIn::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_FloatEaseIn::DoOperation()
{
	float a, b, alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float result = Utils::EaseIn(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatEaseOut::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_FloatEaseOut::DoOperation()
{
	float a, b, alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float result = Utils::EaseOut(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatEaseInOut::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_FloatEaseInOut::DoOperation()
{
	float a, b, alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float result = Utils::EaseInOut(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatBounce::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);
}

size_t VSNode_FloatBounce::DoOperation()
{
	float a, b, alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float result = Utils::Bounce(a, b, alpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_FloatParabola::Init()
{
	CreateDataPin<float>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Squish", PinDirection::Input);
	CreateDataPin<float>("Result", PinDirection::Output, true);

	SetPinData<float>("Squish", 2.0f);
}

size_t VSNode_FloatParabola::DoOperation()
{
	float a, b, alpha, squish;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Squish", squish))
	{
		const float result = Utils::Parabola(a, b, alpha, squish);
		SetPinData("Result", result);
	}

	return 0;
}