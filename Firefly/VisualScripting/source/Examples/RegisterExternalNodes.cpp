#include "RegisterExternalNodes.h"

void VSNode_ExampleNode::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<float>("Value", PinDirection::Input);
	CreateDataPin<float>("OutValue", PinDirection::Output);

	//Available functions
	//void CreateDataPin(const std::string& aLabel, PinDirection aDirection, bool hideLabelOnNode = false);
	//void CreateVariablePin(const std::string& aLabel, PinDirection aDirection, bool hideLabelOnNode = false);
	//void CreateExecPin(const std::string& aLabel, PinDirection aDirection, bool hideLabelOnNode = false);

	//size_t ExitViaPin(const std::string& aPinLabel);
	//size_t ExitWithError(const std::string& anErrorMessage);
}

size_t VSNode_ExampleNode::DoOperation()
{
	//Create a variable for all variables that are going to be fetched
	float value;

	//Do an if-check on GetPinData to make sure that nothing went wrong during execution
	//If it succeeds, the variable will have its data
	if (GetPinData("Value", value))
	{
		//Do stuff
		const float returnValue = value * value / value * sinf(value) * 2 * 3;

		//Set the data of all out pins
		SetPinData("OutValue", returnValue);

		//Call return ExitViaPin() with the given output exec pin
		return ExitViaPin("Out");
	}

	//If something went wrong or if we don't have exec, return 0
	return 0;
}