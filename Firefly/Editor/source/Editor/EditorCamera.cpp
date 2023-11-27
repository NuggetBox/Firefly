#include "EditorPch.h"
#include "EditorCamera.h"

#include "EditorLayer.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Renderer.h"

#include "Utils/InputHandler.h"
#include "Utils/Timer.h"

void EditorCamera::Initialize(Firefly::CameraInfo aCameraInfo)
{
	myCamera = Firefly::Camera::Create(aCameraInfo);
	myCamera->GetTransform().SetLocalPosition(-200, 200, -200);
	myCamera->GetTransform().SetLocalRotation(45, 45, 0);
	myCamera->UpdateFrustum();
	myCamera->SetFarPlane(100000000);
	Firefly::Renderer::SubmitActiveCamera(myCamera);
}

void EditorCamera::Update()
{
	Utils::Transform& transform = myCamera->GetTransform();

	if (Utils::InputHandler::GetRightClickHeld() && myUsing)
	{
		const int scrollDelta = Utils::InputHandler::GetScrollWheelDelta();
		const Utils::Vector2f mouseDelta =
		{
			static_cast<float>(Utils::InputHandler::GetUncappedMouseDelta().x),
			static_cast<float>(Utils::InputHandler::GetUncappedMouseDelta().y)
		};

		float cameraSpeed = myCameraSpeed;

		if (Utils::InputHandler::GetKeyHeld(VK_SHIFT))
		{
			cameraSpeed *= myShiftMultiplier;
		}
		const float step = Utils::Timer::GetUnscaledDeltaTime() * cameraSpeed;

		if (Utils::InputHandler::GetKeyHeld('W'))
		{
			transform.AddPosition(myCamera->GetTransform().GetForward() * step);
		}
		if (Utils::InputHandler::GetKeyHeld('A'))
		{
			transform.AddPosition(myCamera->GetTransform().GetLeft() * step);
		}
		if (Utils::InputHandler::GetKeyHeld('S'))
		{
			transform.AddPosition(myCamera->GetTransform().GetBackward() * step);
		}
		if (Utils::InputHandler::GetKeyHeld('D'))
		{
			transform.AddPosition(myCamera->GetTransform().GetRight() * step);
		}

		if (Utils::InputHandler::GetKeyHeld('Q'))
		{
			transform.AddPosition(Utils::Vector3f(0, -1, 0) * step);
		}
		if (Utils::InputHandler::GetKeyHeld('E'))
		{
			transform.AddPosition(Utils::Vector3f(0, 1, 0) * step);
		}

		//Increase speed on scroll up
		if (scrollDelta > 0)
		{
			myCameraSpeed += Utils::Timer::GetUnscaledDeltaTime() * mySpeedIncreasePerSecScroll;
		}
		//Decrease on scroll down
		else if (scrollDelta < 0)
		{
			myCameraSpeed -= Utils::Timer::GetUnscaledDeltaTime() * mySpeedIncreasePerSecScroll;

			if (myCameraSpeed < 0)
			{
				myCameraSpeed = 0.0f;
			}
		}

		transform.SetRotation(Utils::Quaternion::CreateFromEulerAngles(0, mouseDelta.x * myRotationSpeedMultiplier, 0) * transform.GetQuaternion());

		//Add X rotation if it doesn't roll you over
		auto xRotationDelta = mouseDelta.y * myRotationSpeedMultiplier;
		if (transform.GetRotation().x + xRotationDelta > -89.5f && transform.GetRotation().x + xRotationDelta < 89.5f)
		{
			transform.SetRotation(transform.GetQuaternion() * Utils::Quaternion::CreateFromEulerAngles(xRotationDelta, 0, 0));
		}

		//Tilt hehe
		//if (Utils::InputHandler::GetKeyHeld('R'))
		//{
		//	myCamera->GetTransform().SetRotation(myCamera->GetTransform().GetRotation() * Utils::Quaternion::CreateFromEulerAngles(0, 0, Utils::Timer::GetDeltaTime() * 50));
		//}
		//if (Utils::InputHandler::GetKeyHeld('T'))
		//{
		//	myCamera->GetTransform().SetRotation(myCamera->GetTransform().GetRotation() * Utils::Quaternion::CreateFromEulerAngles(0, 0, -Utils::Timer::GetDeltaTime() * 50));
		//}

		myCamera->UpdateFrustum();
	}

	if (Utils::InputHandler::GetRightClickDown())
	{
		if (ImGui::GetMousePos().x > ImGui::GetWindowPos().x &&
			ImGui::GetMousePos().x < ImGui::GetWindowPos().x + ImGui::GetWindowWidth() &&
			ImGui::GetMousePos().y > ImGui::GetWindowPos().y &&
			ImGui::GetMousePos().y < ImGui::GetWindowPos().y + ImGui::GetWindowHeight())
		{
			myWasHidden = Utils::InputHandler::IsMouseHidden();

			Utils::InputHandler::HideMouseCursor();
			if (myOverrideViewportRect)
			{
				Utils::InputHandler::CaptureMouse(myViewportRect.x, myViewportRect.y, myViewportRect.z, myViewportRect.w);
			}
			else
			{
				Utils::InputHandler::CaptureMouse();
			}
			myUsing = true;
		}
	}

	if (Utils::InputHandler::GetRightClickUp())
	{
		if (!myWasHidden)
		{
			Utils::InputHandler::ShowMouseCursor();
			Utils::InputHandler::ReleaseMouse();
		}
		myUsing = false;
	}
	SetActiveCamera();
}

void EditorCamera::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<EditorStopEvent>(BIND_EVENT_FN(EditorCamera::OnEditorStopEvent));
}

void EditorCamera::SetActiveCamera()
{
	Firefly::Renderer::SubmitActiveCamera(myCamera);
}

bool EditorCamera::OnEditorStopEvent(EditorStopEvent&)
{
	Utils::InputHandler::ShowMouseCursor();
	Utils::InputHandler::ReleaseMouse();
	SetActiveCamera();
	return false;
}

Ref<Firefly::Camera> EditorCamera::GetCamera()
{
	return myCamera;
}
