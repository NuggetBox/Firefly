#include "VSNodes_Quaternion.h"

#include "Utils/Math/Quaternion.h"

void VSNode_MakeQuaternion::Init()
{
	CreateDataPin<float>("X", PinDirection::Input);
	CreateDataPin<float>("Y", PinDirection::Input);
	CreateDataPin<float>("Z", PinDirection::Input);
	CreateDataPin<Utils::Quaternion>("Quaternion", PinDirection::Output);
}

size_t VSNode_MakeQuaternion::DoOperation()
{
	float x, y, z;

	if (GetPinData("X", x) && GetPinData("Y", y) && GetPinData("Z", z))
	{
		const Utils::Quaternion outQuat = Utils::Quaternion::CreateFromEulerAngles(x, y, z);
		SetPinData("Quaternion", outQuat);
	}

	return 0;
}

void VSNode_BreakQuaternion::Init()
{
	CreateDataPin<Utils::Quaternion>("Quaternion", PinDirection::Input);
	CreateDataPin<float>("X", PinDirection::Output);
	CreateDataPin<float>("Y", PinDirection::Output);
	CreateDataPin<float>("Z", PinDirection::Output);
}

size_t VSNode_BreakQuaternion::DoOperation()
{
	Utils::Quaternion inQuat;

	if (GetPinData("Quaternion", inQuat))
	{
		const Utils::Vector3f eulerAngles = inQuat.GetEulerAngles();
		SetPinData("X", eulerAngles.x);
		SetPinData("Y", eulerAngles.y);
		SetPinData("Z", eulerAngles.z);
	}

	return 0;
}