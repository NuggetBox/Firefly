#include "VSNodes_TransformInterpolation.h"
#include "Utils/Math/Lerps.hpp"
#include "Utils/Math/Transform.h"
#include "Utils/Math/Vector3.hpp"

void VSNode_TransformLerp::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);
}

size_t VSNode_TransformLerp::DoOperation()
{
	Utils::Transform a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float lerpAlpha = Utils::Lerp<float>(0.0f, 1.0f, alpha);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_TransformEaseIn::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_TransformEaseIn::DoOperation()
{
	Utils::Transform a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseIn<float>(0.0f, 1.0f, alpha, power);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_TransformEaseOut::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_TransformEaseOut::DoOperation()
{
	Utils::Transform a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseOut<float>(0.0f, 1.0f, alpha, power);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_TransformEaseInOut::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_TransformEaseInOut::DoOperation()
{
	Utils::Transform a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const float lerpAlpha = Utils::EaseInOut<float>(0.0f, 1.0f, alpha, power);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_TransformBounce::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);
}

size_t VSNode_TransformBounce::DoOperation()
{
	Utils::Transform a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const float lerpAlpha = Utils::Bounce<float>(0.0f, 1.0f, alpha);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_TransformParabola::Init()
{
	CreateDataPin<Utils::Transform>("A", PinDirection::Input);
	CreateDataPin<Utils::Transform>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Squish", PinDirection::Input);
	CreateDataPin<Utils::Transform>("Result", PinDirection::Output, true);

	SetPinData<float>("Squish", 2.0f);
}

size_t VSNode_TransformParabola::DoOperation()
{
	Utils::Transform a, b;
	float alpha, squish;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Squish", squish))
	{
		const float lerpAlpha = Utils::Parabola<float>(0.0f, 1.0f, alpha, squish);
		const Utils::Transform result = Utils::Transform::Lerp(a, b, lerpAlpha);
		SetPinData("Result", result);
	}

	return 0;
}