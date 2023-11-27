#include "VSNode_Append.h"

void VSNode_Append::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<std::string>("String1", PinDirection::Input);
	CreateDataPin<std::string>("String2", PinDirection::Input);
	CreateDataPin<std::string>("OutValue", PinDirection::Output);
}

size_t VSNode_Append::DoOperation()
{
	std::string Str1;
	std::string Str2;

	GetPinData("String1", Str1);
	GetPinData("String2", Str2);
	Str1.append(Str2);
	SetPinData("OutValue", Str1);
	return ExitViaPin("Out");
}