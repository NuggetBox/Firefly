#pragma once
#include <Firefly/Event/Event.h>

#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/Camera/Camera.h"

class SubmitEditorCamEvent;
class EditorStopEvent;

class EditorCamera
{
public:
	EditorCamera() = default;

	void Initialize(Firefly::CameraInfo aCameraInfo);
	void Update();

	void OnEvent(Firefly::Event& aEvent);

	void SetActiveCamera();

	bool OnEditorStopEvent(EditorStopEvent&);

	Ref<Firefly::Camera> GetCamera();

	inline void SetViewportRect(Utils::Vector4<int> aViewportRect) { myViewportRect = aViewportRect; myOverrideViewportRect = true; }

private:
	Ref<Firefly::Camera> myCamera;

	Utils::Vector4<int> myViewportRect;

	float myCameraSpeed = 100.0f;
	const float myRotationSpeedMultiplier = 0.125f;
	const float myShiftMultiplier = 4.0f;
	const float mySpeedIncreasePerSecScroll = 10000.0f;

	bool myUsing;

	bool myWasHidden = false;

	bool myOverrideViewportRect = false;
};
