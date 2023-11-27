#include "VSNodes_Casts.h"

void VSNode_FloatToString::Init()
{
	CreateDataPin<float>("In", PinDirection::Input, true);
	CreateDataPin<std::string>("Out", PinDirection::Output, true);
}

size_t VSNode_FloatToString::DoOperation()
{
	float in;

	if (GetPinData("In", in))
	{
		SetPinData("Out", std::to_string(in));
	}

	return 0;
}

void VSNode_IntToFloat::Init()
{
	CreateDataPin<int>("In", PinDirection::Input, true);
	CreateDataPin<float>("Out", PinDirection::Output, true);
}

size_t VSNode_IntToFloat::DoOperation()
{
	int in;

	if (GetPinData("In", in))
	{
		SetPinData("Out", static_cast<float>(in));
	}

	return 0;
}

void VSNode_FloatToInt::Init()
{
	CreateDataPin<float>("In", PinDirection::Input, true);
	CreateDataPin<int>("Out", PinDirection::Output, true);
}

size_t VSNode_FloatToInt::DoOperation()
{
	int in;

	if (GetPinData("In", in))
	{
		SetPinData("Out", static_cast<float>(in));
	}

	return 0;
}

void VSNode_IntToString::Init()
{
	CreateDataPin<int>("In", PinDirection::Input, true);
	CreateDataPin<std::string>("Out", PinDirection::Output, true);
}

size_t VSNode_IntToString::DoOperation()
{
	int in;

	if (GetPinData("In", in))
	{
		SetPinData("Out", std::to_string(in));
	}

	return 0;
}

void VSNode_BoolToString::Init()
{
	CreateDataPin<bool>("In", PinDirection::Input, true);
	CreateDataPin<std::string>("Out", PinDirection::Output, true);
}

size_t VSNode_BoolToString::DoOperation()
{
	bool in;

	if (GetPinData("In", in))
	{
		std::string str = std::to_string(in);
		SetPinData("Out", str);
	}

	return 0;
}