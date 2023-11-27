#include "EditorPch.h"
#include "AnimationEditor.h"
#include "WindowRegistry.h"

#include "Editor/EditorCamera.h"
#include "Editor/Utilities/ImGuiUtils.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Material/MaterialAsset.h"
#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Asset/Importers/FBXImporter.h"

#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"

#include "Utils/Timer.h"


#include "imgui/imgui_internal.h"

REGISTER_WINDOW(AnimationEditor);

Firefly::Frame myCurrentFrame = {};
Firefly::Frame myFrameWithoutKeys = {};


AnimationEditor::AnimationEditor()
	: EditorWindow("Animation Editor")
{
	myIsFirstFrame = true;

	myPreviewRenderSceneID = Firefly::Renderer::InitializeScene();

	myEditorCamera = CreateRef<EditorCamera>();
	myEditorCamera->Initialize(Firefly::CameraInfo());

	myWindowFlags |= ImGuiWindowFlags_MenuBar;

	myCubeMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("Cube");
	myGroundMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AnimationEditor/GroundMaterial.mat");

	myPyramidMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("PyramidBottomPivot");
	myBoneMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AnimationEditor/BoneMaterial.mat");

	mySphereMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("Assets/Graphical Assets/Sphere.mesh", true);


	myPlayButtonTexture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_play.dds", true);
	myPauseButtonTexture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_pause.dds", true);

	myTime = 0;
	myIsDraggingTimeline = false;
	myIsPlaying = false;
	myIsUsingGizmo = false;
	SelectTrack(TrackType::RotationX, false);

	myValueDisplayMax = 360;
	myValueDisplayMin = -360;
	myValueDisplayStep = 45;
	myFramesDisplayedCount = 10;

	myFrameStepSize = 5;
	myFrameSecondaryStepSize = 1;
	mySelectedID = 0;
	myIsDragging = false;


	myManualFocusFlag = true;

}

void AnimationEditor::SetAnimation(const std::string& aPath)
{
	myAnimation = Firefly::ResourceCache::GetAsset<Firefly::Animation>(aPath);
	myHasInitialized = false;
}

void AnimationEditor::OnImGui()
{
	/*
	{
		ImGui::Begin("TestCurves");
		static Utils::Vector2f points[4] = { {100,100}, {120, 50}, {140,150}, {150,100} };
		static Utils::Vector4f colors[4] = { {1,1,1,1}, {1,0,0,1}, {0,1,0,1}, {0,0,1,1} };
		static int index = 0;
		static bool dragging = false;
		auto dList = ImGui::GetWindowDrawList();
		for (int i = 0; i < 4; i++)
		{
			auto pos = UtilsVecToImGuiVec(points[i]) + ImGui::GetWindowPos();
			dList->AddCircleFilled(pos, 5.f, ImGui::GetColorU32(UtilsVecToImGuiVec(colors[i])));
			ImGui::ItemAdd({ pos - ImVec2(5,5), pos + ImVec2(5,5) }, ImGui::GetID(("test" + std::to_string(i)).c_str()));
			if (ImGui::IsItemClicked())
			{
				myIsDragging = true;
				index = i;
			}

			if (index == i && myIsDragging)
			{
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					myIsDragging = false;
				}
				points[i] = ImGuiVecToUtilsVec(ImGui::GetMousePos() - ImGui::GetWindowPos());
			}
		}
		auto lastPoint = points[0] + ImGuiVecToUtilsVec(ImGui::GetWindowPos());
		for (int i = 1; i <= 10; i++)
		{
			auto dist = points[0] - points[3];
			auto t = i / 10.f;
			auto p = points[0].y + std::abs(dist.y) * t + ImGui::GetWindowPos().y;

			Utils::Vector2f result;
			result += (1 - t) * (1 - t) * (1 - t) * points[0];
			result += 3 * (1 - t) * (1 - t) * t * points[1];
			result += 3 * (1 - t) * t * t * points[2];
			result += t * t * t * points[3];

			result += ImGuiVecToUtilsVec(ImGui::GetWindowPos());

			auto d = Utils::Vector2f(result.x, result.y);
			dList->AddLine(UtilsVecToImGuiVec(lastPoint), UtilsVecToImGuiVec(d), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
			lastPoint = d;
		}

		ImGui::End();
	}
	*/


	if (!myAnimation)
	{
		ImGui::Text("No Animation loaded, please load one using the content browser by double clicking it or dropping in in this window.");
		if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".anim"))
		{
			std::string path = (const char*)payload->Data;
			SetAnimation(path);
			ImGui::SetWindowFocus();
		}
		return;
	}
	if (!myAnimation->IsLoaded())
	{
		ImGui::Text("Animation is loading...");
		return;
	}
	if (!myHasInitialized)
	{
		myHasInitialized = true;
		myFramesDisplayedCount = myAnimation->FrameCount - 1;
		myAnimationFrameCount = myAnimation->FrameCount;
	}

	{
		myCurrentFrame = myAnimation->GetFrame(myTime, true);
		myFrameWithoutKeys = myAnimation->GetFrame(myTime, true, false);
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

	auto dockspaceID = ImGui::GetID("AnimationEditorWindowDockspace");

	if (!ImGui::DockBuilderGetNode(dockspaceID))
	{
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());

		auto dockMainID = dockspaceID;
		auto dockBottomID = ImGui::DockBuilderSplitNode(dockMainID,
			ImGuiDir_Down, 0.4f, nullptr, &dockMainID);
		auto dockLeftID = ImGui::DockBuilderSplitNode(dockMainID,
			ImGuiDir_Left, 0.3f, nullptr, &dockMainID);

		ImGui::DockBuilderDockWindow("Viewport##AnimationEditor", dockMainID);
		ImGui::DockBuilderDockWindow("Timeline##AnimationEditor", dockBottomID);
		ImGui::DockBuilderDockWindow("Bone Hierarchy##AnimationEditor", dockLeftID);

		ImGui::DockBuilderFinish(dockspaceID);
	}
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar);



	ImGui::Begin("Bone Hierarchy##AnimationEditor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	bool hierarchyFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawHierarchy();
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
	ImGui::Begin("Viewport##AnimationEditor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	bool viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawViewport();
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Timeline##AnimationEditor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	bool timelineFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawTimeline();
	ImGui::End();

	if (!hierarchyFocused && !viewportFocused && !timelineFocused)
	{
		myIsFocused = false;
	}
	else
	{
		myIsFocused = true;
	}

	if (myIsFirstFrame)
	{
		myIsFirstFrame = false;
		ImGui::DockBuilderRemoveNode(ImGui::GetID("AnimationEditorWindowDockspace"));
	}

	if (myIsPlaying && !myIsDraggingTimeline)
	{
		myTime += Utils::Timer::GetDeltaTime();
		auto duration = myAnimation->GetDuration();
		if (myTime >= duration)
		{
			myTime -= duration;
		}
	}
}

void AnimationEditor::Save()
{
	Firefly::ResourceCache::GetFBXImporter()->ExportAnimationToBinary(myAnimation, myAnimation->GetPath());
}

void AnimationEditor::DrawHierarchy()
{
	ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
	ImGui::Text("Bone Hierarchy:");
	ImGuiUtils::PopFont();
	DrawBoneHierarchyRecursive(myAnimation->GetAnimatedMesh()->GetSkeleton(), 0);

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		myCurrentSelectedBoneIndex = -1;
	}
}

void AnimationEditor::DrawBoneHierarchyRecursive(Firefly::Skeleton& aSkeleton, int aBoneIndex)
{
	auto& bone = aSkeleton.Bones[aBoneIndex];

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

	if (bone.Children.size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (myCurrentSelectedBoneIndex == aBoneIndex)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	ImVec4 boneColor = { 1,1,1,1 };

	ImGui::PushStyleColor(ImGuiCol_Text, boneColor);

	const bool open = ImGui::TreeNodeEx(bone.Name.c_str(), flags);

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Copy##CopyBoneHierarchyName"))
		{
			std::string command = bone.Name;
			OpenClipboard(Firefly::Application::GetWindow()->GetHandle());
			EmptyClipboard();
			HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, command.size() + 1);
			if (!hg) {
				CloseClipboard();
				return;
			}
			memcpy(GlobalLock(hg), command.c_str(), command.size() + 1);
			GlobalUnlock(hg);
			SetClipboardData(CF_TEXT, hg);
			CloseClipboard();
			GlobalFree(hg);

			ImGuiUtils::NotifySuccess(std::string(command + " copied to clipboard!").c_str());

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsItemClicked())
	{
		myCurrentSelectedBoneIndex = aBoneIndex;
	}
	ImGui::PopStyleColor();

	auto drawlist = ImGui::GetWindowDrawList();
	if (open)
	{
		for (int i = 0; i < aSkeleton.Bones[aBoneIndex].Children.size(); i++)
		{
			/*ImVec2 offset = { -16.f, 0 };
			drawlist->AddLine(ImGui::GetCursorScreenPos() + offset, ImGui::GetCursorScreenPos() + offset + ImVec2(0, 16.f * (i + 1)), ImGui::GetColorU32(ImGuiCol_Text));
			drawlist->AddLine(ImGui::GetCursorScreenPos() + offset + ImVec2(0, 16.f * (i + 1)), ImGui::GetCursorScreenPos() + ImVec2(20, 16.f * (i + 1)) + offset, ImGui::GetColorU32(ImGuiCol_Text));*/
			DrawBoneHierarchyRecursive(aSkeleton, aSkeleton.Bones[aBoneIndex].Children[i]);
		}
		ImGui::TreePop();
	}
}

void AnimationEditor::DrawViewport()
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
	environmentData.Intensity = 0.5f;
	Firefly::Renderer::Submit(environmentData);
	//

	Firefly::DirLightPacket pack{};
	pack.Direction = { -0.71, 0.71, 0.71, 1 };
	pack.ColorAndIntensity = { 1,1,1,1 };
	pack.dirLightInfo.x = 1;
	pack.dirLightInfo.y = 1;
	Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res2048);



	Firefly::PostProcessInfo postProcessInfo{};
	postProcessInfo.Enable = true;
	postProcessInfo.Data.Padding.z = 0.1f;
	Firefly::Renderer::Submit(postProcessInfo);

	myEditorCamera->SetViewportRect(
		{ static_cast<int>(windowPos.x), static_cast<int>(windowPos.y),
		static_cast<int>(windowPos.x + contentRegionAvail.x),
		static_cast<int>(windowPos.y + contentRegionAvail.y) });


	if (Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Width) - contentRegionAvail.x) > 1.0f
		|| Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->GetSpecs().Height) - contentRegionAvail.y) > 1.0f)
	{
		auto newSize = Utils::Vector2<uint32_t>(contentRegionAvail.x, contentRegionAvail.y);
		Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID)->Resize(newSize);
		myEditorCamera->GetCamera()->SetSizeX(static_cast<float>(newSize.x));
		myEditorCamera->GetCamera()->SetSizeY(static_cast<float>(newSize.y));
	}

	//mesh 
	auto& frame = myCurrentFrame;
	auto mesh = myAnimation->GetAnimatedMesh();
	auto& skeleton = mesh->GetSkeleton();
	std::vector<Utils::Matrix4f> frameCalculatedMatrices;
	frame.CalculateTransforms(skeleton, frameCalculatedMatrices);
	for (auto& submesh : mesh->GetSubMeshes())
	{
		Firefly::MeshSubmitInfo mdl;
		mdl.Mesh = &submesh;
		mdl.Transform = Utils::Transform(Utils::Vector3f::Zero(), Utils::Quaternion(), Utils::Vector3f::One()).GetMatrix();
		mdl.SetBoneTransforms(frameCalculatedMatrices);
		Firefly::Renderer::Submit(mdl);
	}
	//

	//Skeleton

	auto& bones = skeleton.Bones;

	std::vector<Utils::Vector3f> worldPositions;

	//might take performance, remember to check if needed
	for (int i = 0; i < frame.LocalTransforms.size(); i++)
	{
		worldPositions.push_back(GetGlobalBoneTransform(i, frame, skeleton).GetPosition());
	}

	if (myCurrentSelectedBoneIndex != -1)
	{
		const auto parentIndex = bones[myCurrentSelectedBoneIndex].Parent;
		auto lookRot = Utils::Quaternion();
		float length = 1;
		auto pos = GetGlobalBoneTransform(myCurrentSelectedBoneIndex, frame, skeleton).GetPosition();
		auto dir = Utils::Vector3f(0, 1, 0);
		if (parentIndex != -1)
		{
			auto parentPos = GetGlobalBoneTransform(parentIndex, frame, skeleton).GetPosition();
			dir = pos - parentPos;
			pos = parentPos;
			length = dir.Length();
			dir.Normalize();

			lookRot = Utils::Quaternion::CreateLookRotation(dir, Utils::Vector3f::Up());
			lookRot = lookRot * Utils::Quaternion::CreateFromEulerAngles({ 90,0,0 });
		}

		Firefly::MeshSubmitInfo mdl;

		mdl.Mesh = &myPyramidMesh->GetSubMeshes()[0];
		mdl.Transform = Utils::Transform(pos + dir, lookRot, Utils::Vector3f(0.02f, (length - 1.f) / 100.f, 0.02f)).GetMatrix();
		mdl.Outline = true;
		mdl.CastShadows = false;
		mdl.Material = myBoneMaterial;
		Firefly::Renderer::Submit(mdl);

		Firefly::MeshSubmitInfo sphereMdl;
		sphereMdl.Mesh = &mySphereMesh->GetSubMeshes().front();
		sphereMdl.Transform = Utils::Transform(pos, lookRot, Utils::Vector3f(1.f, 1.f, 1.f)).GetMatrix();
		sphereMdl.Outline = true;
		sphereMdl.CastShadows = false;
		sphereMdl.Material = myBoneMaterial;
		Firefly::Renderer::Submit(sphereMdl);
	}
	//

	//GroundCube
	Firefly::MeshSubmitInfo groundCube;
	groundCube.Mesh = &myCubeMesh->GetSubMeshes().front();
	groundCube.Transform = Utils::Transform(Utils::Vector3f(0, -50.01f, 0), Utils::Quaternion(), Utils::Vector3f(10, 1, 10)).GetMatrix();
	groundCube.Material = myGroundMaterial;
	Firefly::Renderer::Submit(groundCube);
	//

	Firefly::Renderer::EndScene();

	auto frameBuffer = Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID);
	ImGui::Image(frameBuffer->GetColorAttachment(0).Get(), { contentRegionAvail.x , contentRegionAvail.y });


	HandleGizmo();

}

void AnimationEditor::HandleGizmo()
{
	if (!myIsFocused)
	{
		return;
	}
	if (myCurrentSelectedBoneIndex == -1)
	{
		return;
	}
	static ImGuizmo::OPERATION operation = ImGuizmo::ROTATE;
	static ImGuizmo::MODE mode = ImGuizmo::LOCAL;

	const auto& camera = myEditorCamera->GetCamera();
	float viewMatrix[16] = { 0 };
	memcpy(viewMatrix, &camera->GetViewMatrix(), sizeof(float) * 16);
	float projectionMatrix[16] = { 0 };
	memcpy(projectionMatrix, &camera->GetProjectionMatrixPerspective(), sizeof(float) * 16);

	auto& skeleton = myAnimation->GetAnimatedMesh()->GetSkeleton();
	auto globalTransform = GetGlobalBoneTransform(myCurrentSelectedBoneIndex, myCurrentFrame, skeleton);
	auto transformMatrix = globalTransform.CreateMatrix();

	ImGuizmo::Enable(true);

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);


	ImGuizmo::Manipulate(&viewMatrix[0], &projectionMatrix[0], operation, mode, reinterpret_cast<float*>(&transformMatrix));

	if (ImGuizmo::IsUsing())
	{
		if (!myIsUsingGizmo)
		{
			myInitialGizmoLocalTransform = myCurrentFrame.LocalTransforms[myCurrentSelectedBoneIndex];
			myInitialGizmoGlobalTransform = globalTransform;
			myWasPlayingWhenStartedUsingGizmo = myIsPlaying;
			myIsPlaying = false;
		}
		else
		{
			Utils::BasicTransform finalTransform = transformMatrix;
			std::vector<int> parents;
			for (int parent = skeleton.Bones[myCurrentSelectedBoneIndex].Parent; parent >= 0;
				parent = skeleton.Bones[parent].Parent)
			{
				parents.push_back(parent);
			}
			for (int i = parents.size() - 1; i >= 0; --i)
			{
				auto parent = parents[i];
				finalTransform = Utils::BasicTransform::Combine(Utils::BasicTransform::Inverse(myCurrentFrame.LocalTransforms[parent]), finalTransform);
			}

			Utils::Vector3f modifiedPosition = finalTransform.GetPosition();
			Utils::Vector3f modifiedRotation = finalTransform.GetRotation();
			Utils::Vector3f modifiedScale = finalTransform.GetScale();

			switch (operation)
			{
				case ImGuizmo::OPERATION::TRANSLATE:
					myCurrentFrame.LocalTransforms[myCurrentSelectedBoneIndex].SetPosition(myInitialGizmoLocalTransform.GetPosition() + (modifiedPosition - myInitialGizmoGlobalTransform.GetPosition()));
					break;
				case ImGuizmo::ROTATE:
				{
					auto rotDiff = (modifiedRotation - myFrameWithoutKeys.LocalTransforms[myCurrentSelectedBoneIndex].GetRotation());

					AddKey(myTime, rotDiff.x, TrackType::RotationX, myCurrentSelectedBoneIndex);
					AddKey(myTime, rotDiff.y, TrackType::RotationY, myCurrentSelectedBoneIndex);
					AddKey(myTime, rotDiff.z, TrackType::RotationZ, myCurrentSelectedBoneIndex);
					break;
				}
				case ImGuizmo::OPERATION::SCALE:
					myCurrentFrame.LocalTransforms[myCurrentSelectedBoneIndex].SetScale(myInitialGizmoLocalTransform.GetScale() * modifiedScale);
					break;
				default:
					break;
			}

		}
	}
	else if (myIsUsingGizmo)
	{
		myIsPlaying = myWasPlayingWhenStartedUsingGizmo;
	}
	myIsUsingGizmo = ImGuizmo::IsUsing();
}

void AnimationEditor::DrawTimeline()
{
	if (ImGui::BeginChild("LeftPanelInTimeline", { 100,0 }))
	{
		auto playButtonTexture = myIsPlaying ? myPauseButtonTexture : myPlayButtonTexture;
		if (ImGui::ImageButton(playButtonTexture->GetSRV().Get(), ImVec2(40, 40)))
		{
			myIsPlaying = !myIsPlaying;
		}

		bool controlHeld = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
		for (uint32_t trackType = 0; trackType < static_cast<uint32_t>(TrackType::COUNT); trackType++)
		{
			auto trackTypeEnum = static_cast<TrackType>(std::pow(2, trackType));
			if (ImGui::Selectable(TrackTypeToString(trackTypeEnum).c_str(), IsTrackSelected(trackTypeEnum)))
			{
				SelectTrack(trackTypeEnum, controlHeld);
			}
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorScreenPos({ ImGui::GetWindowPos().x + 100, ImGui::GetWindowPos().y });
	if (ImGui::BeginChild("Timeline Graph"))
		//timeline
	{
		auto drawList = ImGui::GetWindowDrawList();
		myTimelineStartPos = ImGuiVecToUtilsVec(ImGui::GetWindowPos());
		myTimelineWidth = ImGui::GetContentRegionAvail().x;

		const auto timelineRectMin = UtilsVecToImGuiVec(myTimelineStartPos);
		const auto timelineRectMax = ImVec2(myTimelineStartPos.x + myTimelineWidth, myTimelineStartPos.y + myTimelineHeight);

		drawList->AddRectFilled(timelineRectMin, timelineRectMax, ImGui::GetColorU32(UtilsVecToImGuiVec(myTimelineBackgroundColor)));
		float maxTimeOnTimeline = myFramesDisplayedCount / myAnimation->FramesPerSecond;
		const float xPos = GetXPosOnTimeline(myTime);

		//draw lines 

		const auto lineSpacing = myTimelineWidth / GetTimelineLineCount();
		const float tallLineHeight = myTimelineHeight / 2.f;
		const float shortLineHeight = myTimelineHeight / 4.f;
		for (int i = 0; i < myFramesDisplayedCount; i += myFrameSecondaryStepSize)
		{
			bool tallLine = (i % myFrameStepSize) == 0;
			const float lineHeight = (tallLine ? tallLineHeight : shortLineHeight);
			const float fraction = i / static_cast<float>(myFramesDisplayedCount);
			const auto linePos = ImVec2(myTimelineStartPos.x + fraction * myTimelineWidth, myTimelineStartPos.y + (myTimelineHeight - lineHeight));
			drawList->AddLine(linePos, { linePos.x, linePos.y + lineHeight }, ImGui::GetColorU32({ 1,1,1,1 }));
			if (tallLine)
			{
				ImGuiUtils::PushFont(ImGuiUtilsFont_Roboto_10);
				drawList->AddText({ linePos.x + 5, linePos.y }, ImGui::GetColorU32(ImGuiCol_Text), std::to_string(i * myFrameSecondaryStepSize).c_str());
				ImGuiUtils::PopFont();
			}
		}



		ImGui::PushClipRect(timelineRectMin, timelineRectMax, false);
		drawList->AddTriangleFilled({ xPos - myTimelineHandleWidth / 2.f, timelineRectMin.y },
			{ xPos + myTimelineHandleWidth / 2.f, timelineRectMin.y },
			{ xPos, timelineRectMin.y + myTimelineHandleHeight }, ImGui::GetColorU32(UtilsVecToImGuiVec(myTimelineHandleColor)));
		ImGui::PopClipRect();

		if (ImGui::IsMouseHoveringRect(timelineRectMin, timelineRectMax))
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				myIsDraggingTimeline = true;
				myIsPlaying = false;
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
				const auto clampedPos = std::clamp(mousePosOnTimeline, 0.f, myTimelineWidth);
				myTime = clampedPos / myTimelineWidth * maxTimeOnTimeline;
			}

		}

		if (myCurrentSelectedBoneIndex != -1)
		{
			DrawKeyframeGraph();
		}

	}
	ImGui::EndChild();
}

void AnimationEditor::DrawKeyframeGraph()
{
	//Keyframes
	auto drawList = ImGui::GetWindowDrawList();
	auto& skeleton = myAnimation->GetAnimatedMesh()->GetSkeleton();
	myKeyframeGraphStartPos = Utils::Vector2f(myTimelineStartPos.x, myTimelineStartPos.y + myTimelineHeight);
	myKeyframeGraphSize = Utils::Vector2f(myTimelineWidth, ImGui::GetContentRegionAvail().y);
	drawList->AddRectFilled(UtilsVecToImGuiVec(myKeyframeGraphStartPos), UtilsVecToImGuiVec(myKeyframeGraphStartPos + myKeyframeGraphSize), ImGui::GetColorU32(UtilsVecToImGuiVec(myKeyframeGraphBackgroundColor)));

	const auto verticalLineCount = GetTimelineLineCount();

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
	{
		auto wheelCounter = ImGui::GetIO().MouseWheel;
		if (wheelCounter != 0)
		{
			myFramesDisplayedCount -= wheelCounter;
		}
	}
	//Draw grid lines
	{
		//Vertical lines
		const auto lineSpacing = myKeyframeGraphSize.x / GetTimelineLineCount();
		for (int i = 0; i < myFramesDisplayedCount; i += myFrameSecondaryStepSize)
		{
			const float lineHeight = myKeyframeGraphSize.y;
			const float fraction = i / static_cast<float>(myFramesDisplayedCount);
			const auto linePos = ImVec2(myKeyframeGraphStartPos.x + fraction * myKeyframeGraphSize.x, myKeyframeGraphStartPos.y);
			drawList->AddLine(linePos, { linePos.x, linePos.y + lineHeight }, ImGui::GetColorU32(UtilsVecToImGuiVec(myKeyframeGraphLineColor)));
		}
		//

		//Horizontal lines
		const auto keyframeHeight = myKeyframeGraphSize.y / skeleton.Bones.size();
		for (float i = myValueDisplayMax; i >= myValueDisplayMin; i -= myValueDisplayStep)
		{
			const float lineWidth = myKeyframeGraphSize.x;
			const auto linePos = ImVec2(myKeyframeGraphStartPos.x, GetYPosOnGraph(i));
			drawList->AddLine(linePos, { linePos.x + lineWidth, linePos.y }, ImGui::GetColorU32(UtilsVecToImGuiVec(myKeyframeGraphLineColor)));

			ImGuiUtils::PushFont(ImGuiUtilsFont_Roboto_10);
			const auto displayText = std::to_string(i);
			const auto textYPos = linePos.y - ImGui::CalcTextSize(displayText.c_str()).y / 2.f;
			if (textYPos > myKeyframeGraphStartPos.y)
			{
				drawList->AddText({ linePos.x + 5, textYPos }, ImGui::GetColorU32(ImGuiCol_Text), displayText.c_str());
			}
			ImGuiUtils::PopFont();
		}
		//
	}

	//on middle mouse create a key
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		{
			int selectedTrackTypeCount = 0;
			for (int i = 0; i < 32; i++)
			{
				uint32_t mask = 1 << i;
				if ((mask & static_cast<uint32_t>(mySelectedTrackTypes)) > 0)
				{
					selectedTrackTypeCount++;
				}
			}

			if (selectedTrackTypeCount == 1)
			{
				AddKeyAtMouse();
			}
		}
	}

	//draw dark areas for outside of animation
	{

	}
	//

	//Draw Curves
	{
		for (uint32_t trackType = 0; trackType < static_cast<uint32_t>(TrackType::COUNT); trackType++)
		{
			auto trackTypeEnum = static_cast<TrackType>(std::pow(2, trackType));
			if (!IsTrackSelected(trackTypeEnum))
			{
				continue;
			}
			if (myAnimation->myAdditiveTracks.contains(myCurrentSelectedBoneIndex))
			{
				if (myAnimation->myAdditiveTracks[myCurrentSelectedBoneIndex].contains(trackTypeEnum))
				{
					auto& track = myAnimation->myAdditiveTracks[myCurrentSelectedBoneIndex][trackTypeEnum];
					for (int i = 0; i < track.KeyFrames.size(); i++)
					{
						if (i == 0)
						{
							//draw line from start of graph to first keyframe
							const auto startPos = ImVec2(myKeyframeGraphStartPos.x, GetYPosOnGraph(track.KeyFrames[i].Value));
							const auto endPos = ImVec2(GetXPosOnGraph(track.KeyFrames[i].Frame), GetYPosOnGraph(track.KeyFrames[i].Value));
							drawList->AddLine(startPos, endPos, ImGui::GetColorU32(UtilsVecToImGuiVec(myCurveColor)), myCurveLineThickness);
						}

						if (i == track.KeyFrames.size() - 1)
						{
							//draw line from last keyframe to end of graph
							const auto startPos = ImVec2(GetXPosOnGraph(track.KeyFrames[i].Frame), GetYPosOnGraph(track.KeyFrames[i].Value));
							const auto endPos = ImVec2(myKeyframeGraphStartPos.x + myKeyframeGraphSize.x, GetYPosOnGraph(track.KeyFrames[i].Value));
							drawList->AddLine(startPos, endPos, ImGui::GetColorU32(UtilsVecToImGuiVec(myCurveColor)), myCurveLineThickness);
						}
						else
						{
							const auto frame1Time = track.KeyFrames[i].Frame;
							const auto frame2Time = track.KeyFrames[i + 1].Frame;
							//draw line between keyframes
							const auto startPos = ImVec2(GetXPosOnGraph(frame1Time), GetYPosOnGraph(track.KeyFrames[i].Value));
							const auto endPos = ImVec2(GetXPosOnGraph(frame2Time), GetYPosOnGraph(track.KeyFrames[i + 1].Value));

							//sample curve between keyframes
							const auto curveSampleCount = 40;
							const auto curveSampleStep = (endPos.x - startPos.x) / curveSampleCount;
							auto prevCurveSamplePos = startPos;
							for (int j = 1; j <= curveSampleCount; j++)
							{
								const auto sampleTime = frame1Time + (frame2Time - frame1Time) * (j / static_cast<float>(curveSampleCount));
								const auto resultValue = track.GetValue(sampleTime);
								const auto curveSamplePos = ImVec2(startPos.x + j * curveSampleStep, GetYPosOnGraph(resultValue));
								drawList->AddLine(prevCurveSamplePos, curveSamplePos, ImGui::GetColorU32(UtilsVecToImGuiVec(myCurveColor)), myCurveLineThickness);
								prevCurveSamplePos = curveSamplePos;
							}
							//draw line from end of sample to next keyframe (to prevent disconnects)
							drawList->AddLine(prevCurveSamplePos, endPos, ImGui::GetColorU32(UtilsVecToImGuiVec(myCurveColor)), myCurveLineThickness);
						}

						const bool isAlone = track.KeyFrames.size() == 1;
						const bool hasKeyBehind = (i > 0) && !isAlone;
						const bool hasKeyAhead = (i < track.KeyFrames.size() - 1) && !isAlone;

						auto drawTangentHandle = [&](KeyFrame& aKey, bool aInTangentFlag)
						{
							auto selectType = aInTangentFlag ? SelectType::TangentIn : SelectType::TangentOut;
							auto& tangent = aInTangentFlag ? aKey.TangentIn : aKey.TangentOut;

							const auto keyXPos = GetXPosOnGraph(aKey.Frame);
							const auto keyYPos = GetYPosOnGraph(aKey.Value);

							auto color = UtilsVecToImGuiVec(myKeyColor);
							if (mySelectedID == aKey.ID && mySelectedType == selectType)
							{
								color = UtilsVecToImGuiVec(myKeySelectedColor);
							}

							ImVec2 tangentPos = { GetXPosOnGraph(aKey.Frame + tangent.x) ,  GetYPosOnGraph(aKey.Value + tangent.y) };
							drawList->AddLine(ImVec2(keyXPos, keyYPos), tangentPos, ImGui::GetColorU32(color), 1.f);

							drawList->AddCircle(tangentPos, 5, ImGui::GetColorU32(color));
							std::string id = "TangentHandle" + std::string(aInTangentFlag ? "In" : "Out") + std::to_string(aKey.ID);
							ImGui::ItemAdd(ImRect(tangentPos - ImVec2(5, 5),
								tangentPos + ImVec2(5, 5)),
								ImGui::GetID(id.c_str()));


							if (ImGui::IsItemClicked())
							{
								SelectKey(aKey, selectType);
								myIsDragging = true;
							}

							if (myIsDragging && mySelectedType == selectType && mySelectedID == aKey.ID)
							{
								if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
								{
									myIsDragging = false;
								}
								auto mousePos = ImGui::GetMousePos();

								auto dir = Utils::Vector2f(mousePos.x - keyXPos, mousePos.y - keyYPos);
								if (dir.LengthSqr() > 0.0001f)
								{
									dir.Normalize();

									auto time = GetTimeFromXPos(mousePos.x);
									auto value = GetValueFromYPos(mousePos.y);

									if ((aInTangentFlag && time > aKey.Frame) || !aInTangentFlag && time < aKey.Frame)
									{
										time = aKey.Frame;
									}

									tangent.x = time - aKey.Frame;
									tangent.y = value - aKey.Value;
								}

							}
						};

						if (mySelectedID == track.KeyFrames[i].ID)
						{

							if (hasKeyBehind)
							{
								auto& behindKey = track.KeyFrames[i - 1];
								if (behindKey.CurveType == CurveType::CubicBroken)
								{
									drawTangentHandle(track.KeyFrames[i], true);
								}
							}
							if (hasKeyAhead)
							{
								auto& aheadKey = track.KeyFrames[i + 1];
								if (track.KeyFrames[i].CurveType == CurveType::CubicBroken)
								{
									drawTangentHandle(track.KeyFrames[i], false);
								}

							}
						}
					}
				}
			}
		}
	}
	//

	//Draw Keyframes
	{
		for (uint32_t trackType = 0; trackType < static_cast<uint32_t>(TrackType::COUNT); trackType++)
		{
			auto trackTypeEnum = static_cast<TrackType>(std::pow(2, trackType));
			if (!IsTrackSelected(trackTypeEnum))
			{
				continue;
			}
			if (myAnimation->myAdditiveTracks.contains(myCurrentSelectedBoneIndex))
			{
				if (myAnimation->myAdditiveTracks[myCurrentSelectedBoneIndex].contains(trackTypeEnum))
				{
					auto& track = myAnimation->myAdditiveTracks[myCurrentSelectedBoneIndex][trackTypeEnum];
					for (int i = 0; i < track.KeyFrames.size(); ++i)
					{
						auto& keyframe = track.KeyFrames[i];
						const ImVec2 pos = { GetXPosOnGraph(keyframe.Frame), GetYPosOnGraph(keyframe.Value) };

						auto color = UtilsVecToImGuiVec(myKeyColor);
						if (mySelectedID == keyframe.ID && mySelectedType == SelectType::KeyFrame)
						{
							color = UtilsVecToImGuiVec(myKeySelectedColor);
						}
						drawList->AddQuadFilled({ pos.x + myKeyframeRadius, pos.y }, { pos.x , pos.y + myKeyframeRadius },
							{ pos.x - myKeyframeRadius, pos.y }, { pos.x, pos.y - myKeyframeRadius },
							ImGui::GetColorU32(color));

						ImGui::ItemAdd({ { pos.x - myKeyframeRadius, pos.y - myKeyframeRadius }, { pos.x + myKeyframeRadius, pos.y + myKeyframeRadius } },
							ImGui::GetID(("KeyFrame" + std::to_string(keyframe.ID)).c_str()));

						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::MenuItem(("Delete KeyFrame##" + std::to_string(keyframe.ID)).c_str()))
							{
								track.KeyFrames.erase(track.KeyFrames.begin() + i);
								mySelectedID = 0;
								i--;
								ImGui::EndPopup();
								continue;
							}

							//constant
							if (ImGui::MenuItem(("Set Curve Constant##" + std::to_string(keyframe.ID)).c_str()))
							{
								keyframe.CurveType = CurveType::Constant;
							}

							//linear
							if (ImGui::MenuItem(("Set Curve Linear##" + std::to_string(keyframe.ID)).c_str()))
							{

								keyframe.CurveType = CurveType::Linear;
							}

							//Beziér
							if (ImGui::MenuItem(("Set Curve Cubic Broken##" + std::to_string(keyframe.ID)).c_str()))
							{

								keyframe.CurveType = CurveType::CubicBroken;
							}

							ImGui::EndPopup();
						}


						if (ImGui::IsItemClicked())
						{
							myIsDragging = true;
							SelectKey(keyframe, SelectType::KeyFrame);
						}

						if (myIsDragging && mySelectedType == SelectType::KeyFrame && mySelectedID == keyframe.ID)
						{
							auto mousePos = ImGui::GetMousePos();

							const auto time = GetTimeFromXPos(mousePos.x);
							const auto value = GetValueFromYPos(mousePos.y);

							keyframe.Frame = time;
							keyframe.Value = value;

							DrawDragKeyData(ImGuiVecToUtilsVec(mousePos), keyframe);

							if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
							{
								myIsDragging = false;
							}

							//check if keyframe time is less than previous keyframe time
							if (i != 0)
							{
								if (keyframe.Frame < track.KeyFrames[i - 1].Frame)
								{
									//swap keyframes
									std::swap(track.KeyFrames[i], track.KeyFrames[i - 1]);
									SelectKey(track.KeyFrames[i - 1], SelectType::KeyFrame);
								}
							}

							//check if keyframe time is greater than next keyframe time
							if (i != track.KeyFrames.size() - 1)
							{
								if (keyframe.Frame > track.KeyFrames[i + 1].Frame)
								{
									//swap keyframes
									std::swap(track.KeyFrames[i], track.KeyFrames[i + 1]);
									SelectKey(track.KeyFrames[i + 1], SelectType::KeyFrame);
								}
							}
						}
					}
				}
			}
		}
	}
	//


}

void AnimationEditor::DrawDragKeyData(const Utils::Vector2f& aPos, KeyFrame& aKeyframe)
{
	auto drawList = ImGui::GetWindowDrawList();
	auto startPos = UtilsVecToImGuiVec(aPos);

	auto timeDisplayText = "Time (Frame): " + FormatTimeToFrameAndTimeText(aKeyframe.Frame);
	drawList->AddText(startPos, ImGui::GetColorU32(ImGuiCol_Text), timeDisplayText.c_str());

	auto textHeight = ImGui::CalcTextSize("").y;

	auto valueDiplayText = "Value: " + std::format("{:.2f}", aKeyframe.Value);
	drawList->AddText(startPos + ImVec2(0.f, ImGui::GetStyle().ItemSpacing.y + textHeight), ImGui::GetColorU32(ImGuiCol_Text), valueDiplayText.c_str());
}

std::string AnimationEditor::FormatTimeToFrameAndTimeText(float aTime)
{
	return std::format("{:.2f}", aTime) + " (" + std::to_string(static_cast<int>(std::roundf(TimeToFrame(aTime)))) + ")";
}

void AnimationEditor::AddKeyAtMouse()
{
	const auto mousePos = ImGui::GetMousePos();
	const auto time = GetTimeFromXPos(mousePos.x);
	const auto value = GetValueFromYPos(mousePos.y);

	for (uint32_t i = 0; i < static_cast<uint32_t>(TrackType::COUNT); i++)
	{
		TrackType trackTypeEnum = static_cast<TrackType>(1 << i);

		if (IsTrackSelected(trackTypeEnum))
		{
			AddKey(time, value, trackTypeEnum, myCurrentSelectedBoneIndex);
		}

	}
}

void AnimationEditor::AddKey(float aTime, float aValue, TrackType aTrackType, int aBoneIndex)
{
	if (!myAnimation->myAdditiveTracks.contains(aBoneIndex))
	{
		myAnimation->myAdditiveTracks[aBoneIndex][aTrackType].Type = aTrackType;
	}
	auto& track = myAnimation->myAdditiveTracks[aBoneIndex][aTrackType];
	//check if a keyframe already exists at this time with tolerance of secondsPerFrame
	const auto tolerance = 1.f / myAnimation->FramesPerSecond;
	const auto it = std::find_if(track.KeyFrames.begin(), track.KeyFrames.end(), [aTime, tolerance](const KeyFrame& aKeyFrame)
		{
			return std::abs(aKeyFrame.Frame - aTime) < tolerance;
		});

	if (it != track.KeyFrames.end())
	{
		//keyframe already exists, replace it
		it->Value = aValue;
	}
	else
	{
		//keyframe doesn't exist, add it
		uint32_t id = myNextID++;
		track.KeyFrames.push_back({ id, aValue, aTime,CurveType::CubicBroken , Utils::Vector2f(-0.05f, 0), Utils::Vector2f(0.05f, 0) });
	}

	//sort the keyframes by time
	std::sort(track.KeyFrames.begin(), track.KeyFrames.end(), [](const KeyFrame& a, const KeyFrame& b) { return a.Frame < b.Frame; });
}

std::optional<KeyFrame*> AnimationEditor::GetKeyWithID(uint32_t aID)
{
	for (auto& bone : myAnimation->myAdditiveTracks)
	{
		for (uint32_t trackType = 0; trackType < static_cast<uint32_t>(TrackType::COUNT); trackType++)
		{
			auto trackTypeEnum = static_cast<TrackType>(std::pow(2, trackType));

			if (bone.second.contains(trackTypeEnum))
			{
				auto& track = bone.second[trackTypeEnum];
				for (auto& keyframe : track.KeyFrames)
				{
					if (keyframe.ID == aID)
					{
						return &keyframe;
					}
				}
			}
		}
	}
	return std::nullopt;
}

void AnimationEditor::SelectKey(KeyFrame& aKey, SelectType aSelectType)
{
	mySelectedType = aSelectType;
	mySelectedID = aKey.ID;
}



float AnimationEditor::FrameToTime(float aFrame)
{
	return aFrame / myAnimation->FramesPerSecond;
}

float AnimationEditor::TimeToFrame(float aTime)
{
	return aTime * myAnimation->FramesPerSecond;
}

float AnimationEditor::GetTimeFromXPos(float aXPos)
{
	auto clamped = Utils::Clamp(aXPos, myTimelineStartPos.x, myTimelineStartPos.x + myTimelineWidth);
	auto fraction = (clamped - myTimelineStartPos.x) / myTimelineWidth;
	return fraction * myFramesDisplayedCount / myAnimation->FramesPerSecond;
}

float AnimationEditor::GetValueFromYPos(float aYPos)
{
	auto clamped = Utils::Clamp(aYPos, myKeyframeGraphStartPos.y, myKeyframeGraphStartPos.y + myKeyframeGraphSize.y);
	auto fraction = (clamped - myKeyframeGraphStartPos.y) / myKeyframeGraphSize.y;
	return fraction * (myValueDisplayMax - myValueDisplayMin) + myValueDisplayMin;
}

float AnimationEditor::GetXPosOnTimeline(float aTime)
{
	const float fraction = aTime / (static_cast<float>(myFramesDisplayedCount) / myAnimation->FramesPerSecond);
	return myTimelineStartPos.x + fraction * myTimelineWidth;
}

float AnimationEditor::GetXPosOnGraph(float aTime)
{
	const float fraction = aTime / (static_cast<float>(myFramesDisplayedCount) / myAnimation->FramesPerSecond);
	return myKeyframeGraphStartPos.x + fraction * myKeyframeGraphSize.x;
}

float AnimationEditor::GetYPosOnGraph(float aValue)
{
	const float mappedValue = (aValue - myValueDisplayMin) / (myValueDisplayMax - myValueDisplayMin);
	return  myKeyframeGraphStartPos.y + mappedValue * myKeyframeGraphSize.y;
}

int AnimationEditor::GetTimelineLineCount()
{
	return myFramesDisplayedCount / myFrameSecondaryStepSize;
}

Utils::BasicTransform AnimationEditor::GetGlobalBoneTransform(int aIndex,
	const Firefly::Frame& aFrame,
	const Firefly::Skeleton& aSkeleton)
{
	Utils::BasicTransform result = aFrame.LocalTransforms[aIndex];
	for (int parent = aSkeleton.Bones[aIndex].Parent; parent >= 0;
		parent = aSkeleton.Bones[parent].Parent)
	{
		result = Utils::BasicTransform::Combine(
			aFrame.LocalTransforms[parent], result);
	}
	return result;
}

bool AnimationEditor::IsTrackSelected(TrackType aTrackType)
{
	return static_cast<bool>(mySelectedTrackTypes & aTrackType);
}

void AnimationEditor::SelectTrack(TrackType aTrackType, bool aAdd)
{
	if (aAdd)
	{
		if (IsTrackSelected(aTrackType))
		{
			mySelectedTrackTypes = mySelectedTrackTypes & ~aTrackType;
		}
		else
		{
			mySelectedTrackTypes = mySelectedTrackTypes | aTrackType;
		}
	}
	else
	{
		mySelectedTrackTypes = aTrackType;
	}
}


