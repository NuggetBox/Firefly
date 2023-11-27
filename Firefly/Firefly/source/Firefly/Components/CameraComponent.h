#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "Firefly/Utilities/FastNoiseLite.h"

namespace Firefly
{
	class AppUpdateEvent;
}

class CameraComponent : public Firefly::Component
{
public:
	CameraComponent();
	~CameraComponent() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnLateUpdateEvent(Firefly::AppLateUpdateEvent& aEvent);
	bool OnPlayEvent(EditorPlayEvent& aEvent);

	static std::string GetFactoryName() { return "CameraComponent"; }
	static Ref<Component> Create() { return CreateRef<CameraComponent>(); }

	void SetFOV(const float& aValue) { myCamera->SetFov(aValue); }
	float GetFOV() { return myCamera->GetFov(); }

	void SendPosition(const Utils::Vec3& aPosition) { myPosition = aPosition; }
	void SendRotation(const Utils::Quaternion& aRotation) { myRotation = aRotation; }

	void ActivateCameraShake(float aDuration = 0.4f, float aShakeAmount = 10, float aFrequency = 8, float aBlendInTime = 0, float aBlendOutTime = 0);

private:
	Ref<Firefly::Camera> myCamera;
	float myNearPlane;
	float myFarPlane;
	float myFov;

	Utils::Vec3 myShakeOffsetPosition;
	Utils::Quaternion myShakeOffsetRotation;

	float myShakeTimer;
	float myTimer;
	float myShakeForce;
	float myBaseForce;
	float myBlendInTime;
	float myBlendOutTime;

	// Just for testing!
	float myInspectorForce = 0;
	float myInspectorFrequency = 0;
	float myInspectorDuration = 0;

	FastNoiseLite myNoise;


	Utils::Vector2f myResolution;
	Utils::Vector3f myPosition;
	Utils::Quaternion myRotation;
};
