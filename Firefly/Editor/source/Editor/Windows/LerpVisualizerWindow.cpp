#include "EditorPch.h"
#include "LerpVisualizerWindow.h"

#include <Firefly/Event/ApplicationEvents.h>

#include "Firefly/Application/Application.h"
#include "WindowRegistry.h"

REGISTER_WINDOW(LerpVisualizerWindow);

LerpVisualizerWindow::LerpVisualizerWindow() : EditorWindow("Lerp Visualizer Window")
{
	
}

void LerpVisualizerWindow::OnImGui()
{
	ImGui::DragInt("Line Resolution##", &myLerpVisualRes, 1, 1, 99999);
	ImGui::Combo("##CurrentLerp", (int*)(&mySelectedLerp), LerpsCharCombo);

	if (mySelectedLerp == Utils::LerpType::EaseIn || mySelectedLerp == Utils::LerpType::EaseOut || mySelectedLerp == Utils::LerpType::EaseInOut)
	{
		ImGui::DragFloat("Smoothing Power", &myPower, 0.05f, 0.001f, 99999.0f);
	}
	else if (mySelectedLerp == Utils::LerpType::Parabola)
	{
		ImGui::DragFloat("Parabola Squish", &mySquish, 0.1f, 0.001f, 99999.0f);
	}

	if (mySelectedLerp == Utils::LerpType::BounceCustom)
	{
		ImGui::DragInt("Bounce Count", &myBounceCount);
		ImGui::DragFloat("Height Loss", &myPower);
	}

	float windowSize = Utils::Min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

	ImVec2 pos = 
	{
		ImGui::GetCursorScreenPos().x,
		ImGui::GetCursorScreenPos().y + windowSize
	};

	ImGui::BeginChild("##LerpChildWindow", { windowSize, windowSize }, true);

	for (int i = 0; i < myLerpVisualRes; ++i)
	{
		float t1 = (float)i / (float)myLerpVisualRes;
		float lerped1 = LerpByType(mySelectedLerp, 0.0f, windowSize, t1, mySelectedLerp == Utils::LerpType::Parabola ? mySquish : myPower, true, myBounceCount);

		float t2 = ((float)i + 1) / (float)myLerpVisualRes;
		float lerped2 = LerpByType(mySelectedLerp, 0.0f, windowSize, t2, mySelectedLerp == Utils::LerpType::Parabola ? mySquish : myPower, true, myBounceCount);

		ImVec2 start(pos.x + t1 * windowSize, pos.y - lerped1);
		ImVec2 end(pos.x + t2 * windowSize, pos.y - lerped2);

		ImGui::GetWindowDrawList()->AddLine(start, end, IM_COL32(255, 0, 0, 255), 5);
	}

	ImGui::EndChild();
}