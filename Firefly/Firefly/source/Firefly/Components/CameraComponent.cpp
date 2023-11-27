#include "FFpch.h"
#include "CameraComponent.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Utils/Timer.h"
#include "Utils/Math/Random.hpp"
#include "Utils/InputHandler.h"

REGISTER_COMPONENT(CameraComponent);

CameraComponent::CameraComponent() : Component("CameraComponent")
{
	myNearPlane = 10.f;
	myFarPlane = 15000;
	myResolution = { 1920, 1080 };
	myFov = 90;

	myShakeTimer = 0;
	myTimer = 0;
	myInspectorForce = 1;
	myInspectorFrequency = 1;
	myInspectorDuration = 1;
	myBlendInTime = 0;
	myBlendOutTime = 0;

	EditorVariable("Near Plane", Firefly::ParameterType::Float, &myNearPlane);
	EditorVariable("Far Plane", Firefly::ParameterType::Float, &myFarPlane);
	EditorVariable("Resolution", Firefly::ParameterType::Vec2, &myResolution);
	EditorVariable("Field of View", Firefly::ParameterType::Float, &myFov);

	EditorHeader("Camera Shake");
	EditorVariable("Force", Firefly::ParameterType::Float, &myInspectorForce);
	EditorVariable("Frequency", Firefly::ParameterType::Float, &myInspectorFrequency);
	EditorVariable("Duration", Firefly::ParameterType::Float, &myInspectorDuration);
	EditorVariable("Blend In Time", Firefly::ParameterType::Float, &myBlendInTime);
	EditorVariable("Blend Out Time", Firefly::ParameterType::Float, &myBlendOutTime);
	EditorButton("Shake!", [&]() {ActivateCameraShake(myInspectorDuration, myInspectorForce, myInspectorFrequency, myBlendInTime, myBlendOutTime); });
}

void CameraComponent::Initialize()
{
	Component::Initialize();

	Firefly::CameraInfo editorCameraInfo;
	editorCameraInfo.NearPlane = myNearPlane;
	editorCameraInfo.FarPlane = myFarPlane;
	editorCameraInfo.ResolutionX = myResolution.x;
	editorCameraInfo.ResolutionY = myResolution.y;
	editorCameraInfo.Fov = myFov;
	myCamera = Firefly::Camera::Create(editorCameraInfo);
	myCamera->GetTransform().SetTransform(myEntity->GetTransform());

	myPosition = myEntity->GetTransform().GetPosition();
	myRotation = myEntity->GetTransform().GetQuaternion();

	myNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

#ifdef FF_SHIPIT
	EditorPlayEvent ev;
	OnPlayEvent(ev);
#endif
}

void CameraComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppLateUpdateEvent>(BIND_EVENT_FN(CameraComponent::OnLateUpdateEvent));
	dispatcher.Dispatch<EditorPlayEvent>(BIND_EVENT_FN(CameraComponent::OnPlayEvent));
}

bool CameraComponent::OnLateUpdateEvent(Firefly::AppLateUpdateEvent& aEvent)
{
	if (!aEvent.GetIsInPlayMode())
	{
		return false;
	}

	if (myShakeTimer > 0)
	{
		const float deltaTime = Utils::Timer::GetDeltaTime();
		myShakeTimer -= deltaTime; // Time remaining in shake
		myTimer += deltaTime; // Time in shake

		if (myTimer <= myBlendInTime)
		{
			myShakeForce = Utils::Lerp(0.f, myBaseForce, myTimer / myBlendInTime);
		}
		else if (myShakeTimer <= myBlendOutTime)
		{
			myShakeForce = Utils::Lerp(0.f, myBaseForce, myShakeTimer / myBlendOutTime);
		}
		else
		{
			myShakeForce = myBaseForce;
		}
		const float x = myNoise.GetNoise(myTimer, 1.f, 1.f) * myShakeForce;
		const float y = myNoise.GetNoise(1.f, myTimer, 1.f) * myShakeForce;
		const float z = myNoise.GetNoise(1.f, 1.f, myTimer) * myShakeForce;

		myShakeOffsetRotation = Utils::Quaternion::CreateFromEulerAngles(Utils::Vec3(x, y, z));
	}
	else
	{
		myShakeOffsetRotation = Utils::Quaternion();
		myTimer = 0;
	}

	myCamera->GetTransform().SetPosition(myEntity->GetTransform().GetPosition());
	myCamera->GetTransform().SetRotation(myEntity->GetTransform().GetQuaternion() * myShakeOffsetRotation);

	myCamera->SetSize(myResolution.x, myResolution.y);
	myCamera->SetNearPlane(myNearPlane);
	myCamera->SetFarPlane(myFarPlane);
	//myCamera->SetFov(myFov);

	if (aEvent.GetIsInPlayMode())
	{
		myCamera->UpdateFrustum();
		Firefly::Renderer::SubmitActiveCamera(myCamera);
	}

	return false;
}

bool CameraComponent::OnPlayEvent(EditorPlayEvent& aEvent)
{
	Firefly::Renderer::SubmitActiveCamera(myCamera);
	return false;
}

void CameraComponent::ActivateCameraShake(float aDuration, float aShakeAmount, float aFrequency, float aBlendInTime, float aBlendOutTime)
{
	myNoise.SetSeed(Utils::RandomInt(0, 10000));
	myNoise.SetFrequency(aFrequency);
	myShakeTimer = aDuration;
	myShakeForce = aShakeAmount;
	myBaseForce = aShakeAmount;
	myBlendInTime = aBlendInTime;
	myBlendOutTime = aBlendOutTime;
}