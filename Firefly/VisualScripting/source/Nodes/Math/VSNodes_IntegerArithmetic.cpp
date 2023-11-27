#include "VSNodes_IntegerArithmetic.h"

void VSNode_IntegerAdd::Init()
{
	CreateDataPin<int>("A", PinDirection::Input);
	CreateDataPin<int>("B", PinDirection::Input);
	CreateDataPin<int>("Out Int", PinDirection::Output, false);
}

size_t VSNode_IntegerAdd::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const int result = a + b;
		SetPinData("Out Int", result);
	}

	return 0;
}

void VSNode_IntegerSub::Init()
{
	CreateDataPin<int>("A", PinDirection::Input);
	CreateDataPin<int>("B", PinDirection::Input);
	CreateDataPin<int>("Result", PinDirection::Output, true);
}

size_t VSNode_IntegerSub::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const int result = a - b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_IntegerMul::Init()
{
	CreateDataPin<int>("A", PinDirection::Input);
	CreateDataPin<int>("B", PinDirection::Input);
	CreateDataPin<int>("Result", PinDirection::Output, true);
}

size_t VSNode_IntegerMul::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		const int result = a * b;
		SetPinData("Result", result);
	}

	return 0;
}

void VSNode_IntegerDiv::Init()
{
	CreateDataPin<int>("A", PinDirection::Input);
	CreateDataPin<int>("B", PinDirection::Input);
	CreateDataPin<int>("Result", PinDirection::Output, true);
}

size_t VSNode_IntegerDiv::DoOperation()
{
	int a, b;

	if (GetPinData("A", a) && GetPinData("B", b))
	{
		if (b == 0)
		{
			return ExitWithError("Division by Zero");
		}

		const int result = a / b;
		SetPinData("Result", result);
	}

	return 0;
}