#include "EditorPch.h"
#include "AnimatorWindow.h"
#include "Editor/Windows/WindowRegistry.h"

#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/Utilities/EditorUtils.h"

#include "imgui/imgui_internal.h"

#include "Utils/Math/LineVolume.hpp"
#include "Utils/Timer.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Animations/BlendSpace.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Components/Animation/AnimatorComponent.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/Framebuffer.h"

#include "Editor/EditorLayer.h"
#include "Editor/EditorCamera.h"

#include <sstream>
#include <fstream>
#include <map>
#include <format>

#include <Firefly/Application/Application.h>

#include "Firefly/Asset/Mesh/AnimatedMesh.h"

REGISTER_WINDOW(AnimatorWindow);




AnimatorWindow::AnimatorWindow()
	: EditorWindow("Animator")
{
	myLeftPanelSize = 200;
	myInspectorWidth = 200;
	myViewportBackgroundColor = { 0.271f, 0.267f, 0.366f, 1.f };

	myStateColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	mySelectedStateColorMultiplier = { 0.6f, 0.6f, 0.6f, 1.0f };

	myIntColor = { 0.8f, 0.53f, 0.6f, 1.f };
	myFloatColor = { 0.564f, 0.93f, 0.564f, 1.f };
	myBoolColor = { 1,0.15f,0.105f,1.f };
	myTriggerColor = { 0.678f, 0.847f, 0.9f, 1.f };

	myEntryStateColor = { 0.486f, 0.988f, 0,1.f };
	myAnyStateColor = { 0, 0.5f, 1.f, 1.f };

	myStateWidth = 200;
	myStateHeight = myStateWidth / 5.f;

	myCameraMatrix(3, 1) = 0;
	myCameraMatrix(3, 2) = 0;

	mySelectedStateID = 0;
	mySelectedTransitionID = 0;
	myCurrentLayerIndex = 0;

	myCameraMovementStartPos = { 0,0 };
	myMovingCameraFlag = false;

	myDraggingStateFlag = false;
	New();


	myPreviewRenderSceneID = Firefly::Renderer::InitializeScene();
	myPreviewMeshMaterials.push_back(Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Default"));
	myMeshPreviewTime = 0;

	myEditorCamera = CreateRef<EditorCamera>();
	myEditorCamera->Initialize(Firefly::CameraInfo());

	myWindowFlags |= ImGuiWindowFlags_MenuBar;
}

void AnimatorWindow::OnImGui()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				New();
			}
			if (ImGui::MenuItem("Save"))
			{
				if (myCurrentPath == "")
				{
					SaveAs();
				}
				else
				{
					Save(myCurrentPath);
				}
			}
			if (ImGui::MenuItem("Save as"))
			{
				SaveAs();
			}
			if (ImGui::MenuItem("Load"))
			{
				DoLoad();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	auto cursorPos = ImGui::GetCursorPos();
	DrawLeftPanel();

	//viewport begin
	ImGui::SetCursorPos(ImVec2(cursorPos.x + myLeftPanelSize, cursorPos.y));
	ImGuiUtils::ResizeWidget("AnimatorResizeParametersWidget", myLeftPanelSize);
	auto viewportBeginX = myLeftPanelSize + ImGui::GetItemRectSize().x;
	ImGui::SetCursorPos(ImVec2(cursorPos.x + viewportBeginX, cursorPos.y));

	cursorPos = ImGui::GetCursorPos();
	auto viewportSize = ImGui::GetContentRegionAvail().x - myInspectorWidth;
	DrawViewport();
	// inspector begin
	ImGui::SetCursorPos(ImVec2(cursorPos.x + viewportSize, cursorPos.y));
	ImGuiUtils::ResizeWidget("AnimatorResizeInspectorWidget", myInspectorWidth, ImGui::GetWindowWidth() - cursorPos.x - 50, true);
	auto inspectorBeginX = viewportSize + ImGui::GetItemRectSize().x + 10;
	ImGui::SetCursorPos(ImVec2(cursorPos.x + inspectorBeginX, cursorPos.y));

	DrawInspector();


	for (int i = myStatesMarkedForDelete.size() - 1; i >= 0; i--)
	{
		GetCurrentLayer().myStates.erase(myStatesMarkedForDelete[i]);
	}
	myStatesMarkedForDelete.clear();

	for (int i = myTransitionsMarkedForDelete.size() - 1; i >= 0; i--)
	{
		GetCurrentLayer().myTransitions.erase(myTransitionsMarkedForDelete[i]);
	}
	myTransitionsMarkedForDelete.clear();

}

void AnimatorWindow::OnEvent(Firefly::Event& aEvent)
{
}

Firefly::AnimatorLayer& AnimatorWindow::GetCurrentLayer()
{
	return myAnimator->myLayers[myCurrentLayerIndex];
}

Firefly::AnimatorState& AnimatorWindow::GetSelectedState()
{
	return GetCurrentLayer().myStates[mySelectedStateID];
}

Firefly::AnimatorTransition& AnimatorWindow::GetSelectedTransition()
{
	return GetCurrentLayer().myTransitions[mySelectedTransitionID];
}

Firefly::AnimatorState& AnimatorWindow::GetState(uint64_t aID)
{
	return GetCurrentLayer().myStates[aID];
}

Firefly::AnimatorTransition& AnimatorWindow::GetTransition(uint64_t aID)
{
	return GetCurrentLayer().myTransitions[aID];
}

void AnimatorWindow::AcceptAnimationFileToAddState()
{
	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".fbx"))
	{
		std::filesystem::path path = (const char*)payload->Data;
		auto relativePath = std::filesystem::relative(path, std::filesystem::current_path());
		auto& createdState = CreateState(ScreenToWorldSpace({ ImGui::GetMousePos().x,ImGui::GetMousePos().y }), GetCurrentLayer());
		createdState.AnimationPath = relativePath.string();
		createdState.Name = path.stem().string();
	}
}

void AnimatorWindow::DrawLeftPanel()
{
	ImGui::BeginChild("##AnimatorLeftPanel", ImVec2(myLeftPanelSize, 0), true);
	DrawLeftPanelTopBar();
	if (myLeftPanelTab == LeftPanelTab::Parameters)
	{
		DrawParametersPanel();
	}
	else if (myLeftPanelTab == LeftPanelTab::Layers)
	{
		DrawLayersPanel();
	}
	ImGui::EndChild();

}

void AnimatorWindow::DrawLeftPanelTopBar()
{
	ImGui::BeginTable("AnimatorLeftPanelTopBar", 2);
	ImGui::TableNextColumn();
	if (ImGui::Selectable("Parameters", myLeftPanelTab == LeftPanelTab::Parameters))
	{
		myLeftPanelTab = LeftPanelTab::Parameters;
	}
	ImGui::TableNextColumn();
	if (ImGui::Selectable("Layers", myLeftPanelTab == LeftPanelTab::Layers))
	{
		myLeftPanelTab = LeftPanelTab::Layers;
	}
	ImGui::EndTable();
}

void AnimatorWindow::DrawParametersPanel()
{

	ImGui::Text("Parameters:");
	ImGui::SameLine();
	ImGui::Button("+##AnimatorAddParameterButton");
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
	{
		for (int i = 0; i < static_cast<uint32_t>(Firefly::AnimatorParameterType::COUNT); i++)
		{
			auto type = static_cast<Firefly::AnimatorParameterType>(i);
			PushColorAccordingToParameterType(type, ImGuiCol_Text);
			if (ImGui::MenuItem(Firefly::AnimParameterToString(type).c_str()))
			{
				CreateParameter(type);
			}
			ImGui::PopStyleColor();
		}


		ImGui::EndPopup();
	}
	if (ImGui::BeginTable("##AnimatorParameterTable", 3, ImGuiTableFlags_SizingStretchProp))
	{
		for (auto& parameter : myAnimator->myParameters)
		{
			PushColorAccordingToParameterType(parameter.second.Type, ImGuiCol_Text);
			ImGui::TableNextColumn();
			ImGui::Text(Firefly::AnimParameterToString(parameter.second.Type).c_str());
			if (ImGui::BeginPopupContextItem(("##AnimatorParameterType" + std::to_string(parameter.second.ID)).c_str()))
			{
				OnRightClickParameter(parameter.second);
				ImGui::EndPopup();
			}
			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText(("##AnimatorParameterName" + std::to_string(parameter.second.ID)).c_str(), &parameter.second.Name);
			if (ImGui::BeginPopupContextItem())
			{
				OnRightClickParameter(parameter.second);
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();

			ImGui::TableNextColumn();

			auto& selectedEntities = EditorLayer::GetSelectedEntities();
			Ref<Firefly::AnimatorComponent> animatorComp = nullptr;
			if (selectedEntities.size() == 1)
			{
				auto ent = selectedEntities[0].lock();
				if (ent->HasComponent<Firefly::AnimatorComponent>())
				{
					animatorComp = ent->GetComponent<Firefly::AnimatorComponent>().lock();
				}
			}

			if (animatorComp)
			{

				switch (parameter.second.Type)
				{
				case Firefly::AnimatorParameterType::Int:
					ImGui::Text(std::to_string(animatorComp->GetIntParameterValue(parameter.first)).c_str());
					break;
				case Firefly::AnimatorParameterType::Float:
					ImGui::Text(std::format("{:.3f}", animatorComp->GetFloatParameterValue(parameter.first)).c_str());
					break;
				case Firefly::AnimatorParameterType::Bool:
					ImGui::Text(animatorComp->GetBoolParameterValue(parameter.first) ? "True" : "False");
					break;
				case Firefly::AnimatorParameterType::Trigger:
					ImGui::Text(animatorComp->GetTriggerParameterValue(parameter.first) ? "True" : "False");
					break;
				}
			}
			else
			{
				switch (parameter.second.Type)
				{
				case Firefly::AnimatorParameterType::Int:
					ImGui::Text("0");
					break;
				case Firefly::AnimatorParameterType::Float:
					ImGui::Text("0.000");
					break;
				case Firefly::AnimatorParameterType::Bool:
					ImGui::Text("False");
					break;
				case Firefly::AnimatorParameterType::Trigger:
					ImGui::Text("False");
					break;
				}
			}


		}
		ImGui::EndTable();
	}
}

void AnimatorWindow::OnRightClickParameter(Firefly::AnimatorParameter& aParameter)
{
	if (ImGui::MenuItem(("Remove##AnimatorRemoveParameter" + std::to_string(aParameter.ID)).c_str()))
	{
		RemoveParameter(aParameter.ID);
	}
}

void AnimatorWindow::DrawLayersPanel()
{
	ImGui::Text("Layers:");
	ImGui::SameLine();
	ImGui::Button("+##AnimatorAddLayerButton");
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
	{
		if (ImGui::MenuItem("Add Layer"))
		{
			CreateLayer();
		}
		ImGui::EndPopup();
	}

	auto& layers = myAnimator->myLayers;


	if (ImGui::BeginChild("##AnimatorLayersList", ImVec2(0, ImGui::GetContentRegionAvail().y - 125), true))
	{
		for (int i = 0; i < layers.size(); i++)
		{
			auto& layer = layers[i];
			ImGui::PushID(i);

			if (ImGui::Selectable(layer.myName.c_str(), myCurrentLayerIndex == i, 0))
			{
				SelectLayer(i);
			}
			auto settingsPopupID = "##LayerSettingsPopup";

			if (i != 0)
			{
				if (ImGui::BeginPopupContextItem())
				{

					ImGui::Checkbox("Additive", &layer.myIsAdditive);
					if (ImGui::MenuItem("Remove (CANNOT BE UNDONE)"))
					{
						RemoveLayer(i);
						i--;

					}
					ImGui::EndPopup();
				}
			}

			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	if (ImGui::BeginChild("##AnimatorLayerInspector"))
	{
		//ImGui::InputText(("##LayerName")layers[myCurrentLayerIndex].GetName() + ": ").c_str());
		ImGui::Separator();
		if (myCurrentLayerIndex != 0)
		{
			ImGuiUtils::BeginParameters();
			ImGuiUtils::SliderParameter("Weight", layers[myCurrentLayerIndex].myWeight, 0, 1.f, "How much influence the layer should have");
			ImGuiUtils::FileParameter("Avatar", layers[myCurrentLayerIndex].myAvatarPath, ".mask");
			if (ImGuiUtils::Button("Clear Avatar"))
			{
				layers[myCurrentLayerIndex].myAvatarPath.clear();
			}
			ImGuiUtils::EndParameters();
		}
		else
		{
			ImGui::Text("The Base layer always has 100\% weight, this cannot be changed.");
			ImGui::Text("The Base layer Cannot use a mask.");
		}


	}
	ImGui::EndChild();
}

void AnimatorWindow::DrawViewport()
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(myViewportBackgroundColor.x, myViewportBackgroundColor.y, myViewportBackgroundColor.z, myViewportBackgroundColor.w));
	ImGui::BeginChild("##AnimatorViewport", ImVec2(ImGui::GetContentRegionAvail().x - myInspectorWidth, 0));
	myViewportPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
	myViewportSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
	ImGui::PopStyleColor();

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		mySelectedStateID = 0;
		mySelectedTransitionID = 0;
	}

	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::MenuItem("Create State"))
		{
			auto mousePos = ImGui::GetMousePos();
			auto mouseWorldPos = ScreenToWorldSpace({ mousePos.x, mousePos.y });
			CreateState(mouseWorldPos, GetCurrentLayer());
		}
		ImGui::EndPopup();
	}

	DrawGrid();
	CameraMovement();
	DrawTransitions();
	if (myMakingTransitionFlag)
	{
		myMakingTransitionEndStateID = 0;
	}
	DrawStates();
	if (myMakingTransitionFlag)
	{
		DrawTransitionPreview();
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
		{
			if (myMakingTransitionEndStateID != 0)
			{
				CreateTransition(myMakingTransitionStartStateID, myMakingTransitionEndStateID, GetCurrentLayer());
			}
			myMakingTransitionStartStateID = 0;
			myMakingTransitionEndStateID = 0;
			myMakingTransitionFlag = false;
		}
	}

	if (myDraggingStateFlag)
	{
		DragState();
	}

	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".animator"))
	{
		std::filesystem::path path = (const char*)payload->Data;

		SetAnimator(path);

	}
	AcceptAnimationFileToAddState();
	ImGui::EndChild();
}

void AnimatorWindow::DrawGrid()
{
	ImU32 lineColor = 0xff222222;

	auto drawList = ImGui::GetWindowDrawList();
	float gridSpacing = myStateHeight / 2;
	float gridSize = 1000 * gridSpacing * 2;

	for (float x = -gridSize / 2; x < gridSize / 2; x += gridSpacing)
	{
		auto screenPos = WorldToScreenSpace({ x,gridSize / 2 });
		drawList->AddLine(ImVec2(screenPos.x, -screenPos.y), ImVec2(screenPos.x, screenPos.y), lineColor);
	}
	for (float y = -gridSize / 2; y < gridSize / 2; y += gridSpacing)
	{
		auto screenPos = WorldToScreenSpace({ gridSize / 2, y });
		drawList->AddLine(ImVec2(-screenPos.x, screenPos.y), ImVec2(screenPos.x, screenPos.y), lineColor);
	}
}

void AnimatorWindow::CameraMovement()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
	{
		myMovingCameraFlag = true;
		myCameraMovementStartPos = { myCameraMatrix(3,1), myCameraMatrix(3,2) };
	}
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
	{
		myMovingCameraFlag = false;
	}

	if (myMovingCameraFlag)
	{
		Utils::Vector2f mouseDelta = { ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle).x, ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle).y };
		myCameraMatrix(3, 1) = myCameraMovementStartPos.x + mouseDelta.x;
		myCameraMatrix(3, 2) = myCameraMovementStartPos.y + mouseDelta.y;
	}

	Utils::Vector2f scaleMul = Utils::Vector2f(1, 1) * (1 + ImGui::GetIO().MouseWheel * 0.1f);
	myCameraMatrix *= Utils::Matrix3f::CreateScaleMatrix(scaleMul);
}

void AnimatorWindow::DrawStates()
{
	auto drawList = ImGui::GetWindowDrawList();
	auto& layer = GetCurrentLayer();
	for (auto& statePair : layer.myStates)
	{
		Firefly::AnimatorState& state = statePair.second;
		auto stateScreenPos = WorldToScreenSpace(state.Position);

		Utils::Vector3f stateSize = { myStateWidth, myStateHeight ,0 };
		stateSize = stateSize * myCameraMatrix;

		ImVec2 stateTL = { stateScreenPos.x - stateSize.x / 2, stateScreenPos.y - stateSize.y / 2 };
		ImVec2 stateBR = { stateScreenPos.x + stateSize.x / 2, stateScreenPos.y + stateSize.y / 2 };
		ImGui::ItemAdd(ImRect(stateTL, stateBR), ImGui::GetID(("##State" + state.Name).c_str()));

		if (ImGui::IsItemHovered() && myMakingTransitionStartStateID != state.ID && myMakingTransitionFlag && GetCurrentLayer().myAnyStateID != state.ID)
		{
			myMakingTransitionEndStateID = state.ID;
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			SelectState(state.ID);
			myDraggingStateFlag = true;
			myStateDragStartPos = state.Position;
		}
		if (ImGui::BeginPopupContextItem())
		{
			SelectState(state.ID);
			OnRightClickState(state);
			ImGui::EndPopup();
		}

		ImVec4 color = ImVec4(myStateColor.x, myStateColor.y, myStateColor.z, myStateColor.w);

		if (state.ID == GetCurrentLayer().myEntryStateID)
		{
			color = { myEntryStateColor.x,
			myEntryStateColor.y,
				myEntryStateColor.z,
				myEntryStateColor.w };
		}
		else if (state.ID == GetCurrentLayer().myAnyStateID)
		{
			color = { myAnyStateColor.x,
				myAnyStateColor.y,
				myAnyStateColor.z,
				myAnyStateColor.w };
		}

		if (mySelectedStateID == state.ID)
		{
			color = { color.x * mySelectedStateColorMultiplier.x,
				color.y * mySelectedStateColorMultiplier.y,
				color.z * mySelectedStateColorMultiplier.z,
				color.w * mySelectedStateColorMultiplier.w };
		}
		drawList->AddRectFilled(stateTL, stateBR, ImGui::GetColorU32(color), 12.f);

		ImGui::SetWindowFontScale(1.f * myCameraMatrix(1, 1)); //TODO: Update this to make text sharper

		ImVec2 textSize = ImGui::CalcTextSize(state.Name.c_str());
		Utils::Vector3f textSizeTransformed = { textSize.x, textSize.y, 0 };
		drawList->AddText({ stateScreenPos.x - textSize.x / 2, stateScreenPos.y - textSize.y / 2 }, ImGui::GetColorU32({ 0, 0, 0, 1.f }), state.Name.c_str());

		ImGui::SetWindowFontScale(1);

		//draw animation time in realtime 
		if (myAnimatorFilePtr)
		{

			auto& selectedEntities = EditorLayer::GetSelectedEntities();
			if (selectedEntities.size() > 0)
			{

				auto ent = selectedEntities[0].lock();
				Ref<Firefly::AnimatorComponent> animatorComp = nullptr;
				if (ent->HasComponent<Firefly::AnimatorComponent>())
				{
					animatorComp = ent->GetComponent<Firefly::AnimatorComponent>().lock();
				}
				if (animatorComp && animatorComp->myAnimator == myAnimatorFilePtr)
				{
					auto& animatorLayerInstance = animatorComp->myLayerInstances[myCurrentLayerIndex];
					if (animatorLayerInstance.GetCurrentState().ID == state.ID)
					{
						constexpr float distanceFromSide = 10;
						constexpr float timeBarHeight = 5;
						auto normalizedTime = 1.f;
						if (animatorLayerInstance.GetCurrentAnimation())
						{
							normalizedTime = animatorLayerInstance.myCurrentAnimationTime / animatorLayerInstance.GetCurrentAnimation()->GetDuration();
						}
						ImVec2 timeBarTL = { stateTL.x + distanceFromSide, stateTL.y + stateSize.y - timeBarHeight };
						ImVec2 timeBarBR = { timeBarTL.x + (stateSize.x - distanceFromSide * 2) * normalizedTime, stateTL.y + stateSize.y };
						drawList->AddRectFilled(timeBarTL, timeBarBR, ImGui::GetColorU32({ 0, 0, 1, 1.f }), 0.f);
					}
				}
			}
		}
		//

	}
}

void AnimatorWindow::OnRightClickState(Firefly::AnimatorState& aState)
{
	//make transition
	if (ImGui::MenuItem("Make Transition"))
	{
		myMakingTransitionFlag = true;
		myMakingTransitionStartStateID = aState.ID;
	}
	if (aState.ID != GetCurrentLayer().myAnyStateID)
	{
		//set default
		if (ImGui::MenuItem("Set as Entry"))
		{
			GetCurrentLayer().myEntryStateID = aState.ID;
		}
		if (ImGui::MenuItem("Delete State"))
		{
			RemoveState(aState.ID);
		}
	}
}

void AnimatorWindow::DragState()
{
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		myDraggingStateFlag = false;
	}
	if (myDraggingStateFlag)
	{
		Utils::Vector2f mouseDelta = { ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x, ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y };
		mouseDelta /= myCameraMatrix(1, 1);
		GetSelectedState().Position = Utils::Vector2f(mouseDelta.x, mouseDelta.y) + myStateDragStartPos;
	}
}

void AnimatorWindow::DrawTransitionPreview()
{
	Utils::Vector2f transtionEndPos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };

	if (myMakingTransitionEndStateID)
	{
		transtionEndPos = GetState(myMakingTransitionEndStateID).Position;
	}
	else
	{
		transtionEndPos = ScreenToWorldSpace(transtionEndPos);
	}
	DrawTransitionArrow(GetState(myMakingTransitionStartStateID).Position, transtionEndPos, 8, { 1.f,0.4f,0.6f,1 });
}

void AnimatorWindow::DrawTransitions()
{
	//collect transition chunks (Transitions that are connected to the same states) // kinda slow operation o.o TODO: if editor laggy Optimize
	std::vector<std::vector<Firefly::AnimatorTransition>> transitionChunks;
	for (auto& toState : GetCurrentLayer().myStates)
	{
		for (auto& fromState : GetCurrentLayer().myStates)
		{
			if (toState.second.ID == fromState.second.ID)
			{
				continue;
			}
			auto chunk = CollectTransitionChunk(fromState.second.ID, toState.second.ID);
			if (chunk.size() > 0)
			{
				transitionChunks.push_back(chunk);
			}
		}
	}
	//

	for (auto& transitionChunk : transitionChunks)
	{
		//can always index 0 since we know there is at least one transition in every chunk
		auto fromPos = GetState(transitionChunk[0].FromStateID).Position;
		auto toPos = GetState(transitionChunk[0].ToStateID).Position;

		Utils::Vector2f arrowDir = toPos - fromPos;
		auto len = arrowDir.Length();
		arrowDir.Normalize();
		Utils::Vector2f arrowPerp = { -arrowDir.y, arrowDir.x };


		const float halfDistBetweenArrows = 8 / GetClampedCameraScale();

		float lerpValue = arrowDir.Dot({ 0,1 });
		lerpValue += 1;
		lerpValue /= 2;
		auto lerpedXOffset = Utils::Lerp<float>(halfDistBetweenArrows, -halfDistBetweenArrows, lerpValue);
		fromPos.x += lerpedXOffset;
		toPos.x += lerpedXOffset;

		lerpValue = arrowDir.Dot({ 1,0 });
		lerpValue += 1;
		lerpValue /= 2;
		auto lerpedYOffset = Utils::Lerp<float>(-halfDistBetweenArrows, halfDistBetweenArrows, lerpValue);
		fromPos.y += lerpedYOffset;
		toPos.y += lerpedYOffset;


		auto color = Utils::Vector4f(1, 1, 1, 1);
		//check if any of the transitions are selected
		for (auto& transition : transitionChunk)
		{
			if (transition.ID == mySelectedTransitionID)
			{
				color *= mySelectedStateColorMultiplier;
				break;
			}
		}
		DrawTransitionArrow(fromPos, toPos, 8, color, transitionChunk.size() > 1);

		//Draw realtime Transition arrow preview
		auto& selectedEntities = EditorLayer::GetSelectedEntities();
		Ref<Firefly::AnimatorComponent> animatorComp = nullptr;
		if (selectedEntities.size() == 1)
		{
			auto ent = selectedEntities[0].lock();
			if (ent->HasComponent<Firefly::AnimatorComponent>())
			{
				animatorComp = ent->GetComponent<Firefly::AnimatorComponent>().lock();
			}
		}
		if (animatorComp)
		{
			if (animatorComp->myAnimator)
			{
				if (animatorComp->myAnimator->GetID() == myAnimator->GetID())
				{
					bool isWithinThisTransitionChunk = false;
					for (auto& transition : transitionChunk)
					{
						if (transition.ID == animatorComp->myLayerInstances[myCurrentLayerIndex].GetCurrentTransition().ID)
						{
							isWithinThisTransitionChunk = true;
							break;
						}
					}
					if (animatorComp->myLayerInstances[myCurrentLayerIndex].IsTransitioning() && isWithinThisTransitionChunk)
					{
						auto transitionProgress = animatorComp->myLayerInstances[myCurrentLayerIndex].myTransitionTime / animatorComp->myLayerInstances[myCurrentLayerIndex].GetCurrentTransition().TransitionDuration;
						auto transitionToPos = Utils::Lerp(fromPos, toPos, transitionProgress);
						Utils::Vector2f start = WorldToScreenSpace(fromPos);
						Utils::Vector2f end = WorldToScreenSpace(transitionToPos);
						ImGui::GetWindowDrawList()->AddLine({ start.x, start.y }, { end.x, end.y }, ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), 3);
					}
				}
			}
		}
		//

		//check if mosue is hovering the arrow
		Utils::Vector2f mousePos = ScreenToWorldSpace({ ImGui::GetMousePos().x, ImGui::GetMousePos().y });



		//names according to if arrow is pointing straight up
		std::vector<Utils::Line<float>> lines;
		auto bl = fromPos - arrowPerp * 8.f;
		auto br = fromPos + arrowPerp * 8.f;
		auto tl = toPos - arrowPerp * 8.f;
		auto tr = toPos + arrowPerp * 8.f;

		lines.push_back({ bl, br });
		lines.push_back({ br, tr });
		lines.push_back({ tr, tl });
		lines.push_back({ tl, bl });

		Utils::LineVolume<float> lineVol(lines);

		bool isHovering = lineVol.IsInside(mousePos) && ImGui::IsWindowHovered();
		if (isHovering && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
		{
			SelectTransition(transitionChunk.front().ID);
		}
		auto id = ("Transition##" + std::to_string(transitionChunk.front().ID));
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && isHovering)
			ImGui::OpenPopup(id.c_str());
		if (ImGui::BeginPopup(id.c_str()))
		{
			if (transitionChunk.size() == 1)
			{
				OnRightClickTransition(transitionChunk.front());

			}
			else if (transitionChunk.size() > 1)
			{
				OnRightClickTransitionChunk(transitionChunk);
			}
			ImGui::EndPopup();
		}


	}
}

//kinda slow since it makes copies of the transitions, but it's not that bad 
//TODO: optimize collection to not copy transitions
std::vector<Firefly::AnimatorTransition> AnimatorWindow::CollectTransitionChunk(uint64_t aFromID, uint64_t aToID)
{
	std::vector<Firefly::AnimatorTransition> transitions;
	for (auto& transition : GetCurrentLayer().myTransitions)
	{
		if (transition.second.FromStateID == aFromID && transition.second.ToStateID == aToID)
		{
			transitions.push_back(transition.second);
		}
	}
	return transitions;
}

void AnimatorWindow::OnRightClickTransition(Firefly::AnimatorTransition& aTransition)
{
	if (ImGui::MenuItem("Remove"))
	{
		RemoveTransition(aTransition.ID);
	}
}

void AnimatorWindow::OnRightClickTransitionChunk(std::vector<Firefly::AnimatorTransition>& aTransitions)
{
	if (ImGui::MenuItem("Remove all"))
	{
		for (auto& transition : aTransitions)
			RemoveTransition(transition.ID);
	}
}

void AnimatorWindow::DrawTransitionArrow(Utils::Vector2f aStart, Utils::Vector2f aEnd, float aArrowSize, Utils::Vector4f aColor, bool aTrippleTriangleFlag)
{
	auto drawList = ImGui::GetWindowDrawList();
	Utils::Vector2f dir = aEnd - aStart;
	dir.Normalize();
	Utils::Vector2f perp = { dir.y, -dir.x };
	//arrow in the middle of the line
	Utils::Vector2f arrowStart = aStart + dir * (aEnd - aStart).Length() / 2.f;

	float scaledArrowSize = aArrowSize / GetClampedCameraScale();
	arrowStart += dir * aArrowSize / myCameraMatrix(1, 1) / 2.f;
	float arrowLength = scaledArrowSize * 1.5f;

	Utils::Vector2f arrowEnd0 = arrowStart + dir * arrowLength;
	Utils::Vector2f arrowEnd1 = arrowStart + perp * scaledArrowSize;
	Utils::Vector2f arrowEnd2 = arrowStart - perp * scaledArrowSize;




	Utils::Vector2f start = WorldToScreenSpace(aStart);
	Utils::Vector2f end = WorldToScreenSpace(aEnd);
	Utils::Vector2f arrowEnd = WorldToScreenSpace(arrowEnd0);
	Utils::Vector2f arrow1 = WorldToScreenSpace(arrowEnd1);
	Utils::Vector2f arrow2 = WorldToScreenSpace(arrowEnd2);

	drawList->AddLine({ start.x, start.y }, { end.x,end.y }, ImGui::GetColorU32({ aColor.x, aColor.y, aColor.z, aColor.w }), 3.f);
	drawList->AddTriangleFilled({ arrow1.x, arrow1.y }, { arrow2.x, arrow2.y }, { arrowEnd.x,arrowEnd.y }, ImGui::GetColorU32({ aColor.x, aColor.y, aColor.z, aColor.w }));


	if (aTrippleTriangleFlag)
	{
		auto offset = dir * arrowLength * myCameraMatrix(1, 1) + dir * 4.f;

		auto arrowEndBehind = arrowEnd - offset;
		auto arrow1Behind = arrow1 - offset;
		auto arrow2Behind = arrow2 - offset;

		drawList->AddTriangleFilled({ arrow1Behind.x, arrow1Behind.y }, { arrow2Behind.x, arrow2Behind.y }, { arrowEndBehind.x,arrowEndBehind.y }, ImGui::GetColorU32({ aColor.x, aColor.y, aColor.z, aColor.w }));

		auto arrowEndFront = arrowEnd + offset;
		auto arrow1Front = arrow1 + offset;
		auto arrow2Front = arrow2 + offset;

		drawList->AddTriangleFilled({ arrow1Front.x, arrow1Front.y }, { arrow2Front.x, arrow2Front.y }, { arrowEndFront.x,arrowEndFront.y }, ImGui::GetColorU32({ aColor.x, aColor.y, aColor.z, aColor.w }));


	}

}

void AnimatorWindow::DrawInspector()
{
	if (ImGui::BeginChild("##AnimatorInspectorPanel"))
	{

		if (mySelectedStateID != 0)
		{
			DrawStateInspector();
		}
		else if (mySelectedTransitionID != 0)
		{
			DrawTransitionInspector();
		}
	}
	ImGui::EndChild();
}

void AnimatorWindow::DrawStateInspector()
{
	Firefly::AnimatorState& state = GetState(mySelectedStateID);

	if (state.ID != GetCurrentLayer().myAnyStateID)
	{
		if (ImGuiUtils::BeginParameters(std::to_string(state.ID)))
		{

			ImGuiUtils::Parameter("Name", state.Name);
			ImGuiUtils::FileParameter("Animation", state.AnimationPath, ".anim;.blend");
			if (!state.AnimationPath.empty())
			{
				const std::string extension = state.AnimationPath.substr(state.AnimationPath.find_last_of('.'));
				if (extension == ".blend")
				{
					auto blend = Firefly::ResourceCache::GetAsset<Firefly::BlendSpace>(state.AnimationPath);
					if (blend)
					{
						//collect all parameters with type float 
						std::vector<std::string> parameterNames;
						parameterNames.push_back("None");
						std::vector<uint64_t> parameterIDs;
						parameterIDs.push_back(0);

						for (auto& parameter : myAnimator->GetParameters())
						{
							if (parameter.second.Type == Firefly::AnimatorParameterType::Float)
							{
								parameterNames.push_back(parameter.second.Name);
								parameterIDs.push_back(parameter.first);
							}
						}
						//find what index is the current parameter
						uint32_t index = 0;
						for (int i = 0; i < parameterIDs.size(); i++)
						{
							if (parameterIDs[i] == state.HorizontalAxisParamID)
							{
								index = i;
								break;
							}
						}
						if (ImGuiUtils::EnumParameter(blend->GetHorizontalAxisName(), index, parameterNames))
						{
							state.HorizontalAxisParamID = parameterIDs[index];
						}
						if (blend->GetDimensionType() == Firefly::BlendspaceType::TwoDimensional)
						{
							for (int i = 0; i < parameterIDs.size(); i++)
							{
								if (parameterIDs[i] == state.VerticalAxisParamID)
								{
									index = i;
									break;
								}
							}
							if (ImGuiUtils::EnumParameter(blend->GetVerticalAxisName(), index, parameterNames))
							{
								state.VerticalAxisParamID = parameterIDs[index];
							}
						}
					}
				}
			}

			ImGuiUtils::Parameter("Speed", state.Speed);
			ImGuiUtils::Parameter("Looping", state.Looping);

			ImGuiUtils::EndParameters();
		}
	}

}

void AnimatorWindow::DrawTransitionInspector()
{
	if (ImGui::BeginChild("##AnimatorTransitionInspectorChild", { 0,ImGui::GetContentRegionAvail().y * 0.60f }))
	{

		auto transitionChunk = CollectTransitionChunk(GetTransition(mySelectedTransitionID).FromStateID, GetTransition(mySelectedTransitionID).ToStateID);

		for (int i = 0; i < transitionChunk.size(); i++)
		{
			if (ImGui::Selectable((transitionChunk[i].Name + "##TransitionSelector" + std::to_string(transitionChunk[i].ID)).c_str(), transitionChunk[i].ID == mySelectedTransitionID))
			{
				SelectTransition(transitionChunk[i].ID);
			}
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove"))
				{
					auto id = transitionChunk[i].ID;
					transitionChunk.erase(std::find_if(transitionChunk.begin(), transitionChunk.end(),
						[&](Firefly::AnimatorTransition& aTransition) {return aTransition.ID == id; }));
					RemoveTransition(id);
					if (transitionChunk.size() > 0)
					{
						SelectTransition(transitionChunk.front().ID);
					}
					else
					{
						ImGui::EndPopup();
						return;
					}
					i--;
				}
				ImGui::EndPopup();
			}
		}
		ImGui::Separator();

		Firefly::AnimatorTransition& transition = GetTransition(mySelectedTransitionID);

		ImGui::Text("From: %s", GetState(transition.FromStateID).Name.c_str());
		ImGui::Text("To: %s", GetState(transition.ToStateID).Name.c_str());
		ImGui::Separator();

		if (ImGuiUtils::BeginParameters("##" + std::to_string(transition.ID)))
		{
			ImGuiUtils::Parameter("Has Exit Time", transition.HasExitTime);
			ImGuiUtils::Parameter("Exit time", transition.ExitTime, 0.05f, 0.0f, 0.0f,
				"If has ");
			if (transition.ExitTime < 0)
			{
				transition.ExitTime = 0;
			}
			ImGuiUtils::Parameter("Transition Duration (s)", transition.TransitionDuration, 0.05f);
			if (transition.TransitionDuration < 0)
			{
				transition.TransitionDuration = 0;
			}
			ImGuiUtils::EndParameters();
		}

		//Draw blend showcase
		auto drawList = ImGui::GetWindowDrawList();

		//Background
		auto& fromState = GetState(transition.FromStateID);
		auto& toState = GetState(transition.ToStateID);
		auto cursorScreenPos = ImGui::GetCursorScreenPos();
		const int transitionShowcaseHeight = 125;
		ImRect transitionShowcaseRect = { cursorScreenPos,{ cursorScreenPos.x + ImGui::GetContentRegionAvail().x, cursorScreenPos.y + transitionShowcaseHeight } };
		drawList->AddRectFilled(transitionShowcaseRect.Min, transitionShowcaseRect.Max,
			ImGui::GetColorU32({ 0.2f, 0.2f, 0.2f, 1.f }), 10);
		//
		ImGui::PushClipRect(transitionShowcaseRect.Min, transitionShowcaseRect.Max, true);


		//Time steps
		float fromAnimationTime = 1;
		float toAnimationTime = 1;
		if (!fromState.AnimationPath.empty())
		{
			const std::string fromExtension = fromState.AnimationPath.substr(fromState.AnimationPath.find_last_of('.'));
			if (fromExtension == ".anim")
			{
				auto anim = Firefly::ResourceCache::GetAsset<Firefly::Animation>(fromState.AnimationPath, true);
				if (anim)
				{
					fromAnimationTime = anim->GetDuration();
				}
			}
			if (fromExtension == ".anim")
			{
				auto anim = Firefly::ResourceCache::GetAsset<Firefly::Animation>(fromState.AnimationPath, true);
				if (anim)
				{
					toAnimationTime = anim->GetDuration();
				}
			}
		}

		const float rectWidth = (transitionShowcaseRect.Max.x - transitionShowcaseRect.Min.x);
		const float timeSpan = fromAnimationTime + toAnimationTime + 1;
		const float timeStepSize = 0.5f;
		const int maxTimeSteps = timeSpan / timeStepSize;
		const float widthPerTimeStep = rectWidth / maxTimeSteps;
		int timeStep = 0;

		ImGuiUtils::PushFont(ImGuiUtilsFont_Roboto_10);
		for (int timeStep = 0; timeStep < maxTimeSteps; timeStep++)
		{
			auto xPos = transitionShowcaseRect.Min.x + timeStep * widthPerTimeStep;
			drawList->AddLine(ImVec2(xPos, transitionShowcaseRect.Min.y),
				ImVec2(xPos, transitionShowcaseRect.Max.y), ImGui::GetColorU32({ 0.8f,  0.8f,  0.8f, 1.f }), 1);

			std::ostringstream time;
			time.precision(2);
			time << std::fixed << timeStepSize * timeStep;

			drawList->AddText({ xPos + 2, transitionShowcaseRect.Min.y },
				ImGui::GetColorU32({ 0.8f,0.8f,0.8f,1.f }), (time).str().c_str());
		}
		ImGuiUtils::PopFont();
		//

		const int showcaseStateHeight = 50;
		//from state
		const float fromStateWidth = widthPerTimeStep * fromAnimationTime / timeStepSize;
		const ImVec2 fromStateMin = { transitionShowcaseRect.Min.x , transitionShowcaseRect.Max.y - showcaseStateHeight * 2 };
		const ImVec2 fromStateMax = { fromStateMin.x + fromStateWidth , fromStateMin.y + showcaseStateHeight };

		drawList->AddRectFilled(fromStateMin, fromStateMax,
			ImGui::GetColorU32({ myStateColor.x,myStateColor.y, myStateColor.z, myStateColor.w }), 5);
		//drawList->AddRect(fromStateMin, fromStateMax,
		//	ImGui::GetColorU32({ 0.2f,  0.2f,  0.3f, 1.f }), 5, 0, 3);
		drawList->AddText(fromStateMin, ImGui::GetColorU32({ 0,0,0,1.f }), fromState.Name.c_str());
		//

		//to state
		const float toStateWidth = widthPerTimeStep * toAnimationTime / timeStepSize;
		const ImVec2 toStateMin = { fromStateMax.x, fromStateMax.y };
		const ImVec2 toStateMax = { fromStateMax.x + toStateWidth, toStateMin.y + showcaseStateHeight };

		drawList->AddRectFilled(toStateMin, toStateMax,
			ImGui::GetColorU32({ myStateColor.x,myStateColor.y, myStateColor.z, myStateColor.w }), 5);
		//drawList->AddRect(toStateMin, toStateMax,
		//	ImGui::GetColorU32({ 0.2f,  0.2f,  0.4f, 1.f }), 5, 0, 3);
		drawList->AddText(toStateMin, ImGui::GetColorU32({ 0,0,0,1.f }), toState.Name.c_str());
		//

		//Transition Preview
		const float startTime = fromAnimationTime * transition.ExitTime;
		const float endTime = startTime + transition.TransitionDuration;

		std::array<ImVec2, 4> transitionPreviewPoints = {
			ImVec2(transitionShowcaseRect.Min.x + widthPerTimeStep * startTime / timeStepSize, fromStateMin.y),
			ImVec2(transitionShowcaseRect.Min.x + widthPerTimeStep * endTime / timeStepSize, toStateMin.y),
			ImVec2(transitionShowcaseRect.Min.x + widthPerTimeStep * endTime / timeStepSize, toStateMax.y),
			ImVec2(transitionShowcaseRect.Min.x + widthPerTimeStep * startTime / timeStepSize, fromStateMax.y)
		};

		drawList->AddConvexPolyFilled(transitionPreviewPoints.data(), 4, ImGui::GetColorU32({ 0.3f, 0.3f, 0, 0.7f }));
		//

		ImGui::PopClipRect();
		//

		ImGui::SetCursorScreenPos({ cursorScreenPos.x, cursorScreenPos.y + transitionShowcaseHeight });

		ImGui::Separator();

		ImGui::PushID(transition.ID);
		for (int i = 0; i < transition.Parameters.size(); i++)
		{
			auto& paramInst = transition.Parameters[i];
			auto& param = myAnimator->GetParameter(paramInst.ParameterID);

			auto is = static_cast<uint32_t>(paramInst.Condition);
			std::vector<std::string> availableIsConditions;
			if (param.Type == Firefly::AnimatorParameterType::Float)
			{
				availableIsConditions.push_back("Less");
				availableIsConditions.push_back("Greater");
			}
			else if (param.Type == Firefly::AnimatorParameterType::Int)
			{
				availableIsConditions.push_back("Less");
				availableIsConditions.push_back("Greater");
				availableIsConditions.push_back("Equal");
			}
			else if (param.Type == Firefly::AnimatorParameterType::Bool)
			{
				availableIsConditions.push_back("False");
				availableIsConditions.push_back("True");
			}

			auto cursorScreenPos = ImGui::GetCursorScreenPos();
			ImGui::BeginTable("##AnimatorParameterTable", 3, ImGuiTableFlags_Resizable);
			ImGui::TableNextColumn();
			ImGui::Text(param.Name.c_str());
			ImGui::TableNextColumn();
			if (param.Type != Firefly::AnimatorParameterType::Trigger && param.Type != Firefly::AnimatorParameterType::Bool)
			{
				ImGuiUtils::Combo(("##" + param.Name + std::to_string(i)), is, availableIsConditions, ImGuiComboFlags_NoArrowButton);
			}
			ImGui::TableNextColumn();
			switch (param.Type)
			{
			case Firefly::AnimatorParameterType::Float:
				ImGuiUtils::DragFloat(("##" + std::to_string(i)), *reinterpret_cast<float*>(&(paramInst.Value)));
				break;

			case Firefly::AnimatorParameterType::Int:
				ImGuiUtils::DragInt(("##" + std::to_string(i)), *reinterpret_cast<int*>(&(paramInst.Value)));
				break;

			case Firefly::AnimatorParameterType::Bool:
				ImGuiUtils::Combo(("##" + param.Name + std::to_string(i)), is, availableIsConditions, ImGuiComboFlags_NoArrowButton);
				break;

			case Firefly::AnimatorParameterType::Trigger:
				auto cursorPos = ImGui::GetCursorPos();
				ImGui::Text(" Triggered");
				break;
			}
			paramInst.Condition = static_cast<uint8_t> (is);
			ImGui::EndTable();
			ImRect rect;
			rect.Min = cursorScreenPos;
			rect.Max = { cursorScreenPos.x + ImGui::GetContentRegionAvail().x, ImGui::GetCursorScreenPos().y };
			ImGui::ItemAdd(rect, ImGui::GetID(("##TransitionInspectorParameter" + std::to_string(paramInst.ID)).c_str()));
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove"))
				{
					transition.Parameters.erase(std::find_if(transition.Parameters.begin(), transition.Parameters.end(),
						[paramInst](const Firefly::AnimatorParameterInstance& aParamInst) { return paramInst.ID == aParamInst.ID; }));
					i--;
				}
				ImGui::EndPopup();
			}
		}
		ImGui::Button("Add Parameter");
		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
		{
			for (auto& paramPair : myAnimator->myParameters)
			{
				auto& param = paramPair.second;
				if (ImGui::MenuItem((param.Name + "##" + std::to_string(param.ID)).c_str()))
				{
					Firefly::AnimatorParameterInstance p;
					memset(&p.Value, 0, sizeof(p.Value));
					p.ID = GenerateRandomID();
					p.Condition = 0;
					p.ParameterID = param.ID;
					transition.Parameters.push_back(p);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
	DrawInspectorMeshPreview();

}

void AnimatorWindow::DrawParameterInstanceInInspector(Firefly::AnimatorParameterInstance& aInstance)
{
	//ImGuiUtils::Parameter();
}

void AnimatorWindow::DrawInspectorMeshPreview()
{
	ImGui::BeginChild("AnimatorMeshPreview");
	Firefly::Renderer::BeginScene(myPreviewRenderSceneID);
	myEditorCamera->Update();
	myEditorCamera->SetActiveCamera();

	auto windowPos = ImGui::GetWindowPos();
	auto contentRegionAvail = ImGui::GetContentRegionAvail();
	myEditorCamera->SetViewportRect({ static_cast<int>(windowPos.x), static_cast<int>(windowPos.y), static_cast<int>(windowPos.x + contentRegionAvail.x), static_cast<int>(windowPos.y + contentRegionAvail.y) });

	//Resize to content region
	if (Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Width) - contentRegionAvail.x) > 1.0f
		|| Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Height) - contentRegionAvail.y) > 1.0f)
	{
		auto newSize = Utils::Vector2<uint32_t>(contentRegionAvail.x, contentRegionAvail.y);
		Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->Resize(newSize);
		myEditorCamera->GetCamera()->SetSizeX(static_cast<float>(newSize.x));
		myEditorCamera->GetCamera()->SetSizeY(static_cast<float>(newSize.y));
	}

	if (myPreviewMesh)
	{

		auto& fromStateId = GetTransition(mySelectedTransitionID).FromStateID;
		auto& toStateId = GetTransition(mySelectedTransitionID).ToStateID;
		auto& fromState = GetState(fromStateId);
		auto& toState = GetState(toStateId);
		auto fromStateAnimation = Firefly::ResourceCache::GetAsset<Firefly::Animation>(fromState.AnimationPath);
		std::vector<Utils::Matrix4f> matrices;
		if (fromStateAnimation)
		{
			myMeshPreviewTime += Utils::Timer::GetDeltaTime();
			if (myMeshPreviewTime >= fromStateAnimation->GetDuration())
			{
				myMeshPreviewTime -= fromStateAnimation->GetDuration();
			}
			auto frame = fromStateAnimation->GetFrame(myMeshPreviewTime, true);
			frame.CalculateTransforms(myPreviewMesh->GetSkeleton(), matrices);
		}
		Firefly::MeshSubmitInfo cmd(Utils::Matrix4f(), true);
		cmd.Mesh = &myPreviewMesh->GetSubMeshes()[0];
		cmd.SetBoneTransforms(matrices);
		if (!myPreviewMeshMaterials.empty())
		{
			//cmd.Materials = myPreviewMeshMaterials;
		}
		Firefly::Renderer::Submit(cmd);

	}
	Firefly::Renderer::EndScene();
	auto frame = Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID);
	ImGui::Image(frame->GetColorAttachment(0).Get(), { contentRegionAvail.x , contentRegionAvail.y });

	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".fbx"))
	{
		std::string path = (const char*)payload->Data;
		myPreviewMesh = Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(path);
	}
	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".mat"))
	{
		std::string path = (const char*)payload->Data;
		myPreviewMeshMaterials[0] = Firefly::ResourceCache::GetAsset < Firefly::MaterialAsset>(path);
	}

	ImGui::EndChild();
}

Firefly::AnimatorState& AnimatorWindow::CreateState(Utils::Vector2f aPos, Firefly::AnimatorLayer& aLayer)
{
	std::string name = "New State";
	//check if name is already taken
	int i = 0;
	bool found = true;
	while (found)
	{
		found = false;
		for (auto& state : aLayer.myStates)
		{
			if (state.second.Name == name)
			{
				name = "New State" + std::to_string(i);
				i++;
				found = true;
				break;
			}
		}
	}

	Firefly::AnimatorState state = {};
	state.Name = name;
	state.ID = GenerateRandomID();
	state.Position = aPos;
	aLayer.myStates[state.ID] = state;

	return aLayer.myStates[state.ID];
}

Firefly::AnimatorParameter& AnimatorWindow::CreateParameter(Firefly::AnimatorParameterType aType)
{
	std::string name = "new " + Firefly::AnimParameterToString(aType) + " Parameter";
	//check if name is already taken
	int i = 0;
	bool found = true;
	while (found)
	{
		found = false;
		for (auto& param : myAnimator->myParameters)
		{
			if (param.second.Name == name)
			{
				name = "new " + Firefly::AnimParameterToString(aType) + " Parameter" + std::to_string(i);
				i++;
				found = true;
				break;
			}
		}
	}
	Firefly::AnimatorParameter param;
	param.Name = name;
	param.ID = GenerateRandomID();
	param.Type = aType;
	myAnimator->myParameters.emplace(param.ID, param);

	return myAnimator->myParameters[param.ID];

}

Firefly::AnimatorTransition& AnimatorWindow::CreateTransition(uint64_t aFromState, uint64_t aToState, Firefly::AnimatorLayer& aLayer)
{
	Firefly::AnimatorTransition transition;
	transition.ID = GenerateRandomID();
	transition.FromStateID = aFromState;
	transition.ToStateID = aToState;
	aLayer.myTransitions.emplace(transition.ID, transition);

	SelectTransition(transition.ID);
	return aLayer.myTransitions.at(transition.ID);
}

Firefly::AnimatorLayer& AnimatorWindow::CreateLayer()
{
	std::string name = "New Layer";
	//check if name is already taken
	int i = 0;
	bool found = true;
	while (found)
	{
		found = false;
		for (auto& layer : myAnimator->myLayers)
		{
			if (layer.myName == name)
			{
				name = "New Layer" + std::to_string(i);
				i++;
				found = true;
				break;
			}
		}
	}

	Firefly::AnimatorLayer layer;
	layer.myName = name;

	auto& anyState = CreateState({ 0,40 }, layer);
	anyState.Name = "Any state";
	layer.myAnyStateID = anyState.ID;

	auto& defaultEntryState = CreateState({ 0, -40 }, layer);
	layer.myEntryStateID = defaultEntryState.ID;


	myAnimator->myLayers.push_back(layer);
	myLayerInfoList.push_back({ 0,0 });

	SelectLayer(myAnimator->myLayers.size() - 1);
	return myAnimator->myLayers.back();
}

void AnimatorWindow::RemoveState(uint64_t aID)
{
	auto& layer = GetCurrentLayer();
	if (aID == layer.myEntryStateID)
	{
		layer.myEntryStateID = 0;
	}
	if (aID == mySelectedStateID)
	{
		DeselectAll();
	}
	if (aID == myMakingTransitionStartStateID)
	{
		myMakingTransitionStartStateID = 0;
		myMakingTransitionFlag = false;
	}
	myStatesMarkedForDelete.push_back(aID);

	//also remove all transitions linked
	std::vector<uint64_t> transitionsToRemove;
	for (auto& transPair : layer.myTransitions)
	{
		auto& trans = transPair.second;
		if (trans.FromStateID == aID || trans.ToStateID == aID)
		{
			myTransitionsMarkedForDelete.push_back(trans.ID);
		}
	}
}

void AnimatorWindow::RemoveParameter(uint64_t aID)
{
	myAnimator->myParameters.erase(aID);
	for (auto& trans : GetCurrentLayer().myTransitions)
	{
		std::vector<uint64_t> conditionsToRemove;
		for (auto& param : trans.second.Parameters)
		{
			if (param.ParameterID == aID)
			{
				conditionsToRemove.push_back(param.ID);
			}
		}
	}
}

void AnimatorWindow::RemoveTransition(uint64_t aID)
{
	if (aID == mySelectedTransitionID)
	{
		DeselectAll();
	}
	myTransitionsMarkedForDelete.push_back(aID);
}

void AnimatorWindow::RemoveLayer(size_t aIndex)
{
	if (aIndex == myCurrentLayerIndex)
	{
		SelectLayer(0);
	}
	if (aIndex < myCurrentLayerIndex)
	{
		myCurrentLayerIndex--;
	}

	if (aIndex < myAnimator->myLayers.size())
	{
		myAnimator->myLayers.erase(myAnimator->myLayers.begin() + aIndex);
		myLayerInfoList.erase(myLayerInfoList.begin() + aIndex);
	}
	else
	{
		LOGERROR("AnimatorWindow::RemoveLayer: Layer at index {} does not exist", aIndex);
	}
}

void AnimatorWindow::SelectState(uint64_t aID)
{
	DeselectAll();
	mySelectedStateID = aID;
}

void AnimatorWindow::SelectTransition(uint64_t aID)
{
	DeselectAll();
	mySelectedTransitionID = aID;
}

void AnimatorWindow::SelectLayer(size_t aIndex)
{
	myLayerInfoList[myCurrentLayerIndex].mySelectedStateID = mySelectedStateID;
	myLayerInfoList[myCurrentLayerIndex].mySelectedTransitionID = mySelectedTransitionID;
	DeselectAll();
	myCurrentLayerIndex = aIndex;
	mySelectedStateID = myLayerInfoList[myCurrentLayerIndex].mySelectedStateID;
	mySelectedTransitionID = myLayerInfoList[myCurrentLayerIndex].mySelectedTransitionID;

}

void AnimatorWindow::DeselectAll()
{
	mySelectedTransitionID = 0;
	mySelectedStateID = 0;
	myDraggingStateFlag = false;
}

void AnimatorWindow::PushColorAccordingToParameterType(Firefly::AnimatorParameterType aType, int aImGuiCol)
{
	auto color = myFloatColor;

	switch (aType)
	{
	case Firefly::AnimatorParameterType::Bool:
		color = myBoolColor;
		break;
	case Firefly::AnimatorParameterType::Int:
		color = myIntColor;
		break;
	case Firefly::AnimatorParameterType::Float:
		color = myFloatColor;
		break;
	case Firefly::AnimatorParameterType::Trigger:
		color = myTriggerColor;
		break;
	}

	ImVec4 colorVec = { color.x, color.y, color.z, color.w };
	ImGui::PushStyleColor(aImGuiCol, colorVec);
}

Utils::Vector2f AnimatorWindow::WorldToScreenSpace(Utils::Vector2f aPos)
{
	Utils::Vector3f result3 = { aPos.x, aPos.y, 1 };
	result3 = result3 * myCameraMatrix;
	return Utils::Vector2f(result3.x, result3.y) + myViewportPos + myViewportSize / 2.f;
}

Utils::Vector2f AnimatorWindow::ScreenToWorldSpace(Utils::Vector2f aPos)
{
	Utils::Vector2f result = aPos - myViewportPos - myViewportSize / 2.f;
	Utils::Vector3f result3 = { result.x, result.y, 1 };
	result3 = result3 * Utils::Matrix3f::GetInverse(myCameraMatrix);
	return Utils::Vector2f(result3.x, result3.y);
}

float AnimatorWindow::GetClampedCameraScale()
{
	float result = myCameraMatrix(1, 1);
	if (result < 0.6f)
	{
		result = 0.6f;
	}
	else if (result > 3)
	{
		result = 3;
	}


	return result;
}

uint64_t AnimatorWindow::GenerateRandomID()
{
	//generate random uint64_t
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis;
	return  dis(gen);
}

void AnimatorWindow::New()
{
	myLayerInfoList.clear();
	mySelectedTransitionID = 0;
	mySelectedStateID = 0;
	myAnimatorFilePtr = nullptr;
	myAnimator = CreateRef<Firefly::Animator>();
	myAnimator->myID = GenerateRandomID();

	Firefly::AnimatorLayer& layer = CreateLayer();
	layer.myName = "Base Layer";
}

void AnimatorWindow::Save(const std::filesystem::path& aPath)
{
	if (GetCurrentLayer().GetEntryStateID() == 0)
	{
		ImGuiUtils::NotifyError("Cannot save an animator without an entry state! Please mark a state as the entry state.", myCurrentPath.string());
		return;
	}
	std::ofstream file(aPath);
	if (!file.is_open())
	{
		ImGuiUtils::NotifyError("Could not save animator to path \"{}\"\n File could not be opened for write, make sure it is checked out in perforce!", myCurrentPath.string());
		return;
	}
	nlohmann::json json;

	json["ID"] = myAnimator->myID;
	auto& layersJsonArr = json["Layers"];

	for (int layerIndex = 0; layerIndex < myAnimator->myLayers.size(); layerIndex++)
	{
		auto& layersJson = layersJsonArr[layerIndex];
		auto& statesJsonArr = layersJsonArr[layerIndex]["States"];
		int i = 0;
		for (auto& statePair : myAnimator->myLayers[layerIndex].myStates)
		{
			auto& state = statePair.second;
			nlohmann::json& stateJson = statesJsonArr[i];
			stateJson["Name"] = state.Name;
			stateJson["AnimationPath"] = state.AnimationPath;
			stateJson["ID"] = state.ID;
			stateJson["Position"] = { state.Position.x, state.Position.y };
			stateJson["Speed"] = state.Speed;
			stateJson["Looping"] = state.Looping;
			stateJson["BlendspaceHorizontalAxisParamID"] = state.HorizontalAxisParamID;
			stateJson["BlendspaceVerticalAxisParamID"] = state.VerticalAxisParamID;

			i++;
		}

		auto& transitionsJsonArr = layersJson["Transitions"];
		i = 0;
		for (auto& transitionPair : myAnimator->myLayers[layerIndex].myTransitions)
		{
			auto& transition = transitionPair.second;
			nlohmann::json& transitionJson = transitionsJsonArr[i];
			transitionJson["ID"] = transition.ID;
			transitionJson["FromStateID"] = transition.FromStateID;
			transitionJson["ToStateID"] = transition.ToStateID;
			transitionJson["HasExitTime"] = transition.HasExitTime;
			transitionJson["ExitTime"] = transition.ExitTime;
			transitionJson["TransitionDuration"] = transition.TransitionDuration;

			auto& parametersJsonArr = transitionJson["Parameters"];
			for (int j = 0; j < transition.Parameters.size(); j++)
			{
				auto& parameter = transition.Parameters[j];
				nlohmann::json& parameterJson = parametersJsonArr[j];
				parameterJson["ID"] = parameter.ID;
				parameterJson["ParameterID"] = parameter.ParameterID;
				parameterJson["Condition"] = parameter.Condition;
				parameterJson["Value"] = *(reinterpret_cast<int*>(&parameter.Value[0]));
			}
			i++;
		}
		layersJson["AnyStateID"] = myAnimator->myLayers[layerIndex].myAnyStateID;
		layersJson["EntryStateID"] = myAnimator->myLayers[layerIndex].myEntryStateID;
		layersJson["Name"] = myAnimator->myLayers[layerIndex].myName;
		layersJson["IsAdditive"] = myAnimator->myLayers[layerIndex].myIsAdditive;
		layersJson["Avatar"] = myAnimator->myLayers[layerIndex].myAvatarPath;
		layersJson["Weight"] = myAnimator->myLayers[layerIndex].myWeight;
	}

	auto& parametersJsonArr = json["Parameters"];
	int i = 0;
	for (auto& parameterPair : myAnimator->myParameters)
	{
		auto& parameter = parameterPair.second;
		nlohmann::json& parameterJson = parametersJsonArr[i];
		parameterJson["Name"] = parameter.Name;
		parameterJson["Type"] = static_cast<int>(parameter.Type);
		parameterJson["ID"] = parameter.ID;
		i++;
	}




	file << std::setw(4) << json;
	file.close();

	if (myAnimatorFilePtr)
	{
		*myAnimatorFilePtr = *myAnimator; //update the cached animator file
	}

	EditorAnimatorChangedEvent ev(myAnimator->GetID());
	Firefly::Application::Get().OnEvent(ev);

	ImGuiUtils::NotifySuccess("Successfully saved animator to path: \"{}\"", aPath.string());
}

void AnimatorWindow::SaveAs()
{
	std::filesystem::path path = EditorUtils::GetSaveFilePath("Animator Controller (*.animator)\0*.animator\0", "animator");
	if (path == "")
		return;

	myAnimator->myID = GenerateRandomID();
	Save(path);
	myCurrentPath = path;
	myAnimatorFilePtr = Firefly::ResourceCache::GetAsset<Firefly::Animator>(path);
}

void AnimatorWindow::DoLoad()
{
	mySelectedTransitionID = 0;
	mySelectedStateID = 0;
	std::filesystem::path path = std::filesystem::relative(EditorUtils::GetOpenFilePath("Animator Controller (*.animator)\0*.animator\0"));
	if (path == "")
		return;


	SetAnimator(path);

}

void AnimatorWindow::SetAnimator(std::filesystem::path aPath)
{
	myAnimatorFilePtr = Firefly::ResourceCache::GetAsset<Firefly::Animator>(aPath, true);
	myCurrentPath = aPath;

	myAnimator = std::make_shared<Firefly::Animator>(*myAnimatorFilePtr);
	mySelectedTransitionID = 0;
	mySelectedStateID = 0;
	myCurrentLayerIndex = 0;
	myLayerInfoList.clear();

	for (auto& layer : myAnimator->GetLayers())
	{
		myLayerInfoList.push_back({ 0,0 });
		for (auto& statePair : layer.myStates)
		{
			auto& state = statePair.second;
			if (state.AnimationPath != "")
			{
				Firefly::ResourceCache::GetAsset<Firefly::Animation>(state.AnimationPath);
			}
		}
	}
	myDraggingStateFlag = false;

	myCameraMatrix = Utils::Matrix3f();
}
