#include "VSNode_CameraControl.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include "Firefly/Components/CameraComponent.h"

void VSNode_CameraShake::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);
	CreateDataPin<uint64_t>("Camera", PinDirection::Input);
	CreateDataPin<float>("Force", PinDirection::Input);
	CreateDataPin<float>("Frequency", PinDirection::Input);
	CreateDataPin<float>("Duration", PinDirection::Input);
	CreateDataPin<float>("Blend In", PinDirection::Input);
	CreateDataPin<float>("Blend Out", PinDirection::Input);
}

size_t VSNode_CameraShake::DoOperation()
{
	uint64_t entityID;
	auto entity = Firefly::GetEntityWithComponent<CameraComponent>();
	if (!entity.expired())
	{
		auto camComp = entity.lock()->GetComponent<CameraComponent>();
		if (!camComp.expired())
		{
			float force = 1;
			float frequency = 1;
			float duration = 1;
			float blendIn = 0;
			float blendOut = 0;

			GetPinData("Force", force);
			GetPinData("Frequency", frequency);
			GetPinData("Duration", duration);
			GetPinData("Blend In", blendIn);
			GetPinData("Blend Out", blendOut);

			camComp.lock()->ActivateCameraShake(duration, force, frequency, blendIn, blendOut);
		}

	}
	return ExitViaPin("Out");
}
