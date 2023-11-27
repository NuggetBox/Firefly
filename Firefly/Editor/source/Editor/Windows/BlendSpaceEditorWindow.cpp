#include "EditorPch.h"
#include "BlendSpaceEditorWindow.h"

#include "Editor/Windows/WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"

#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Animations/BlendSpace.h"

#include "Editor/Utilities/ImGuiUtils.h"

#include "Utils/Timer.h"

#include "imgui/imgui_internal.h"

#include "Editor/EditorCamera.h"

#include "Firefly/Asset/Mesh/AnimatedMesh.h"

#include "Utils/InputHandler.h"

REGISTER_WINDOW(BlendSpaceEditorWindow);

BlendSpaceEditorWindow::BlendSpaceEditorWindow()
	: EditorWindow("Blendspace Editor")
{

	myBlendspaceTypesStrings[0] = "1D";
	myBlendspaceTypesStrings[1] = "2D";

	myPreviewRenderSceneID = Firefly::Renderer::InitializeScene();
	myPreviewMeshMaterials.push_back(Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Default"));

	myCubeMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("Cube");

	myEditorCamera = CreateRef<EditorCamera>();
	myEditorCamera->Initialize(Firefly::CameraInfo());

	myPlayButtonTexture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_play.dds");
	myPauseButtonTexture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_pause.dds");

	myTime = 0;
	myMaxTime = 3;
	myPreviewGridPosition = { 0,0 };

	myCubeTransform.SetScale(100, 1, 100);
	myCubeTransform.SetYPosition(-50);

	myWindowFlags |= ImGuiWindowFlags_MenuBar;

}

void BlendSpaceEditorWindow::OnImGui()
{
	if (!myBlendSpace)
	{
		ImGui::Text("No BlendSpace loaded, please load one using the content browser by double clicking it or dropping in in this window.");
		if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".blend"))
		{
			std::string path = (const char*)payload->Data;
			SetBlendSpace(path);
			ImGui::SetWindowFocus();
		}
		return;
	}
	
	if (!myBlendSpace->IsLoaded())
	{
		ImGui::Text("BlendSpace is loading...");
		return;
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				Save();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	myIsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	auto dockspaceID = ImGui::GetID("BlendspaceWindowDockspace");

	if (!ImGui::DockBuilderGetNode(dockspaceID))
	{
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());

		auto dockMainID = dockspaceID;
		auto dockLeftID = ImGui::DockBuilderSplitNode(dockMainID,
			ImGuiDir_Left, 0.3f, nullptr, &dockMainID);
		auto dockBottomID = ImGui::DockBuilderSplitNode(dockMainID,
			ImGuiDir_Down, 0.4f, nullptr, &dockMainID);

		ImGui::DockBuilderDockWindow("Viewport", dockMainID);
		ImGui::DockBuilderDockWindow("Blendspace View", dockBottomID);
		ImGui::DockBuilderDockWindow("Inspector", dockLeftID);

		ImGui::DockBuilderFinish(dockspaceID);
	}
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar);


	//zzzz
	{
		ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
		myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		ImGui::Combo("##SelectBlendspaceType",
			reinterpret_cast<int*>(&myBlendSpace->myBlendspaceType), myBlendspaceTypesStrings, 2);
		DrawInspector();
		ImGui::End();
	}

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawViewport();
	ImGui::End();

	ImGui::Begin("Blendspace View", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	ImGui::BeginChild("BlendspaceViewSettings", { 0,myBlendSpaceViewSettingsHeight });
	DrawBlendspaceViewSettings();
	ImGui::EndChild();

	ImGui::BeginChild("BlendspaceView", { 0,ImGui::GetContentRegionAvail().y - myTimelineHeight });
	DrawBlendspaceView();
	ImGui::EndChild();

	ImGui::BeginChild("Timeline", { 0,0 }, false);
	DrawTimeline();
	ImGui::EndChild();
	ImGui::End();

	if (myIsFirstFrame)
	{
		myIsFirstFrame = false;
		ImGui::DockBuilderRemoveNode(ImGui::GetID("BlendspaceWindowDockspace"));
	}

	if (myIsPlaying && !myIsDraggingTimeline)
	{
		myTime += Utils::Timer::GetDeltaTime();
		auto duration = myBlendSpace->GetDuration(myPreviewGridPosition);
		if (myTime >= duration)
		{
			myTime -= duration;
		}
	}
}

void BlendSpaceEditorWindow::SetBlendSpace(const std::filesystem::path& aPath)
{
	auto blendSpace = Firefly::ResourceCache::GetAsset<Firefly::BlendSpace>(aPath.string(), true);
	if (!blendSpace)
	{
		LOGERROR("BlendSpaceEditorWindow::SetBlendSpace: Failed to load blendspace: {}", aPath.string());
		ImGuiUtils::NotifyError("Failed to load blendspace: {}", aPath.string());
		return;
	}
	SetBlendSpace(blendSpace);
}

void BlendSpaceEditorWindow::SetBlendSpace(const Ref<Firefly::BlendSpace>& aBlendSpace)
{
	myBlendSpace = CreateRef<Firefly::BlendSpace>(*aBlendSpace);
}

void BlendSpaceEditorWindow::Save()
{
	auto fileStatus = std::filesystem::status(myBlendSpace->GetPath());

	bool hasWritePermission = (fileStatus.permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
	if (!hasWritePermission)
	{
		ImGuiUtils::NotifyErrorCustom(myBaseWindowPos, myBaseWindowSize, "No write permission for file: \n\"{}\". \nMake sure the file is checked out in perforce.", myBlendSpace->GetPath().string().c_str());
		return;
	}
	myBlendSpace->SaveTo(myBlendSpace->GetPath());

	ImGuiUtils::NotifySuccessCustom(myBaseWindowPos, myBaseWindowSize, "Saved blendspace to path: \n\"{}\"", myBlendSpace->GetPath().string().c_str());

	return;
}

void BlendSpaceEditorWindow::DrawInspector()
{
	if (ImGui::CollapsingHeader("AxisSettings"))
	{
		ImGui::Indent();

		if (ImGui::CollapsingHeader("HorizontalAxis"))
		{
			ImGui::Indent();
			ImGuiUtils::BeginParameters();
			ImGuiUtils::Parameter("Name", myBlendSpace->myHorizontalAxis.Name);
			ImGuiUtils::Parameter("Min", myBlendSpace->myHorizontalAxis.Min);
			ImGuiUtils::Parameter("Max", myBlendSpace->myHorizontalAxis.Max);
			ImGuiUtils::EndParameters();
			ImGui::Unindent();
		}
		if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::TwoDimensional)
		{
			if (ImGui::CollapsingHeader("VerticalAxis"))
			{
				ImGui::Indent();
				ImGuiUtils::BeginParameters();
				ImGuiUtils::Parameter("Name", myBlendSpace->myVerticalAxis.Name);
				ImGuiUtils::Parameter("Min", myBlendSpace->myVerticalAxis.Min);
				ImGuiUtils::Parameter("Max", myBlendSpace->myVerticalAxis.Max);
				ImGuiUtils::EndParameters();
				ImGui::Unindent();
			}
		}

		ImGui::Unindent();
	}


}

void BlendSpaceEditorWindow::DrawBlendspaceViewSettings()
{
	if (mySelectedPointIndex != -1)
	{
		ImGui::PushItemWidth(std::max(20.f, std::min(60.f, ImGui::GetWindowSize().x / 4)));
		if (ImGui::DragFloat("X", &myBlendSpace->myEntries[mySelectedPointIndex].HorizontalAxisPosition, 0.1f,
			myBlendSpace->myHorizontalAxis.Min, myBlendSpace->myHorizontalAxis.Max, "%.2f"))
		{
			myBlendSpace->myEntries[mySelectedPointIndex].HorizontalAxisPosition =
				std::clamp(myBlendSpace->myEntries[mySelectedPointIndex].HorizontalAxisPosition,
					myBlendSpace->myHorizontalAxis.Min, myBlendSpace->myHorizontalAxis.Max);
		}
		ImGui::SameLine();
		if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::TwoDimensional)
		{
			if (ImGui::DragFloat("Y", &myBlendSpace->myEntries[mySelectedPointIndex].VerticalAxisPosition, 0.1f,
				myBlendSpace->myVerticalAxis.Min, myBlendSpace->myVerticalAxis.Max, "%.2f"))
			{
				myBlendSpace->myEntries[mySelectedPointIndex].VerticalAxisPosition =
					std::clamp(myBlendSpace->myEntries[mySelectedPointIndex].VerticalAxisPosition,
						myBlendSpace->myVerticalAxis.Min, myBlendSpace->myVerticalAxis.Max);
			}
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("Speed", &myBlendSpace->myEntries[mySelectedPointIndex].Speed, 0.1f))
		{
			myBlendSpace->myEntries[mySelectedPointIndex].Speed =
				std::clamp(myBlendSpace->myEntries[mySelectedPointIndex].Speed, 0.01f, 100.f);
		}
		ImGui::PopItemWidth();

		ImGui::SameLine();
		//name of entry
		ImGui::Text("Name: ");
		ImGui::SameLine();
		ImGui::Text(myBlendSpace->myEntries[mySelectedPointIndex].Animation->GetPath().stem().string().c_str());
	}
}

void BlendSpaceEditorWindow::DrawBlendspaceView()
{
	auto drawList = ImGui::GetWindowDrawList();
	auto windowPos = ImGui::GetWindowPos();
	auto windowSize = ImGui::GetWindowSize();

	const auto verticalAxisInfoTextSize = ImGui::CalcTextSize(myBlendSpace->myVerticalAxis.Name.c_str());
	const float verticalAxisInfoWidth = verticalAxisInfoTextSize.x + 15;

	const auto horizontalAxisInfoTextSize = ImGui::CalcTextSize(myBlendSpace->myHorizontalAxis.Name.c_str());
	const float horizontalAxisInfoHeight = horizontalAxisInfoTextSize.y + 15;


	//vertical Axis info
	if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::TwoDimensional)
	{
		auto verticalAxisInfoPos = ImGui::GetWindowPos();
		Utils::Vector2f verticalAxisInfoSize = { verticalAxisInfoWidth, windowSize.y - horizontalAxisInfoHeight };

		const Utils::Vector2f verticalAxisInfoCenter = {
			verticalAxisInfoPos.x + verticalAxisInfoSize.x / 2,
			verticalAxisInfoPos.y + verticalAxisInfoSize.y / 2 };

		drawList->AddText({ verticalAxisInfoCenter.x - verticalAxisInfoTextSize.x / 2.f, verticalAxisInfoCenter.y - verticalAxisInfoTextSize.y / 2.f },
			ImGui::GetColorU32(ImGuiCol_Text), myBlendSpace->myVerticalAxis.Name.c_str());

		const float arrowLenght = Utils::Max(10.f, verticalAxisInfoSize.y / 8.f);

		//up arrow
		const Utils::Vector2f upArrowStart = { verticalAxisInfoCenter.x, verticalAxisInfoCenter.y - verticalAxisInfoTextSize.y - 10.f };
		ImGuiUtils::DrawlistAddLineArrow(drawList,
			upArrowStart,
			{ upArrowStart.x, upArrowStart.y - arrowLenght },
			ImGui::GetColorU32({ 1,1,1,1 }), 1, 10);

		//down arrow
		const Utils::Vector2f downArrowStart = { verticalAxisInfoCenter.x, verticalAxisInfoCenter.y + verticalAxisInfoTextSize.y + 10.f };
		ImGuiUtils::DrawlistAddLineArrow(drawList,
			downArrowStart,
			{ downArrowStart.x, downArrowStart.y + arrowLenght },
			ImGui::GetColorU32({ 1,1,1,1 }), 1, 10);

		//vertical max value
		auto verticalMaxValueText = std::format("{:.3f}", myBlendSpace->myVerticalAxis.Max);
		verticalMaxValueText.erase(verticalMaxValueText.find_last_not_of('0') + 1, std::string::npos);
		if (verticalMaxValueText.back() == '.')
		{
			verticalMaxValueText.pop_back();
		}
		const auto verticalMaxValueTextSize = ImGui::CalcTextSize(verticalMaxValueText.c_str());
		const ImVec2 verticalMaxValueTextPos = { verticalAxisInfoPos.x, verticalAxisInfoPos.y + verticalMaxValueTextSize.y };
		drawList->AddText(verticalMaxValueTextPos,
			ImGui::GetColorU32(ImGuiCol_Text), verticalMaxValueText.c_str());
		//

		//vertical min value
		auto verticalMinValueText = std::format("{:.3f}", myBlendSpace->myVerticalAxis.Min);
		verticalMinValueText.erase(verticalMinValueText.find_last_not_of('0') + 1, std::string::npos);
		if (verticalMinValueText.back() == '.')
		{
			verticalMinValueText.pop_back();
		}
		const auto verticalMinValueTextSize = ImGui::CalcTextSize(verticalMinValueText.c_str());
		const ImVec2 verticalMinValueTextPos = { verticalAxisInfoPos.x,
			verticalAxisInfoPos.y + verticalAxisInfoSize.y - verticalMinValueTextSize.y };
		drawList->AddText(verticalMinValueTextPos,
			ImGui::GetColorU32(ImGuiCol_Text), verticalMinValueText.c_str());
		//
	}

	//horizontal axis info
	{
		const Utils::Vector2f horizontalAxisInfoPos =
		{ windowPos.x + verticalAxisInfoWidth,
		windowPos.y + windowSize.y - horizontalAxisInfoHeight };

		const Utils::Vector2f horizontalAxisInfoSize =
		{ windowSize.x - verticalAxisInfoWidth, horizontalAxisInfoHeight };

		const Utils::Vector2f horizontalAxisInfoCenter =
		{ horizontalAxisInfoPos.x + horizontalAxisInfoSize.x / 2.f,
			horizontalAxisInfoPos.y + horizontalAxisInfoSize.y / 2.f };

		drawList->AddText(
			{ horizontalAxisInfoCenter.x - horizontalAxisInfoTextSize.x / 2.f,
				horizontalAxisInfoCenter.y - horizontalAxisInfoTextSize.y / 2.f },
			ImGui::GetColorU32({ 1,1,1, 1 }), myBlendSpace->myHorizontalAxis.Name.c_str());

		const float arrowLenght = Utils::Max(10.f, horizontalAxisInfoSize.x / 8.f);

		//left arrow
		const Utils::Vector2f leftArrowStart =
		{ horizontalAxisInfoCenter.x - horizontalAxisInfoTextSize.x - 10.f, horizontalAxisInfoCenter.y };
		ImGuiUtils::DrawlistAddLineArrow(drawList,
			leftArrowStart,
			{ leftArrowStart.x - arrowLenght, leftArrowStart.y },
			ImGui::GetColorU32({ 1,1,1, 1 }), 1, 10);
		//

		//right arrow
		const Utils::Vector2f rightArrowStart =
		{ horizontalAxisInfoCenter.x + horizontalAxisInfoTextSize.x + 10.f, horizontalAxisInfoCenter.y };
		ImGuiUtils::DrawlistAddLineArrow(drawList,
			rightArrowStart,
			{ rightArrowStart.x + arrowLenght, rightArrowStart.y },
			ImGui::GetColorU32({ 1,1,1, 1 }), 1, 10);
		//

		//horizontal min value
		auto horizontalMinValueText = std::format("{:.3f}", myBlendSpace->myHorizontalAxis.Min);
		horizontalMinValueText.erase(horizontalMinValueText.find_last_not_of('0') + 1, std::string::npos);
		if (horizontalMinValueText.back() == '.')
		{
			horizontalMinValueText.pop_back();
		}
		const auto horizontalMinValueTextSize = ImGui::CalcTextSize(horizontalMinValueText.c_str());
		const ImVec2 horizontalMinValueTextPos = { horizontalAxisInfoPos.x,
			horizontalAxisInfoCenter.y - horizontalMinValueTextSize.y / 2.f };
		drawList->AddText(horizontalMinValueTextPos,
			ImGui::GetColorU32(ImGuiCol_Text), horizontalMinValueText.c_str());
		//

		//horizontal max value
		auto horizontalMaxValueText = std::format("{:.3f}", myBlendSpace->myHorizontalAxis.Max);
		horizontalMaxValueText.erase(horizontalMaxValueText.find_last_not_of('0') + 1, std::string::npos);
		if (horizontalMaxValueText.back() == '.')
		{
			horizontalMaxValueText.pop_back();
		}
		const auto horizontalMaxValueTextSize = ImGui::CalcTextSize(horizontalMaxValueText.c_str());
		const ImVec2 horizontalMaxValueTextPos = { horizontalAxisInfoPos.x + horizontalAxisInfoSize.x - horizontalMaxValueTextSize.x,
			horizontalAxisInfoCenter.y - horizontalMaxValueTextSize.y / 2.f };
		drawList->AddText(horizontalMaxValueTextPos,
			ImGui::GetColorU32(ImGuiCol_Text), horizontalMaxValueText.c_str());
		//

	}

	myBlendSpaceGridPos =
	{ windowPos.x + verticalAxisInfoWidth,
		windowPos.y };
	myBlendSpaceGridSize =
	{ windowSize.x - verticalAxisInfoWidth,
		windowSize.y - horizontalAxisInfoHeight };

	//background
	drawList->AddRectFilled({ myBlendSpaceGridPos.x, myBlendSpaceGridPos.y },
		{ myBlendSpaceGridPos.x + myBlendSpaceGridSize.x, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y },
		ImGui::GetColorU32({ 0.1f, 0.1f, 0.1f, 1.f }));

	//Draw horizontal line through center
	drawList->AddLine({ myBlendSpaceGridPos.x, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y / 2.f },
		{ myBlendSpaceGridPos.x + myBlendSpaceGridSize.x, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y / 2.f }, ImGui::GetColorU32({ 0.2f,0.2f,0.2f,1 }));

	//Draw vertical line through center
	drawList->AddLine({ myBlendSpaceGridPos.x + myBlendSpaceGridSize.x / 2.f, myBlendSpaceGridPos.y },
		{ myBlendSpaceGridPos.x + myBlendSpaceGridSize.x / 2.f, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y }, ImGui::GetColorU32({ 0.2f,0.2f,0.2f,1 }));

	//outline 
	drawList->AddRect({ myBlendSpaceGridPos.x ,myBlendSpaceGridPos.y },
		{ myBlendSpaceGridPos.x + myBlendSpaceGridSize.x, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y },
		ImGui::GetColorU32({ 0.05f, 0.05f, 0.05f, 1 }), 0, 0, 2);

	const float blendSpaceValuesWidth = myBlendSpace->myHorizontalAxis.Max - myBlendSpace->myHorizontalAxis.Min;
	const float blendSpaceValuesHeight = myBlendSpace->myVerticalAxis.Max - myBlendSpace->myVerticalAxis.Min;

	std::array<int, 3> triangle;
	std::array<float, 3> weights;
	if (myBlendSpace->myTriangles.size() > 0)
	{
		auto triangleIndexOptional = myBlendSpace->GetTriangleThatContainsPoint(myPreviewGridPosition);
		int triangleIndex = 0;
		auto posToCheck = myPreviewGridPosition;
		if (triangleIndexOptional.has_value())
		{
			triangleIndex = triangleIndexOptional.value();
		}
		else
		{
			posToCheck = myBlendSpace->TranslatePositionIntoNearestTriangle(myPreviewGridPosition);
			triangleIndexOptional = myBlendSpace->GetTriangleThatContainsPoint(posToCheck);
			if (triangleIndexOptional.has_value())
			{
				triangleIndex = triangleIndexOptional.value();
			}

		}

		triangle = myBlendSpace->myTriangles[triangleIndex];
		weights = myBlendSpace->CalculateWeightsInTriangle(posToCheck,
			myBlendSpace->myEntries[triangle[0]], myBlendSpace->myEntries[triangle[1]], myBlendSpace->myEntries[triangle[2]]);
	}


	//Draw animation entries
	for (int i = 0; i < myBlendSpace->myEntries.size(); i++)
	{
		auto& entry = myBlendSpace->myEntries[i];

		//find screen position of entry
			//find preview screen pos 
		Utils::Vector2f screenPos = GridToScreenPosition({ entry.HorizontalAxisPosition, entry.VerticalAxisPosition });

		int indexInTriangle = -1;
		if (myBlendSpace->myTriangles.size() > 0)
		{
			if (i == triangle[0])
			{
				indexInTriangle = 0;
			}
			else if (i == triangle[1])
			{
				indexInTriangle = 1;
			}
			else if (i == triangle[2])
			{
				indexInTriangle = 2;
			}
		}

		//draw diamond
		const ImVec2 topPos = { screenPos.x, screenPos.y - myEntryDiamondRadius };
		const ImVec2 bottomPos = { screenPos.x, screenPos.y + myEntryDiamondRadius };
		const ImVec2 leftPos = { screenPos.x - myEntryDiamondRadius, screenPos.y };
		const ImVec2 rightPos = { screenPos.x + myEntryDiamondRadius, screenPos.y };




		//logic for dragging a point
		ImGui::ItemAdd({ screenPos.x - myEntryDiamondRadius, screenPos.y - myEntryDiamondRadius,
			screenPos.x + myEntryDiamondRadius, screenPos.y + myEntryDiamondRadius },
			ImGui::GetID(std::format("Entry{}", i).c_str()));
		ImVec4 color = { 0.9f,0.9f,0.9f,1 };
		if (i == mySelectedPointIndex)
		{
			color = { 0.75f,0.75f,0.75f,1 };
		}
		if (ImGui::IsItemHovered())
		{
			color = { color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, color.w };
			if (ImGui::IsMouseClicked(0) && !myIsDraggingPoint)
			{
				myIsDraggingPoint = true;
				mySelectedPointIndex = i;
				myStartDragMousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup(("BlendspaceWindowRightClickEntry" + std::to_string(i)).c_str());
			}
		}
		if (ImGui::BeginPopup(("BlendspaceWindowRightClickEntry" + std::to_string(i)).c_str()))
		{
			if (ImGui::MenuItem("Remove"))
			{
				myBlendSpace->myEntries.erase(myBlendSpace->myEntries.begin() + i);
				mySelectedPointIndex = -1;
				Triangulate();
				i--;
				ImGui::EndPopup();
				continue;
			}
			ImGui::EndPopup();
		}
		drawList->AddQuadFilled(topPos, rightPos, bottomPos, leftPos, ImGui::GetColorU32(color));

		if (myBlendSpace->myTriangles.size() > 0)
		{
			//draw a circle depending on the weight of the entry
			float weight = 0.f;
			if (indexInTriangle != -1)
			{
				weight = weights[indexInTriangle];
				const float radius = 40.f * weight;
				if (weight > 0.001f)
				{
					drawList->AddCircleFilled({ screenPos.x, screenPos.y }, radius, ImGui::GetColorU32({ 0.2f, 0.15f, 0.3f, 0.5f }));
				}
			}
		}

	}

	if (ImGui::IsMouseReleased(0))
	{
		myIsDraggingPoint = false;
	}

	if (myIsDraggingPoint)
	{
		auto newGridPos =
			ScreenToGridPosition({ myStartDragMousePos.x + ImGui::GetMouseDragDelta().x, myStartDragMousePos.y + ImGui::GetMouseDragDelta().y });
		myBlendSpace->myEntries[mySelectedPointIndex].HorizontalAxisPosition = newGridPos.x;
		myBlendSpace->myEntries[mySelectedPointIndex].VerticalAxisPosition = newGridPos.y;
		Triangulate();
	}

	//draw Triangulation
	for (auto& triangle : myBlendSpace->myTriangles)
	{
		const auto& aIndex = triangle[0];
		const auto& bIndex = triangle[1];
		const auto& cIndex = triangle[2];

		const Utils::Vector2f pointA = { myBlendSpace->myEntries[aIndex].HorizontalAxisPosition, myBlendSpace->myEntries[aIndex].VerticalAxisPosition };
		const Utils::Vector2f pointB = { myBlendSpace->myEntries[bIndex].HorizontalAxisPosition, myBlendSpace->myEntries[bIndex].VerticalAxisPosition };
		const Utils::Vector2f pointC = { myBlendSpace->myEntries[cIndex].HorizontalAxisPosition, myBlendSpace->myEntries[cIndex].VerticalAxisPosition };

		const auto aScreenPos = GridToScreenPosition(pointA);
		const auto bScreenPos = GridToScreenPosition(pointB);
		const auto cScreenPos = GridToScreenPosition(pointC);

		drawList->AddTriangle({ aScreenPos.x, aScreenPos.y },
			{ bScreenPos.x, bScreenPos.y },
			{ cScreenPos.x, cScreenPos.y },
			ImGui::GetColorU32({ 1.f,1.f,1.f,1 }), 1);

		drawList->AddTriangleFilled({ aScreenPos.x, aScreenPos.y },
			{ bScreenPos.x, bScreenPos.y },
			{ cScreenPos.x, cScreenPos.y },
			ImGui::GetColorU32({ 0.2f,0.2f,1.f,0.1f }));


		auto& a = aScreenPos;
		auto& b = bScreenPos;
		auto& c = cScreenPos;


		Utils::Vector2f circumcenter = myBlendSpace->GetTriangleCircumferenceCenter(a, b, c);

		//calculate the circumradius
		if (circumcenter.x != FLT_MAX && circumcenter.y != FLT_MAX)
		{
			float circumradius = (a - circumcenter).Length();
			//drawList->AddCircle({ circumcenter.x,circumcenter.y }, circumradius, ImGui::GetColorU32({ 1,0,0,1 }));
		}
	}


	//select preview pos by holding ctrl
	if (myIsFocused && Utils::InputHandler::GetKeyHeld(VK_CONTROL))
	{
		myPreviewGridPosition = GetMouseGridPosition();
	}
	//find preview screen pos 
	Utils::Vector2f previewScreenPos = GridToScreenPosition(myPreviewGridPosition);

	//draw a x at preview pos
	drawList->AddLine({ previewScreenPos.x - 5, previewScreenPos.y - 5 },
		{ previewScreenPos.x + 5, previewScreenPos.y + 5 }, ImGui::GetColorU32({ 1,0,0,1 }));
	drawList->AddLine({ previewScreenPos.x - 5, previewScreenPos.y + 5 },
		{ previewScreenPos.x + 5, previewScreenPos.y - 5 }, ImGui::GetColorU32({ 1,0,0,1 }));

	Utils::Vector2f previewActualPos = GridToScreenPosition(myBlendSpace->TranslatePositionIntoNearestTriangle(myPreviewGridPosition));

	//draw a x at preview pos
	drawList->AddLine({ previewActualPos.x - 5, previewActualPos.y - 5 },
		{ previewActualPos.x + 5, previewActualPos.y + 5 }, ImGui::GetColorU32({ 0,1,0,1 }));
	drawList->AddLine({ previewActualPos.x - 5, previewActualPos.y + 5 },
		{ previewActualPos.x + 5, previewActualPos.y - 5 }, ImGui::GetColorU32({ 0,1,0,1 }));

	//accept animations to add entry
	if (ImGui::BeginDragDropTargetCustom(
		{ myBlendSpaceGridPos.x, myBlendSpaceGridPos.y,
		myBlendSpaceGridPos.x + myBlendSpaceGridSize.x, myBlendSpaceGridPos.y + myBlendSpaceGridSize.y },
		ImGui::GetID("BlendSpaceViewDragDropTarget")))
	{
		auto payload = ImGui::GetDragDropPayload();
		std::filesystem::path path = (const char*)payload->Data;
		if (path.extension() == ".anim")
		{
			if (ImGui::AcceptDragDropPayload("FILE"))
			{
				Firefly::BlendSpaceEntry entry;
				entry.Animation = Firefly::ResourceCache::GetAsset<Firefly::Animation>(path);
				const auto mouseGridPosition = GetMouseGridPosition();
				entry.HorizontalAxisPosition = mouseGridPosition.x;
				entry.VerticalAxisPosition = mouseGridPosition.y;

				entry.Speed = 1;
				myBlendSpace->myEntries.push_back(entry);

				Triangulate();
			}

			auto screenPos = ImGui::GetMousePos();

			const ImVec2 topPos = { screenPos.x, screenPos.y - myEntryDiamondRadius };
			const ImVec2 bottomPos = { screenPos.x, screenPos.y + myEntryDiamondRadius };
			const ImVec2 leftPos = { screenPos.x - myEntryDiamondRadius, screenPos.y };
			const ImVec2 rightPos = { screenPos.x + myEntryDiamondRadius, screenPos.y };
			drawList->AddQuadFilled(topPos, rightPos, bottomPos, leftPos, ImGui::GetColorU32({ 0.5f,0.9f,0.6f,0.8f }));
			drawList;
		}
	}

}

void BlendSpaceEditorWindow::DrawViewport()
{
	auto windowPos = ImGui::GetWindowPos();
	auto contentRegionAvail = ImGui::GetContentRegionAvail();

	if (contentRegionAvail.x <= 1 || contentRegionAvail.y <= 1)
	{
		return;
	}
	Firefly::Renderer::BeginScene(myPreviewRenderSceneID);
	myEditorCamera->Update();
	myEditorCamera->SetActiveCamera();
	//Environment Light
	Firefly::EnvironmentData environmentData{};
	environmentData.EnvironmentMap = nullptr;
	environmentData.Intensity = 1;
	Firefly::Renderer::Submit(environmentData);
	//

	Firefly::DirLightPacket pack{};
	pack.Direction = { -0.71, 0.71, -0.71, 1 };
	pack.ColorAndIntensity = { 1,1,1,1 };
	pack.dirLightInfo.x = 0;
	Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res1024);

	////Directional Light
	//Firefly::DirLightPacket directionalLightPacket;
	//directionalLightPacket.ColorAndIntensity = { 1, 1, 1, 1 }; // why not in separate variables
	//directionalLightPacket.Direction = { 1, 1, 0 , 0 };// why 4?
	//directionalLightPacket.Direction.Normalize();

	////why should i have to do this
	//auto& cameraTransform = Firefly::Renderer::GetActiveCamera()->GetTransform();
	//float offset = 4500.f;
	//directionalLightPacket.ViewMatrix = Utils::Matrix4f::CreateLookAt((cameraTransform.GetPosition() + Utils::Vector3f(0, 0, 1) * offset) + Utils::Vec4ToVec3(directionalLightPacket.Direction), (cameraTransform.GetPosition() + Utils::Vector3f(0, 0, 1) * offset) - Utils::Vec4ToVec3(directionalLightPacket.Direction), { 0,1,0 });
	//directionalLightPacket.ProjMatrix = Utils::Matrix4f::CreateProjectionMatrixOrthographic(10000, 10000, 1.f, 12000.f);
	//directionalLightPacket.dirLightInfo.x = static_cast<uint32_t>(true);
	//directionalLightPacket.dirLightInfo.y = static_cast<uint32_t>(true);
	//Firefly::Renderer::Submit(directionalLightPacket, Firefly::ShadowResolutions::res4098);
	////
	Firefly::PostProcessInfo postProcessInfo{};
	postProcessInfo.Enable = true;
	postProcessInfo.Data.Padding.z = 0.1f;

	Firefly::Renderer::Submit(postProcessInfo);



	myEditorCamera->SetViewportRect(
		{ static_cast<int>(windowPos.x), static_cast<int>(windowPos.y),
		static_cast<int>(windowPos.x + contentRegionAvail.x),
		static_cast<int>(windowPos.y + contentRegionAvail.y) });


	//Resize to content region
	if (Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Width) - contentRegionAvail.x) > 1.0f
		|| Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Height) - contentRegionAvail.y) > 1.0f)
	{
		auto newSize = Utils::Vector2<uint32_t>(contentRegionAvail.x, contentRegionAvail.y);
		Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->Resize(newSize);
		myEditorCamera->GetCamera()->SetSizeX(static_cast<float>(newSize.x));
		myEditorCamera->GetCamera()->SetSizeY(static_cast<float>(newSize.y));
	}

	if (myBlendSpace->myMesh)
	{
		std::vector<Utils::Matrix4f> matrices;
		if (myBlendSpace->myEntries.size() > 0)
		{
			std::optional<Firefly::Frame> frameOpt;
			if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::TwoDimensional)
			{
				frameOpt = myBlendSpace->Sample2D(myTime, myPreviewGridPosition);

			}
			else
			{
				frameOpt = myBlendSpace->Sample1D(myTime, myPreviewGridPosition.x);
			}

			if (frameOpt.has_value())
			{
				frameOpt.value().CalculateTransforms(myBlendSpace->myMesh->GetSkeleton(), matrices);
			}
		}
		for (auto& submesh : myBlendSpace->myMesh->GetSubMeshes())
		{
			Firefly::MeshSubmitInfo cmd(Utils::Matrix4f(), true);
			cmd.Mesh = &submesh;
			cmd.SetBoneTransforms(matrices);
			if (!myPreviewMeshMaterials.empty())
			{
				cmd.Material = myPreviewMeshMaterials[0];
			}
			Firefly::Renderer::Submit(cmd);
		}

	}

	if (myCubeMesh)
	{
		//Firefly::ModelInfo cmd(myCubeMesh, myCubeTransform.GetMatrix());
		//Firefly::Renderer::Submit(cmd);
	}
	Firefly::Renderer::EndScene();
	auto frame = Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID);
	ImGui::Image(frame->GetColorAttachment(0).Get(), { contentRegionAvail.x , contentRegionAvail.y });
}

void BlendSpaceEditorWindow::DrawTimeline()
{
	auto drawList = ImGui::GetWindowDrawList();
	const auto timelineChildPos = ImGui::GetWindowPos();
	const auto timelineChildSize = ImGui::GetContentRegionAvail();

	const auto timelineWidth = timelineChildSize.x - (timelineChildSize.y * myTimelineButtonCount);
	const auto timelineRectMin = timelineChildPos;
	const auto timelineRectMax = ImVec2(timelineChildPos.x + timelineWidth, timelineChildPos.y + timelineChildSize.y);
	const float timelineOutlineThickness = 5;
	drawList->AddRect(timelineRectMin, timelineRectMax, ImGui::GetColorU32(ImGuiCol_Separator), 3, 0, timelineOutlineThickness);

	ImGui::SetCursorScreenPos({ timelineRectMax.x , timelineRectMin.y });
	auto playButtonTexture = myIsPlaying ? myPauseButtonTexture : myPlayButtonTexture;
	if (ImGui::ImageButton(playButtonTexture->GetSRV().Get(), ImVec2(40, 40)))
	{
		myIsPlaying = !myIsPlaying;
	}

	auto duration = myBlendSpace->GetDuration(myPreviewGridPosition);
	//draw rect with drawlist instead
	ImGui::PushClipRect(timelineRectMin, timelineRectMax, false);
	drawList->AddRectFilled({ timelineRectMin.x + myTime / duration * timelineWidth, timelineRectMin.y + timelineOutlineThickness },
		{ timelineRectMin.x + myTime / duration * timelineWidth + 20, timelineRectMax.y - timelineOutlineThickness },
		ImGui::GetColorU32({ 146.f / 255.f,63.f / 255.f,63.f / 255.f,1 }));
	ImGui::PopClipRect();

	if (ImGui::IsMouseHoveringRect(timelineRectMin, timelineRectMax))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			myIsDraggingTimeline = true;
		}
	}
	if (myIsDraggingTimeline)
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			myIsDraggingTimeline = false;
		}
		else
		{
			//find where on the timeline the mouse is
			const auto mousePos = ImGui::GetMousePos();
			const auto mousePosOnTimeline = mousePos.x - timelineRectMin.x;
			const auto clampedPos = std::clamp(mousePosOnTimeline, 0.f, timelineWidth);

			myTime = clampedPos / timelineWidth * duration;

		}

	}

}

void BlendSpaceEditorWindow::Triangulate()
{
	if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::TwoDimensional)
	{
		myBlendSpace->Triangulate2D();
	}
}

Utils::Vector2f BlendSpaceEditorWindow::GetMouseGridPosition()
{
	const Utils::Vector2f mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
	auto mousePosInBlendspace = mousePos - myBlendSpaceGridPos;
	//clamp
	mousePosInBlendspace = { Utils::Clamp(mousePosInBlendspace.x / myBlendSpaceGridSize.x, 0.f, 1.f),
	   Utils::Clamp(mousePosInBlendspace.y / myBlendSpaceGridSize.y, 0.f, 1.f) };
	mousePosInBlendspace.x = Utils::Lerp(myBlendSpace->myHorizontalAxis.Min, myBlendSpace->myHorizontalAxis.Max, mousePosInBlendspace.x);
	mousePosInBlendspace.y = Utils::Lerp(myBlendSpace->myVerticalAxis.Max, myBlendSpace->myVerticalAxis.Min, mousePosInBlendspace.y);
	return mousePosInBlendspace;
}

Utils::Vector2f BlendSpaceEditorWindow::ScreenToGridPosition(Utils::Vector2f aScreenPosition)
{
	auto gridPosition = aScreenPosition - myBlendSpaceGridPos;
	//clamp
	gridPosition = { Utils::Clamp(gridPosition.x / myBlendSpaceGridSize.x, 0.f, 1.f),
	   Utils::Clamp(gridPosition.y / myBlendSpaceGridSize.y, 0.f, 1.f) };
	gridPosition.x = Utils::Lerp(myBlendSpace->myHorizontalAxis.Min, myBlendSpace->myHorizontalAxis.Max, gridPosition.x);
	gridPosition.y = Utils::Lerp(myBlendSpace->myVerticalAxis.Max, myBlendSpace->myVerticalAxis.Min, gridPosition.y);
	return gridPosition;
}

Utils::Vector2f BlendSpaceEditorWindow::GridToScreenPosition(Utils::Vector2f aGridPosition)
{
	const float blendSpaceValuesWidth = myBlendSpace->myHorizontalAxis.Max - myBlendSpace->myHorizontalAxis.Min;
	const float blendSpaceValuesHeight = myBlendSpace->myVerticalAxis.Max - myBlendSpace->myVerticalAxis.Min;

	Utils::Vector2f screenPos = { 0,0 };
	const float xFraction = (aGridPosition.x - myBlendSpace->myHorizontalAxis.Min) / blendSpaceValuesWidth;
	const float yFraction = (aGridPosition.y - myBlendSpace->myVerticalAxis.Min) / blendSpaceValuesHeight;
	screenPos.x = myBlendSpaceGridPos.x + xFraction * myBlendSpaceGridSize.x;
	if (myBlendSpace->myBlendspaceType == Firefly::BlendspaceType::OneDimensional)
	{
		screenPos.y = myBlendSpaceGridPos.y + myBlendSpaceGridSize.y / 2.f;
	}
	else
	{
		screenPos.y = myBlendSpaceGridPos.y + (1.f - yFraction) * myBlendSpaceGridSize.y;
	}
	return screenPos;
}
