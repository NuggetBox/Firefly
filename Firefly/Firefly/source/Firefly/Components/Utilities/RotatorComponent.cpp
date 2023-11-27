#include "FFpch.h"
#include "RotatorComponent.h"

#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Utils/Timer.h"

REGISTER_COMPONENT(RotatorComponent);

RotatorComponent::RotatorComponent()
	: Component("RotatorComponent")
{
	EditorVariable("Rotation Speed", Firefly::ParameterType::Vec3, &myRotationSpeed);
}

void RotatorComponent::Initialize()
{
	myRigidBody = myEntity->GetComponent<Firefly::RigidbodyComponent>();
}

void RotatorComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& e)
		{
			if (e.GetIsInPlayMode() && myRigidBody.expired())
			{
				myEntity->GetTransform().AddLocalRotation(myRotationSpeed * Utils::Timer::GetDeltaTime());
			}
			return false;
		});
	dispatcher.Dispatch<Firefly::AppFixedUpdateEvent>([&](Firefly::AppFixedUpdateEvent& e)
		{
			if (e.GetIsInPlayMode() && !myRigidBody.expired())
			{
				myRigidBody.lock()->SetRotation(myEntity->GetTransform().GetLocalQuaternion() *
					Utils::Quaternion::CreateFromEulerAngles(myRotationSpeed * Utils::Timer::GetFixedDeltaTime()));
			}
			return false;
		});
}