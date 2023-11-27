#include "VSNodes_Vector3Interpolation.h"
#include "Utils/Math/Lerps.hpp"
#include "Utils/Math/Vector3.hpp"

void VSNode_Vector3Lerp::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);
}

size_t VSNode_Vector3Lerp::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const Utils::Vector3f result = Utils::Lerp(a, b, alpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_Vector3EaseIn::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_Vector3EaseIn::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const Utils::Vector3f result = Utils::EaseIn(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_Vector3EaseOut::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_Vector3EaseOut::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const Utils::Vector3f result = Utils::EaseOut(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_Vector3EaseInOut::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Smoothing Power", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);

	SetPinData<float>("Smoothing Power", 2.0f);
}

size_t VSNode_Vector3EaseInOut::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha, power;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Smoothing Power", power))
	{
		const Utils::Vector3f result = Utils::EaseInOut(a, b, alpha, power);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_Vector3Bounce::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);
}

size_t VSNode_Vector3Bounce::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha))
	{
		const Utils::Vector3f result = Utils::Bounce(a, b, alpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_Vector3Parabola::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<float>("Squish", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);

	SetPinData<float>("Squish", 2.0f);
}

size_t VSNode_Vector3Parabola::DoOperation()
{
	Utils::Vector3f a, b;
	float alpha, squish;

	if (GetPinData("A", a) && GetPinData("B", b) && GetPinData("Alpha", alpha) && GetPinData("Squish", squish))
	{
		const Utils::Vector3f result = Utils::Parabola(a, b, alpha, squish);
		SetPinData("Result", result);
	}

	return 0;
}