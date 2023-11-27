#include "VSNodes_IntegerComparison.h"

void VSNode_IntegerEqual::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Equal", PinDirection::Output, true);
}

size_t VSNode_IntegerEqual::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool equal = a == b;
		SetPinData("Equal", equal);
	}

	return 0;
}

void VSNode_IntegerNotEqual::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Not Equal", PinDirection::Output, true);
}

size_t VSNode_IntegerNotEqual::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool notEqual = a != b;
		SetPinData("Not Equal", notEqual);
	}

	return 0;
}

void VSNode_IntegerGreater::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Greater", PinDirection::Output, true);
}

size_t VSNode_IntegerGreater::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool greater = a > b;
		SetPinData("Greater", greater);
	}

	return 0;
}

void VSNode_IntegerGreaterEqual::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Greater Equal", PinDirection::Output, true);
}

size_t VSNode_IntegerGreaterEqual::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool greaterEqual = a >= b;
		SetPinData("Greater Equal", greaterEqual);
	}

	return 0;
}

void VSNode_IntegerLess::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Less", PinDirection::Output, true);
}

size_t VSNode_IntegerLess::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool less = a < b;
		SetPinData("Less", less);
	}

	return 0;
}

void VSNode_IntegerLessEqual::Init()
{
	CreateDataPin<int>("A", PinDirection::Input, true);
	CreateDataPin<int>("B", PinDirection::Input, true);
	CreateDataPin<bool>("Less Equal", PinDirection::Output, true);
}

size_t VSNode_IntegerLessEqual::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		bool lessEqual = a <= b;
		SetPinData("Less Equal", lessEqual);
	}

	return 0;
}