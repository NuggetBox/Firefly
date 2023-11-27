#include "VSNodes_QuaternionInterpolation.h"
#include "Utils/Math/Lerps.hpp"
#include "Utils/Math/Quaternion.h"

void VSNode_QuaternionLerp::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);
}

size_t VSNode_QuaternionLerp::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float lerpAlpha = Utils::Lerp<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_QuaternionEaseIn::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_QuaternionEaseIn::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseIn<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_QuaternionEaseOut::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_QuaternionEaseOut::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseOut<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_QuaternionEaseInOut::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_QuaternionEaseInOut::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseInOut<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_QuaternionBounce::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);
}

size_t VSNode_QuaternionBounce::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float lerpAlpha = Utils::Bounce<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_QuaternionParabola::Init()
{
	CreateDataPin<Utils::Quaternion>("A", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Squish", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Result", PinDirection::Output, true);

	SetPinData<float>("Squish", 2.0f);
}

size_t VSNode_QuaternionParabola::DoOperation()
{
	Utils::Quaternion a, b;
	float alpha, squish;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Squish", squish))
	{
		const float lerpAlpha = Utils::Parabola<float>(0.0f, 1.0f, alpha);
		const Utils::Quaternion result = Utils::Quaternion::SLerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}