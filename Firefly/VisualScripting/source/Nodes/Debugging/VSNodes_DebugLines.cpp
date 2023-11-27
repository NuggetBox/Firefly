#include "VSNodes_DebugLines.h"

#include "Firefly/Rendering/Renderer.h"

#include "Utils/Math/Vector3.hpp"

void VSNode_DebugLine::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<Utils::Vector3f>("Start", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("End", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Color", PinDirection::Input);
	CreateDataPin<float>("Duration", PinDirection::Input);
}

size_t VSNode_DebugLine::DoOperation()
{
	Utils::Vector3f start, end, color;
	float duration;

	if (GetPinData("Start", start) && GetPinData("End", end) && GetPinData("Color", color) && GetPinData("Duration", duration))
	{
		Firefly::Renderer::SubmitDebugLine(start, end, Utils::Vec3ToVec4(color));
		return ExitViaPin("Out");
	}

	return Exit();
}

void VSNode_DebugSphere::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<Utils::Vector3f>("Center", PinDirection::Input);
	CreateDataPin<float>("Radius", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Color", PinDirection::Input);
}

size_t VSNode_DebugSphere::DoOperation()
{
	Utils::Vector3f center, color;
	float radius;

	if (GetPinData("Center", center) && GetPinData("Radius", radius) && GetPinData("Color", color))
	{
		Firefly::Renderer::SubmitDebugSphere(center, radius, 25, Utils::Vec3ToVec4(color));
		return ExitViaPin("Out");
	}

	return Exit();
}

void VSNode_DebugCube::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<Utils::Vector3f>("Center", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Size", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Color", PinDirection::Input);
}

size_t VSNode_DebugCube::DoOperation()
{
	Utils::Vector3f center, size, color;

	if (GetPinData("Center", center) && GetPinData("Size", size) && GetPinData("Color", color))
	{
		Firefly::Renderer::SubmitDebugCuboid(center, size, Utils::Vec3ToVec4(color));
		return ExitViaPin("Out");
	}

	return Exit();
}

void VSNode_DebugArrow::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<Utils::Vector3f>("Start", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("End", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("Color", PinDirection::Input);
}

size_t VSNode_DebugArrow::DoOperation()
{
	Utils::Vector3f start, end, color;

	if (GetPinData("Start", start) && GetPinData("End", end) && GetPinData("Color", color))
	{
		Firefly::Renderer::SubmitDebugArrow(start, end, Utils::Vec3ToVec4(color));
		return ExitViaPin("Out");
	}

	return Exit();
}