#include "VSNodes_Vector3.h"

#include "Utils/Math/Vector3.hpp"

void VSNode_MakeVector::Init()
{
	CreateDataPin<float>("X", PinDirection::Input, false);
	CreateDataPin<float>("Y", PinDirection::Input, false);
	CreateDataPin<float>("Z", PinDirection::Input, false);
	CreateDataPin<Utils::Vector3f>("Vector", PinDirection::Output, false);
}

size_t VSNode_MakeVector::DoOperation()
{
	float x, y, z;

	if (GetPinData("X", x) && GetPinData("Y", y) && GetPinData("Z", z))
	{
		const Utils::Vector3f result(x, y, z);
		SetPinData("Vector", result);
	}

	return 0;
}

void VSNode_BreakVector::Init()
{
	CreateDataPin<Utils::Vector3f>("Vector", PinDirection::Input, false);
	CreateDataPin<float>("X", PinDirection::Output, false);
	CreateDataPin<float>("Y", PinDirection::Output, false);
	CreateDataPin<float>("Z", PinDirection::Output, false);
}

size_t VSNode_BreakVector::DoOperation()
{
	Utils::Vector3f inVector;

	if (GetPinData("Vector", inVector))
	{
		SetPinData("X", inVector.x);
		SetPinData("Y", inVector.y);
		SetPinData("Z", inVector.z);
	}

	return 0;
}

void VSNode_VectorLength::Init()
{
	CreateDataPin<Utils::Vector3f>("Vector", PinDirection::Input, false);
	CreateDataPin<float>("Length", PinDirection::Output, false);
}

size_t VSNode_VectorLength::DoOperation()
{
	Utils::Vector3f inVector;

	if (GetPinData("Vector", inVector))
	{
		const float length = inVector.Length();
		SetPinData("Length", length);
	}

	return 0;
}

void VSNode_VectorDistance::Init()
{
	CreateDataPin<Utils::Vector3f>("Vector A", PinDirection::Input, false);
	CreateDataPin<Utils::Vector3f>("Vector B", PinDirection::Input, false);
	CreateDataPin<float>("Distance", PinDirection::Output, false);
}

size_t VSNode_VectorDistance::DoOperation()
{
	Utils::Vector3f vectorA, vectorB;

	if (GetPinData("Vector A", vectorA) && GetPinData("Vector B", vectorB))
	{
		const float distance = (vectorA - vectorB).Length();
		SetPinData("Distance", distance);
	}

	return 0;
}

void VSNode_VectorLerp::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<float>("Alpha", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output);
}

size_t VSNode_VectorLerp::DoOperation()
{
	Utils::Vector3f vectorA, vectorB;
	float alpha;

	if (GetPinData("A", vectorA) && GetPinData("B", vectorB) && GetPinData("Alpha", alpha))
	{
		const Utils::Vector3f result = Utils::Vector3f::Lerp(vectorA, vectorB, alpha);
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_VectorNormalize::Init()
{
	CreateDataPin<Utils::Vector3f>("Vector", PinDirection::Input, true);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output, true);
}

size_t VSNode_VectorNormalize::DoOperation()
{
	Utils::Vector3f vector;

	if (GetPinData("Vector", vector))
	{
		vector.Normalize();
		SetPinData("Result", vector);
	}

	return 0;
}

void VSNode_VectorMultiply::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<float>("B", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output);
}

size_t VSNode_VectorMultiply::DoOperation()
{
	Utils::Vector3f a;
	float b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const Utils::Vector3f result = a * b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_VectorAdd::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output);
}

size_t VSNode_VectorAdd::DoOperation()
{
	Utils::Vector3f vectorA, vectorB;

	if (GetPinData("A", vectorA) && GetPinData("B", vectorB))
	{
		const Utils::Vector3f result = vectorA + vectorB;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_VectorSub::Init()
{
	CreateDataPin<Utils::Vector3f>("A", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("B", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Result", PinDirection::Output);
}

size_t VSNode_VectorSub::DoOperation()
{
	Utils::Vector3f vectorA, vectorB;

	if (GetPinData("A", vectorA) && GetPinData("B", vectorB))
	{
		const Utils::Vector3f result = vectorA - vectorB;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_VectorDot::Init()
{
	CreateDataPin<Utils::Vector3f>("Vector A", PinDirection::Input, false);
	CreateDataPin<Utils::Vector3f>("Vector B", PinDirection::Input, false);
	CreateDataPin<float>("Dot Product", PinDirection::Output, false);
}

size_t VSNode_VectorDot::DoOperation()
{
	Utils::Vector3f a, b;

	if (GetPinData("Vector A", a) && GetPinData("Vector B", b))
	{
		const float result = a.Dot(b);
		SetPinData("Dot Product", result);
	}

	return 0;
}