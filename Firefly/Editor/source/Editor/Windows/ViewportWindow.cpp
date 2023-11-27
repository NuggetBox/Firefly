#include "EditorPch.h"
#include "ViewportWindow.h"

#include <Editor/Utilities/EditorUtils.h>
#include <Firefly/Components/ParticleSystem/ParticleEmitterComponent.h>

#include "Utils/InputHandler.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"

#include "Editor/EditorLayer.h"
#include "Editor/Event/EditorOnlyEvents.h"
#include "Editor/UndoSystem/Commands/EntityCommands/EntityDeselectAllCommand.h"
#include "Editor/UndoSystem/Commands/EntityCommands/EntitySelectCommand.h"

#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/SceneManager.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/Texture/Texture2D.h"

#include "Firefly/Rendering/Renderer.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/SceneEvents.h"
#include "Utils/InputHandler.h"
#include "Editor/UndoSystem/Commands/SceneCommands/EntityDuplicateCommand.h"
#include "Editor/UndoSystem/Commands/EntityCommands/TransformCommand.h"
#include "Editor/UndoSystem/Commands/SceneCommands/EntityCreateCommand.h"
#include "Editor/UndoSystem/Commands/SceneCommands/PrefabCreateCommand.h"

#include "Editor/Utilities/ImGuiUtils.h"

#include "Editor/Windows/WindowRegistry.h"
#include "Firefly/Components/Mesh/AnimatedMeshComponent.h"
#include "Utils/Timer.h"
#include "Utils/Math/Intersection.hpp"
#include "Firefly/Components/Mesh/MeshComponent.h"

#include "imgui/imgui_internal.h"

using namespace Firefly;

REGISTER_WINDOW(ViewportWindow);

ViewportWindow::ViewportWindow() : EditorWindow("Viewport")
{
	myEditorCamera = std::make_shared<EditorCamera>();

	Firefly::CameraInfo editorCameraInfo;
	editorCameraInfo.NearPlane = 10.f;
	editorCameraInfo.FarPlane = 100000;
	editorCameraInfo.ResolutionX = 1280;
	editorCameraInfo.ResolutionY = 720;
	editorCameraInfo.Fov = 90;

	mySettingsChildLerpedWidth = 30.f;

	myEditorCamera->Initialize(editorCameraInfo);

	mySnapSize = { 50, 50 ,50 };
	myRotSnapSize = { 15,15,15 };
	myScaleSnapSize = { 0.5f,0.5,0.5 };

	myGridEnabled = true;
	myDebugLinesEnabled = true;
	myGuizmoEnabled = true;
	mySixteenNineEnabled = true;
	myOutlineEnabled = true;
	mySettingsCollapsed = true;
	myNoWindowPadding = true;

	mySettingsIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_settings.dds");
	myGridEnabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_grid.dds");
	myGridDisabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_grid_disabled.dds");
	myDebugLinesEnabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_debuglines.dds");
	myDebugLinesDisabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_debuglines_disabled.dds");
	myGuizmoEnabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_gizmo.dds");
	myGuizmoDisabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_gizmo_disabled.dds");
	mySixteenNineEnabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_sixteen_nine.dds");
	mySixteenNineDisabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_sixteen_nine_disabled.dds");
	myOutlineEnabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_outline.dds");
	myOutlineDisabledIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_outline_disabled.dds");
	myBackIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_back.dds");
	myPlayIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_play.dds");
	myStopIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_stop.dds");

	myWindowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void ViewportWindow::OnImGui()
{
	DrawPlayButtonWindow();

	myEditorCamera->Update();
	if (!Application::Get().GetIsInPlayMode())
		myEditorCamera->SetActiveCamera();

	CalcActualPosAndSize();

	if (Renderer::GetActiveCamera())
	{
		Renderer::GetActiveCamera()->SetSize(myActualWindowSize.x, myActualWindowSize.y);
	}

	Utils::InputHandler::SetMouseRelativePos(myActualMousePos.x, myActualMousePos.y);
	Utils::InputHandler::SetWindowSize({ myActualWindowSize.x, myActualWindowSize.y });

	if (Utils::Abs(static_cast<float>(Renderer::GetFrameBuffer()->GetSpecs().Width) - myActualWindowSize.x) > 1.0f
		|| Utils::Abs(static_cast<float>(Renderer::GetFrameBuffer()->GetSpecs().Height) - myActualWindowSize.y) > 1.0f)
	{
		auto newSize = Utils::Vector2<uint32_t>(static_cast<uint32_t>(myActualWindowSize.x), static_cast<uint32_t>(myActualWindowSize.y));
		Renderer::GetFrameBuffer()->Resize(newSize);
	}

	ImGui::SetCursorScreenPos(myActualWindowPos);
	ImGui::Image(Renderer::GetFrameBuffer()->GetColorAttachment(0).Get(), myActualWindowSize);
	if (globalRendererStats.VisablePass != 0)
	{
		std::string str = "current pass: " + GetNameOfPassID(globalRendererStats.VisablePass);
		auto textWidth = ImGui::CalcTextSize(str.c_str()).x / 2;
		ImGui::SetCursorPos({ (ImGui::GetWindowSize().x / 2) - textWidth, 0 });
		ImGui::Text(str.c_str());
	}

	//If we are in editor mode and there are selected entities, update the guizmo
	if (!Application::Get().GetIsInPlayMode())
	{
		if (!myIsSquareSelecting && !myIsUsingGuizmo && myMousePickingEnabled)
		{
			UpdateMousePicking();
		}

		if (myGuizmoEnabled)
		{
			UpdateGuizmo();
		}

		UpdateSquareSelection();
	}

	if (IsFocused())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_G, false))
		{
			if (myGridEnabled || myDebugLinesEnabled)
			{
				myGridEnabled = false;
				myDebugLinesEnabled = false;
			}
			else
			{
				myGridEnabled = true;
				myDebugLinesEnabled = true;
			}

			ApplyDebugChanges();
		}
	}

	if (EditorLayer::GetEditingPrefab())
	{
		ImGui::SetCursorPos(ImGui::GetCursorStartPos());
		if (ImGui::ImageButton(myBackIcon->GetSRV().Get(), { 64,32 },
			ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 0.3f)))
		{
			PrefabEditorBackButtonPressedEvent ev;
			Firefly::Application::Get().OnEvent(ev);
		}
	}

	if (EditorUtils::AcceptAllDragDrops(myEditorCamera->GetCamera()->GetTransform().GetPosition() + myEditorCamera->GetCamera()->GetTransform().GetForward() * myDragDropSpawnDistance))
	{
		SetFocused();
	}
}

void ViewportWindow::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<EditorPlayEvent>([&](EditorPlayEvent&)
		{
			myMousePickingEnabled = false;
			myGridEnabled = false;
			myGuizmoEnabled = false;
			mySixteenNineEnabled = true;
			ApplyDebugChanges();

			return false;
		});

	dispatcher.Dispatch<EditorStopEvent>([&](EditorStopEvent&)
		{
			myMousePickingEnabled = true;
			myGridEnabled = true;
			myGuizmoEnabled = true;
			ApplyDebugChanges();

			return false;
		});

	myEditorCamera->OnEvent(aEvent);
}

void ViewportWindow::EnableMousePicking(bool aEnabled)
{
	myMousePickingEnabled = aEnabled;
}

void ViewportWindow::DrawPlayButtonWindow()
{
	int buttonCount = 8;
	if (mySettingsCollapsed)
	{
		buttonCount = 2;
	}
	constexpr float buttonSize = 40.0f;
	auto& style = ImGui::GetStyle();
	const auto childWidth = (buttonSize + style.ItemSpacing.x) * buttonCount - buttonSize / 2;
	mySettingsChildLerpedWidth = Utils::Lerp(mySettingsChildLerpedWidth, childWidth, Utils::Timer::GetDeltaTime() * 20.f);
	ImGui::SetCursorPos({ ImGui::GetWindowWidth() - mySettingsChildLerpedWidth,0 });
	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.1f,0.1f,0.1f,0.3f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0,0 });
	ImGui::BeginChild("PlayButton##", { mySettingsChildLerpedWidth,  buttonSize });
	ImGui::PopStyleColor();
	ImGuiID id = ImGui::GetWindowDockID();
	std::string collapseLable = mySettingsCollapsed ? "<" : ">";
	collapseLable += "##CollapseSettingsButton";
	if (ImGui::Button(collapseLable.c_str(), { buttonSize / 2, buttonSize }))
	{
		mySettingsCollapsed = !mySettingsCollapsed;
	}
	ImGui::SameLine();
	auto playButtonIcon = myPlayIcon;
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		playButtonIcon = myStopIcon;
	}
	bool playButtonPressed = ImGui::ImageButton(
		playButtonIcon->GetSRV().Get(),
		ImVec2(buttonSize, buttonSize));
	Application::Get().GetIsInPlayMode() ? ImGuiUtils::ToolTip("Exit Play Mode") : ImGuiUtils::ToolTip("Enter Play Mode");

	if (!mySettingsCollapsed)
	{

		ImGui::SameLine();

		auto gridIcon = myGridEnabled ? myGridEnabledIcon : myGridDisabledIcon;
		if (ImGui::ImageButton(gridIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize)))
		{
			myGridEnabled = !myGridEnabled;
			Renderer::SetGridActive(myGridEnabled);
		}
		myGridEnabled ? ImGuiUtils::ToolTip("Disable Grid") : ImGuiUtils::ToolTip("Enable Grid");
		ImGui::SameLine();

		auto debuglinesIcon = myDebugLinesEnabled ? myDebugLinesEnabledIcon : myDebugLinesDisabledIcon;
		if (ImGui::ImageButton(debuglinesIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize)))
		{
			myDebugLinesEnabled = !myDebugLinesEnabled;
			Renderer::SetDebugLinesActive(myDebugLinesEnabled);
		}
		myDebugLinesEnabled ? ImGuiUtils::ToolTip("Disable Debug Lines") : ImGuiUtils::ToolTip("Enable Debug Lines");
		ImGui::SameLine();

		auto guizmoIcon = myGuizmoEnabled ? myGuizmoEnabledIcon : myGuizmoDisabledIcon;
		if (ImGui::ImageButton(guizmoIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize)))
		{
			myGuizmoEnabled = !myGuizmoEnabled;
		}
		myGuizmoEnabled ? ImGuiUtils::ToolTip("Disable Transform Gizmo") : ImGuiUtils::ToolTip("Enable Transform Gizmo");
		ImGui::SameLine();

		auto outlineIcon = myOutlineEnabled ? myOutlineEnabledIcon : myOutlineDisabledIcon;
		if (ImGui::ImageButton(outlineIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize)))
		{
			myOutlineEnabled = !myOutlineEnabled;
		}
		myOutlineEnabled ? ImGuiUtils::ToolTip("Disable Selected Object Outlining") : ImGuiUtils::ToolTip("Enable Selected Object Outlining");
		ImGui::SameLine();

		auto sixteenNineIcon = mySixteenNineEnabled ? mySixteenNineEnabledIcon : mySixteenNineDisabledIcon;
		if (ImGui::ImageButton(sixteenNineIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize)))
		{
			mySixteenNineEnabled = !mySixteenNineEnabled;
		}
		mySixteenNineEnabled ? ImGuiUtils::ToolTip("Disable 16:9 aspect ratio") : ImGuiUtils::ToolTip("Enable 16:9 aspect ratio");


		ApplyDebugChanges();

		ImGui::SameLine();
		ImGui::ImageButton(mySettingsIcon->GetSRV().Get(), ImVec2(buttonSize, buttonSize));
		if (ImGui::BeginPopupContextItem(nullptr, ImGuiMouseButton_Left))
		{
			ImGui::BeginPopup("ViewportSettings");
			ImGui::DragFloat3("SnapSize##ViewportSettings", &mySnapSize.x);
			ImGui::DragFloat3("SnapRotSize##ViewportSettings", &myRotSnapSize.x);
			ImGui::DragFloat3("SnapScaleSize##ViewportSettings", &myScaleSnapSize.x);

			ImGui::EndPopup();
		}
		ImGuiUtils::ToolTip("Snapping Settings");
	}

	if (playButtonPressed)
	{
		//Enter player mode, ourPlayScene exists in EditorLayer
		EditorLayer::TogglePlayMode();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

bool ViewportWindow::InputGuizmoMode(ImGuizmo::OPERATION& aOutOperation, ImGuizmo::MODE& aOutMode, Utils::Vector3f& aOutSnapping)
{
	bool changed = false;

	if (!EditorLayer::IsCoreWindowsFocused())
	{
		return true;
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) //TODO: Temp switch local/world on middle mouse
	{
		myGuizmoWorldEnabled = !myGuizmoWorldEnabled;
		changed = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_W, false))
	{
		aOutOperation = ImGuizmo::OPERATION::TRANSLATE;
		changed = true;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_E, false))
	{
		aOutOperation = ImGuizmo::OPERATION::ROTATE;
		changed = true;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_R, false))
	{
		aOutOperation = ImGuizmo::OPERATION::SCALE;
		changed = true;
	}

	if (myGuizmoWorldEnabled && aOutOperation != ImGuizmo::OPERATION::SCALE)
	{
		aOutMode = ImGuizmo::MODE::WORLD;
	}
	else
	{
		aOutMode = ImGuizmo::MODE::LOCAL;
	}

	if (Utils::InputHandler::GetKeyHeld(VK_CONTROL))
	{
		if (aOutOperation & ImGuizmo::OPERATION::TRANSLATE)
		{
			aOutSnapping = mySnapSize;
		}
		else if (aOutOperation & ImGuizmo::OPERATION::ROTATE)
		{
			aOutSnapping = myRotSnapSize;
		}
		else if (aOutOperation & ImGuizmo::OPERATION::SCALE)
		{
			aOutSnapping = myScaleSnapSize;
		}
	}

	return changed;
}

void ViewportWindow::OnBeginGuizmo(const ImGuizmo::OPERATION& aOperation)
{
	SetFocused();

	if (!myIsUsingGuizmo)
	{
		const auto& selectedEntities = EditorLayer::GetSelectedEntities();

		for (const auto& entity : selectedEntities)
		{
			myInitialGuizmoTransforms.emplace_back(
				entity.lock()->GetTransform().GetLocalPosition(),
				entity.lock()->GetTransform().GetLocalQuaternion(),
				entity.lock()->GetTransform().GetLocalScale());
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && (aOperation & ImGuizmo::TRANSLATE || aOperation & ImGuizmo::ROTATE))
		{
			std::vector<Ptr<Entity>> entitiesToDuplicate;

			for (auto& entity : selectedEntities)
			{
				if (!EditorLayer::HasSelectedParentRecursive(entity))
				{
					entitiesToDuplicate.push_back(entity);
				}
			}

			EditorLayer::DuplicateEntities(entitiesToDuplicate);
		}
	}
}

void ViewportWindow::OnEndGuizmo()
{
	if (myIsUsingGuizmo)
	{
		const auto& selectedEntities = EditorLayer::GetSelectedEntities();

		EditorLayer::BeginEntityUndoSeries();

		for (int i = 0; i < selectedEntities.size(); ++i)
		{
			Utils::Vector3f	positionDiff = selectedEntities[i].lock()->GetTransform().GetLocalPosition() - myInitialGuizmoTransforms[i].GetPosition();
			Utils::Quaternion rotationDiff = myInitialGuizmoTransforms[i].GetQuaternion().GetInverse() * selectedEntities[i].lock()->GetTransform().GetLocalQuaternion();
			Utils::Vector3f	scaleDiff = selectedEntities[i].lock()->GetTransform().GetLocalScale() / myInitialGuizmoTransforms[i].GetScale();

			const auto transformCommand = CreateRef<TransformCommand>(selectedEntities[i], positionDiff, rotationDiff, scaleDiff);
			transformCommand->UpdateModification();
			EditorLayer::AddEntityUndo(transformCommand);
		}

		EditorLayer::EndEntityUndoSeries();

		myInitialGuizmoTransforms.clear();
	}
}

void ViewportWindow::UpdateGuizmo()
{
	const auto& selectedEntities = EditorLayer::GetSelectedEntities();

	if (selectedEntities.empty())
	{
		myIsUsingGuizmo = false;
		//ImGuizmo::Enable(false);
		return;
	}

	ImGuizmo::AllowAxisFlip(false);

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		myIsUsingGuizmo = false;
	}
	else
	{
		ImGuizmo::Enable(true);

		static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		static ImGuizmo::MODE mode = ImGuizmo::WORLD;
		Utils::Vector3f snapping;

		if (InputGuizmoMode(operation, mode, snapping))
		{
			myIsUsingGuizmo = false;
			//ImGuizmo::BENNEFORCEOFF();
			return;
		}

		const auto& camera = Firefly::Renderer::GetActiveCamera();
		float viewMatrix[16] = { 0 };
		memcpy(viewMatrix, &camera->GetViewMatrix(), sizeof(float) * 16);
		float projectionMatrix[16] = { 0 };
		memcpy(projectionMatrix, &camera->GetProjectionMatrixPerspective(), sizeof(float) * 16);

		const auto& lastSelectedEntity = EditorLayer::GetLastSelectedEntity();
		Utils::Transform lastSelectedTransform(lastSelectedEntity.lock()->GetTransform());
		Utils::Matrix4f lastSelectedMatrix = lastSelectedTransform.GetMatrix();
		const Utils::Matrix4f originMatrix = lastSelectedMatrix;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(myActualWindowPos.x, myActualWindowPos.y, myActualWindowSize.x, myActualWindowSize.y);

		ImGuizmo::Manipulate(&viewMatrix[0], &projectionMatrix[0], operation, mode, reinterpret_cast<float*>(&lastSelectedMatrix), nullptr, &snapping.x);

		if (ImGuizmo::IsUsing())
		{
			if (!myIsUsingGuizmo)
			{
				OnBeginGuizmo(operation);
			}
			else
			{
				if (selectedEntities.size() == 1)
				{
					HandleSingleObjectGuizmo(lastSelectedMatrix, operation);
				}
				else
				{
					HandleMultipleObjectGuizmo(originMatrix, lastSelectedMatrix, operation);
				}
			}
		}
		else if (myIsUsingGuizmo)
		{
			OnEndGuizmo();
		}

		//save if imguizmo was active last frame
		myIsUsingGuizmo = ImGuizmo::IsUsing();
	}
}

void ViewportWindow::HandleSingleObjectGuizmo(const Utils::Matrix4f& aModifiedMatrix, const ImGuizmo::OPERATION& aOperation)
{
	const Ptr<Firefly::Entity>& entity = EditorLayer::GetLastSelectedEntity();
	Utils::Matrix4f	finalMatrix = aModifiedMatrix;

	//Take the modified matrix from world space into parents space
	if (entity.lock()->HasParent())
	{
		finalMatrix = finalMatrix * Utils::Matrix4f::GetInverse(entity.lock()->GetParent().lock()->GetTransform().GetMatrix());
	}

	//Decompose the modified matrix, now in parent space, and find out the new local transformations
	Utils::Vector3f modifiedPosition;
	Utils::Quaternion modifiedRotation;
	Utils::Vector3f modifiedScale;
	Utils::Matrix4f::Decompose(finalMatrix, modifiedPosition, modifiedRotation, modifiedScale);

	//Set the local transformations of the single selected entity, switch case to avoid drifting
	switch (aOperation)
	{
		case ImGuizmo::OPERATION::TRANSLATE:
			entity.lock()->GetTransform().SetLocalPosition(modifiedPosition);
			break;
		case ImGuizmo::OPERATION::ROTATE:
			entity.lock()->GetTransform().SetLocalRotation(modifiedRotation);
			break;
		case ImGuizmo::OPERATION::SCALE:
			entity.lock()->GetTransform().SetLocalScale(modifiedScale);
			break;
		default:
			break;
	}
}

void ViewportWindow::HandleMultipleObjectGuizmo(const Utils::Matrix4f& aOriginMatrix, const Utils::Matrix4f& aModifiedMatrix, const ImGuizmo::OPERATION& aOperation)
{
	const auto& selectedEntities = EditorLayer::GetSelectedEntities();
	Utils::Vector3f modifiedPosition;
	Utils::Quaternion modifiedRotation;
	Utils::Vector3f modifiedScale;
	Utils::Matrix4f::Decompose(aModifiedMatrix, modifiedPosition, modifiedRotation, modifiedScale);

	//Create a transform with the same transformations as the last-selected entity
	//Will be used as a temporary parent to all transformed entities
	Utils::Transform tempOriginParent(aOriginMatrix);

	//Store previous parents for all selected entities without selected parents
	std::vector<Utils::Transform*> oldParents(selectedEntities.size());

	for (int i = 0; i < selectedEntities.size(); ++i)
	{
		const auto& entity = selectedEntities[i];

		if (EditorLayer::HasSelectedParentRecursive(entity))
		{
			continue;
		}

		Utils::Transform* oldParent = &entity.lock()->GetParent().lock()->GetTransform();
		oldParents[i] = oldParent;

		entity.lock()->GetTransform().SetParent(&tempOriginParent);
	}

	//Do the transformation on the temporary parent
	switch (aOperation)
	{
		case ImGuizmo::OPERATION::TRANSLATE:
			tempOriginParent.SetLocalPosition(modifiedPosition);
			break;
		case ImGuizmo::OPERATION::ROTATE:
			tempOriginParent.SetLocalRotation(modifiedRotation);
			break;
		case ImGuizmo::OPERATION::SCALE:
			tempOriginParent.SetLocalScale(modifiedScale);
			break;
		default:
			break;
	}

	//After their temporary parent has transformed, reset their parents and keep their new global transformation
	for (int i = 0; i < selectedEntities.size(); ++i)
	{
		if (EditorLayer::HasSelectedParentRecursive(selectedEntities[i]))
		{
			continue;
		}

		selectedEntities[i].lock()->GetTransform().SetParent(oldParents[i]);
	}
}

void ViewportWindow::UpdateSquareSelection()
{
	if (!IsFocused())
	{
		return;
	}

	//While dragging with square multiselect, draw debug lines
	if (myIsSquareSelecting)
	{
		Utils::Vector3f clickedPos3d(myClickedPos.x, myClickedPos.y, 0);
		Utils::Vector3f actualPos3d(myActualMousePos.x, myActualMousePos.y, 0);

		clickedPos3d.x /= myActualWindowSize.x;
		clickedPos3d.x *= 2;
		clickedPos3d.x -= 1;

		clickedPos3d.y = myActualWindowSize.y - clickedPos3d.y;
		clickedPos3d.y /= myActualWindowSize.y;
		clickedPos3d.y *= 2;
		clickedPos3d.y -= 1;

		actualPos3d.x /= myActualWindowSize.x;
		actualPos3d.x *= 2;
		actualPos3d.x -= 1;

		actualPos3d.y = myActualWindowSize.y - actualPos3d.y;
		actualPos3d.y /= myActualWindowSize.y;
		actualPos3d.y *= 2;
		actualPos3d.y -= 1;

		Renderer::SubmitDebugLine({ clickedPos3d.x, clickedPos3d.y, 0 }, { clickedPos3d.x, actualPos3d.y, 0 }, { 1, 1, 1, 1 }, true);
		Renderer::SubmitDebugLine({ clickedPos3d.x, actualPos3d.y, 0 }, { actualPos3d.x, actualPos3d.y, 0 }, { 1, 1, 1, 1 }, true);
		Renderer::SubmitDebugLine({ actualPos3d.x, actualPos3d.y, 0 }, { actualPos3d.x, clickedPos3d.y, 0 }, { 1, 1, 1, 1 }, true);
		Renderer::SubmitDebugLine({ actualPos3d.x, clickedPos3d.y, 0 }, { clickedPos3d.x, clickedPos3d.y, 0 }, { 1, 1, 1, 1 }, true);
	}
	//Check for Square selection begin
	else
	{
		//Unreal Engine keybind for square selecting was pressed, start the selection
		if (!ImGuizmo::IsOver() && Utils::InputHandler::GetKeyHeld(VK_CONTROL) && ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			if (myActualMousePos.x >= 0.0f && myActualMousePos.x <= myActualWindowSize.x && myActualMousePos.y >= 0.0f && myActualMousePos.y <= myActualWindowSize.y)
			{
				myIsSquareSelecting = true;
				myClickedPos = myActualMousePos;
			}

			myShouldAddSquareSelected = ImGui::IsKeyDown(ImGuiKey_LeftShift);
		}
	}

	//Finish the square multiselect on mouse release
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && myIsSquareSelecting)
	{
		ImVec2 clicked = myClickedPos, released = myActualMousePos;

		if (clicked.x > released.x) Utils::Swap(clicked.x, released.x);
		if (clicked.y > released.y) Utils::Swap(clicked.x, released.x);

		//Calculate a custom frustum based on where square select began and ended
		const Utils::Vector2f windowSize(myActualWindowSize.x, myActualWindowSize.y);
		auto& camTransform = myEditorCamera->GetCamera()->GetTransform();

		const Utils::Vector3f nearPlaneCenter = camTransform.GetPosition() + camTransform.GetForward() * myEditorCamera->GetCamera()->GetNearPlane();
		const Utils::Plane<float> nearPlane(nearPlaneCenter, camTransform.GetBackward());

		const Utils::Vector3f farPlaneCenter = camTransform.GetPosition() + camTransform.GetForward() * myEditorCamera->GetCamera()->GetFarPlane();
		const Utils::Plane<float> farPlane(farPlaneCenter, camTransform.GetForward());

		const Utils::Vector2f topLeftScreenPos(clicked.x, clicked.y);
		const Utils::Vector3f topLeftDir = myEditorCamera->GetCamera()->ScreenPosToWorldDirection(topLeftScreenPos, windowSize);
		Utils::Ray<float> topLeftRay;
		topLeftRay.InitWithOriginAndDirection(camTransform.GetPosition(), topLeftDir);
		Utils::Vector3f nearTopLeft;
		Utils::IntersectionPlaneRay(nearPlane, topLeftRay, nearTopLeft);
		Utils::Vector3f farTopLeft;
		Utils::IntersectionPlaneRay(farPlane, topLeftRay, farTopLeft);

		const Utils::Vector2f topRightScreenPos(released.x, clicked.y);
		const Utils::Vector3f topRightDir = myEditorCamera->GetCamera()->ScreenPosToWorldDirection(topRightScreenPos, windowSize);
		Utils::Ray<float> topRightRay;
		topRightRay.InitWithOriginAndDirection(camTransform.GetPosition(), topRightDir);
		Utils::Vector3f nearTopRight;
		Utils::IntersectionPlaneRay(nearPlane, topRightRay, nearTopRight);
		Utils::Vector3f farTopRight;
		Utils::IntersectionPlaneRay(farPlane, topRightRay, farTopRight);

		const Utils::Vector2f bottomLeftScreenPos(clicked.x, released.y);
		const Utils::Vector3f bottomLeftDir = myEditorCamera->GetCamera()->ScreenPosToWorldDirection(bottomLeftScreenPos, windowSize);
		Utils::Ray<float> bottomLeftRay;
		bottomLeftRay.InitWithOriginAndDirection(camTransform.GetPosition(), bottomLeftDir);
		Utils::Vector3f nearBottomLeft;
		Utils::IntersectionPlaneRay(nearPlane, bottomLeftRay, nearBottomLeft);
		Utils::Vector3f farBottomLeft;
		Utils::IntersectionPlaneRay(farPlane, bottomLeftRay, farBottomLeft);

		const Utils::Vector2f bottomRightScreenPos(released.x, released.y);
		const Utils::Vector3f bottomRightDir = myEditorCamera->GetCamera()->ScreenPosToWorldDirection(bottomRightScreenPos, windowSize);
		Utils::Ray<float> bottomRightRay;
		bottomRightRay.InitWithOriginAndDirection(camTransform.GetPosition(), bottomRightDir);
		Utils::Vector3f nearBottomRight;
		Utils::IntersectionPlaneRay(nearPlane, bottomRightRay, nearBottomRight);
		Utils::Vector3f farBottomRight;
		Utils::IntersectionPlaneRay(farPlane, bottomRightRay, farBottomRight);

		const Utils::Plane<float> topPlane(nearTopLeft, farTopLeft, nearTopRight);
		const Utils::Plane<float> rightPlane(nearTopRight, farTopRight, nearBottomRight);
		const Utils::Plane<float> bottomPlane(nearBottomRight, farBottomRight, nearBottomLeft);
		const Utils::Plane<float> leftPlane(nearBottomLeft, farBottomLeft, nearTopLeft);

		Utils::PlaneVolume<float> squareSelection;
		squareSelection.AddPlane(topPlane);
		squareSelection.AddPlane(bottomPlane);
		squareSelection.AddPlane(rightPlane);
		squareSelection.AddPlane(leftPlane);
		squareSelection.AddPlane(nearPlane);
		squareSelection.AddPlane(farPlane);
		//

		std::vector<Ptr<Entity>> meshesInScene;

		for (auto& scene : EditorLayer::GetEditingScenes())
		{
			if (scene.expired())
			{
				LOGERROR("ViewportWindow::UpdateSqareSelection: Scene is expired");
				break;
			}

			meshesInScene.reserve(meshesInScene.size() + scene.lock()->GetEntities().size());

			for (auto& entity : scene.lock()->GetEntities())
			{
				if (entity.expired())
				{
					LOGERROR("ViewportWindow::UpdateSqareSelection: Entity is expired");
					continue;
				}

				if (entity.lock()->HasComponent<MeshComponent>() || entity.lock()->HasComponent<AnimatedMeshComponent>())
				{
					meshesInScene.push_back(entity);
				}
			}
		}

		//Select all entities with pivot inside of the custom frustum
		EditorLayer::BeginEntityUndoSeries(true);

		if (!myShouldAddSquareSelected)
		{
			//EditorLayer::DeselectAllEntities();
			EditorLayer::DeselectAllEntitiesCommand();
		}

		for (const auto& entityWeak : meshesInScene)
		{
			const auto& entity = entityWeak.lock();

			if (squareSelection.IsInside(entity->GetTransform().GetPosition()))
			{
				//TODO: Benne, figure out some smart thing that prefers selecting parents instead of children
				//Maybe if all children in a parent gets selected we select the parent instead?s

				//If prefab, select prefab root instead so children doesn't get moved, wanted behavior from LD
				if (entity->IsPrefab())
				{
					//EditorLayer::AddSelectedEntity(GetEntityWithID(entity->GetPrefabRootEntityID()));
					const auto& prefabRoot = GetEntityWithID(entity->GetPrefabRootEntityID());

					if (!EditorLayer::IsEntitySelected(prefabRoot))
					{
						const auto select = CreateRef<EntitySelectCommand>(prefabRoot, true);
						EditorLayer::ExecuteAndAddEntityUndo(select);
					}
				}
				else
				{
					//EditorLayer::AddSelectedEntity(entityWeak);
					const auto select = CreateRef<EntitySelectCommand>(entityWeak, true);
					EditorLayer::ExecuteAndAddEntityUndo(select);
				}
			}
		}

		EditorLayer::EndEntityUndoSeries(true);

		myIsSquareSelecting = false;
	}
}

void ViewportWindow::UpdateMousePicking()
{
	//Initial mouse click
	if (EditorLayer::GetSelectedEntities().empty() || !ImGuizmo::IsUsing() && ImGui::IsWindowHovered() && !ImGuizmo::IsOver())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			myClickedPos = myActualMousePos;
			myLatestClickedEntity = Renderer::GetEntityFromScreenPos(0, static_cast<int>(myActualMousePos.x), static_cast<int>(myActualMousePos.y));
		}
	}

	//On release mouse, select the originally pressed entity, or deselect all
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
	{
		if (myLatestClickedEntity > 0)
		{
			constexpr float MinMouseDistanceNoSelect = 3.0f;

			const Utils::Vector2f clicked(myClickedPos.x, myClickedPos.y);
			const Utils::Vector2f actual(myActualMousePos.x, myActualMousePos.y);

			if ((clicked - actual).Length() < MinMouseDistanceNoSelect)
			{
				Ptr<Entity> clickedEntity;

				for (const auto& scene : EditorLayer::GetEditingScenes())
				{
					clickedEntity = scene.lock()->GetEntityByID(myLatestClickedEntity);

					if (clickedEntity.lock())
					{
						break;
					}
				}

				const auto& clicked = clickedEntity.lock();

				if (clicked)
				{
					if (clicked->IsPrefab())
					{
						const auto& selectedEntities = EditorLayer::GetSelectedEntities();

						//Selected a prefab, no other selected, just select root
						if (selectedEntities.empty())
						{
							const auto select = CreateRef<EntitySelectCommand>(GetEntityWithID(clicked->GetPrefabRootEntityID()), Utils::InputHandler::GetKeyHeld(VK_CONTROL));
							EditorLayer::ExecuteAndAddEntityUndo(select);
						}
						else
						{
							const auto& curSelected = EditorLayer::GetFirstSelectedEntity().lock();

							//Parent is already selected, select child on second click
							if (curSelected->GetID() == clicked->GetPrefabRootEntityID()
								|| (!curSelected->IsPrefabRoot() && curSelected->GetPrefabRootEntityID() == clicked->GetPrefabRootEntityID() && curSelected->GetID() != clicked->GetID()))
							{
								const auto select = CreateRef<EntitySelectCommand>(clickedEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
								EditorLayer::ExecuteAndAddEntityUndo(select);
							}
							else
							{
								const auto select = CreateRef<EntitySelectCommand>(GetEntityWithID(clicked->GetPrefabRootEntityID()), Utils::InputHandler::GetKeyHeld(VK_CONTROL));
								EditorLayer::ExecuteAndAddEntityUndo(select);
							}
						}
					}
					//Not a prefab, regular select entity on mouse pick
					else
					{
						const auto select = CreateRef<EntitySelectCommand>(clickedEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
						EditorLayer::ExecuteAndAddEntityUndo(select);
					}
				}
			}
		}
		else
		{
			//EditorLayer::RemoveOutlineFromSelectedChildren(); //Shouldn't be needed with new guizmo
			//EditorLayer::DeselectAllEntities();
			EditorLayer::DeselectAllEntitiesCommand();
		}
	}
}

void ViewportWindow::CalcActualPosAndSize()
{
	const auto initialWindowPos = ImGui::GetWindowPos();
	const auto initialWindowSize = ImGui::GetWindowSize();
	const auto initialMousePos = ImGui::GetMousePos();

	auto finalWindowPos = initialWindowPos;
	auto finalWindowSize = initialWindowSize;
	auto finalMousePos = initialMousePos;

	float expectedX = initialWindowSize.y;
	float expectedY = initialWindowSize.x;
	if (mySixteenNineEnabled)
	{
		expectedX *= 16.0f / 9.0f;
		expectedY *= 9.0f / 16.0f;
	}
	else
	{
		expectedX *= initialWindowSize.x / initialWindowSize.y;
		expectedY *= initialWindowSize.y / initialWindowSize.x;
	}
	ImVec2 imageOffset = { 0.0f, 0.0f };

	if (expectedX < initialWindowSize.x)
	{
		imageOffset.x = (initialWindowSize.x - expectedX) / 2.0f;
		finalWindowSize.x = expectedX;
	}
	else if (expectedY < initialWindowSize.y)
	{
		imageOffset.y = (initialWindowSize.y - expectedY) / 2.0f;
		finalWindowSize.y = expectedY;
	}

	finalWindowPos.x += imageOffset.x;
	finalWindowPos.y += imageOffset.y;

	finalMousePos.x -= finalWindowPos.x;
	finalMousePos.y -= finalWindowPos.y;

	myActualWindowPos = finalWindowPos;
	myActualWindowSize = finalWindowSize;
	myActualMousePos = finalMousePos;
}

void ViewportWindow::ApplyDebugChanges()
{
	Renderer::SetGridActive(myGridEnabled);
	Renderer::SetDebugLinesActive(myDebugLinesEnabled);
	Renderer::SetOutlineActive(myOutlineEnabled);
}