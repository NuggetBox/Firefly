#include "EditorPch.h"
#include "AvatarMaskEditorWindow.h"
#include "Firefly/Asset/Animations/AvatarMask.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Material/MaterialAsset.h"
#include "Firefly/Asset/ResourceCache.h"
#include "imgui/imgui_internal.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"

REGISTER_WINDOW(AvatarMaskEditorWindow);

AvatarMaskEditorWindow::AvatarMaskEditorWindow()
	: EditorWindow("Avatar Mask Editor")
{
	myIsFirstFrame = true;
	myCurrentSelectedBoneIndex = 0;

	myPreviewRenderSceneID = Firefly::Renderer::InitializeScene();

	myPyramidMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("PyramidBottomPivot");
	myDeactivatedMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AvatarMask/AvatarMaskDeactivatedMat.mat");
	myActivatedMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AvatarMask/AvatarMaskActivatedMat.mat");
	myGreenMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AvatarMask/AvatarMaskGreenMat.mat");
	myOrangeMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AvatarMask/AvatarMaskOrangeMat.mat");
	myRedMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Editor/AvatarMask/AvatarMaskRedMat.mat");

	myEditorCamera = CreateRef<EditorCamera>();
	myEditorCamera->Initialize(Firefly::CameraInfo());

	myWindowFlags |= ImGuiWindowFlags_MenuBar;

}

void AvatarMaskEditorWindow::OnImGui()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::Button("Save"))
		{
			Save();
		}
		ImGui::EndMenuBar();
	}

	if (!myAvatarMask)
	{
		ImGui::Text("No Avatar Mask Loaded, load one using the content browser or drag and drop it here.");
		if (auto* payload = ImGuiUtils::DragDropWindow("FILE", "Avatar Mask", ".Mask"))
		{
			std::string path = reinterpret_cast<const char*>(payload->Data);
			SetAvatarMask(path);
		}
		return;
	}
	if (!myAvatarMask->IsLoaded())
	{
		ImGui::Text("Avatar Mask is not loaded yet, please wait until it is loaded.");
		return;
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

		ImGui::DockBuilderDockWindow("Viewport", dockMainID);
		ImGui::DockBuilderDockWindow("InfoPanel", dockLeftID);

		ImGui::DockBuilderFinish(dockspaceID);
	}
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar);




	ImGui::Begin("InfoPanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawInfoPanel();
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
	DrawViewport();
	ImGui::End();
	ImGui::PopStyleVar();

	if (myIsFirstFrame)
	{
		myIsFirstFrame = false;
		ImGui::DockBuilderRemoveNode(ImGui::GetID("BlendspaceWindowDockspace"));
	}





	return; // dont run the rest
	ImGui::Text("Avatar Mask Loaded: %s", myAvatarMask->GetPath().string().c_str());
	uint32_t index = 0;
	auto animatedMesh = myAvatarMask->GetAnimatedMesh();
	ImGui::BeginTable("AvatarMaskWindowTable", 2, ImGuiTableFlags_SizingFixedFit);
	for (auto& bone : animatedMesh->GetSkeleton().Bones)
	{
		ImGui::TableNextColumn();
		bool dontIgnore = !myAvatarMask->GetBonesToIgnore().contains(index);
		if (ImGui::Checkbox(("Bone: " + bone.Name + " ##" + std::to_string(index)).c_str(), &dontIgnore))
		{
			if (!dontIgnore)
			{
				myAvatarMask->AddBoneToIgnore(index);
			}
			else
			{
				myAvatarMask->RemoveBoneToIgnore(index);
			}
		}
		ImGui::TableNextColumn();

		if (dontIgnore)
		{
			ImGui::SetNextItemWidth(100);
			int influence = myAvatarMask->GetInfluence(index) * 100.f;
			if (ImGui::SliderInt(("%## Influence" + std::to_string(index)).c_str(), &influence, 0, 100))
			{
				myAvatarMask->SetInfluence(index, influence / 100.f);
			}
		}
		index++;
	}
	ImGui::EndTable();
	if (ImGui::Button("Save"))
	{
		Save();
	}




}

void AvatarMaskEditorWindow::SetAvatarMask(const std::string& aAvatarMaskPath)
{
	myAvatarMaskPtr = Firefly::ResourceCache::GetAsset<Firefly::AvatarMask>(aAvatarMaskPath, true);

	myAvatarMask = CreateRef<Firefly::AvatarMask>(*myAvatarMaskPtr);
}

void AvatarMaskEditorWindow::Save()
{
	auto fileStatus = std::filesystem::status(myAvatarMask->GetPath());
	bool hasWritePermission = (fileStatus.permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

	if (!hasWritePermission)
	{
		ImGuiUtils::NotifyErrorLocal("You do not have write permission to file at path: {}", myAvatarMaskPtr->GetPath().string().c_str());
		return;
	}
	*myAvatarMaskPtr = *myAvatarMask;
	myAvatarMaskPtr->SaveTo(myAvatarMaskPtr->GetPath());

	ImGuiUtils::NotifySuccessLocal("Saved Avatar Mask to path: {}", myAvatarMaskPtr->GetPath().string().c_str());

}

void AvatarMaskEditorWindow::DrawInfoPanel()
{
	ImGui::BeginChild("Inspector##AvatarMaskEditorWindow", ImVec2(0, 50), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		if (ImGui::BeginTable("AvatarMaskWindowTable", 2, ImGuiTableFlags_SizingFixedFit))
		{

			ImGui::TableNextColumn();
			auto bone = myAvatarMask->GetAnimatedMesh()->GetSkeleton().Bones[myCurrentSelectedBoneIndex];
			bool dontIgnore = !myAvatarMask->GetBonesToIgnore().contains(myCurrentSelectedBoneIndex);
			if (ImGui::Checkbox(("Bone: " + bone.Name + " ##" + std::to_string(myCurrentSelectedBoneIndex)).c_str(), &dontIgnore))
			{
				if (!dontIgnore)
				{
					myAvatarMask->AddBoneToIgnore(myCurrentSelectedBoneIndex);
				}
				else
				{
					myAvatarMask->RemoveBoneToIgnore(myCurrentSelectedBoneIndex);
				}
			}
			ImGui::TableNextColumn();

			if (dontIgnore)
			{
				ImGui::SetNextItemWidth(100);
				int influence = myAvatarMask->GetInfluence(myCurrentSelectedBoneIndex) * 100.f;
				if (ImGui::SliderInt(("%## Influence" + std::to_string(myCurrentSelectedBoneIndex)).c_str(), &influence, 0, 100))
				{
					myAvatarMask->SetInfluence(myCurrentSelectedBoneIndex, influence / 100.f);
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();

	//hierarchy
	int parentIndex = 0;
	auto skeleton = myAvatarMask->GetAnimatedMesh()->GetSkeleton();
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	DrawBoneHierarchyRecursive(skeleton, parentIndex);
}
void AvatarMaskEditorWindow::DrawBoneHierarchyRecursive(Firefly::Skeleton& aSkeleton, int aBoneIndex)
{
	auto& bone = aSkeleton.Bones[aBoneIndex];

	////find all children and their children etc
	//std::vector<int> childrenRecurs;
	//std::vector<int> childrenRecursTemp;
	//while(childrenRecursTemp.size() > 0)
	//{
	//	auto currChild = childrenRecursTemp.back();
	//	childrenRecurs.push_back(currChild);
	//	childrenRecursTemp.pop_back();
	//	for (auto child : aSkeleton.Bones[currChild].Children)
	//	{
	//		childrenRecursTemp.push_back(child);
	//	}
	//}


	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (bone.Children.size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (myCurrentSelectedBoneIndex == aBoneIndex)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	const bool ignored = myAvatarMask->GetBonesToIgnore().contains(aBoneIndex);
	ImVec4 boneColor = { 1,1,1,1 };
	if (ignored)
	{
		boneColor = { boneColor.x * 0.7f, boneColor.y * 0.7f,boneColor.z * 0.7f,boneColor.w };
	}

	ImGui::PushStyleColor(ImGuiCol_Text, boneColor);
	const bool open = ImGui::TreeNodeEx(bone.Name.c_str(), flags);
	if (ImGui::IsItemClicked())
	{
		myCurrentSelectedBoneIndex = aBoneIndex;
	}
	ImGui::PopStyleColor();

	const auto influence = myAvatarMask->GetInfluence(aBoneIndex);
	if (influence <= 0.99f && !ignored)
	{
		ImGui::SameLine();
		ImVec4 color = { 0.1f,1,0.1f,1 };
		//red when below 30%
		if (influence < 0.3f)
		{
			color = { 1.f,0.1f,0.2f, 1.f };
		}
		//Orange when below 60%
		else if (influence < 0.6f)
		{
			color = { 1.f, 0.9f,0.f, 1.f };
		}

		ImGui::TextColored(color, (std::to_string(static_cast<int>(std::roundf(influence * 100.f))) + "%").c_str());
	}


	if (open)
	{
		for (int i = 0; i < aSkeleton.Bones[aBoneIndex].Children.size(); i++)
		{
			DrawBoneHierarchyRecursive(aSkeleton, aSkeleton.Bones[aBoneIndex].Children[i]);
		}
		ImGui::TreePop();
	}

}

void AvatarMaskEditorWindow::DrawViewport()
{
	if (!myPyramidMesh->IsLoaded())
	{
		return;
	}
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
	pack.Direction = { -0.71, 0.71, -0.71, 1 };
	pack.ColorAndIntensity = { 1,1,1,1 };
	pack.dirLightInfo.x = 0;
	Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res1024);



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

	auto mesh = myAvatarMask->GetAnimatedMesh();
	auto& skeleton = mesh->GetSkeleton();

	auto& bones = skeleton.Bones;

	std::vector<Utils::Vector3f> worldPositions;
	for (auto& bone : bones)
	{
		auto bindPose = Utils::Matrix4f::GetInverse(bone.BindPoseInverse).GetTranspose();
		Utils::Vector3f pos;
		Utils::Quaternion rot;
		Utils::Vector3f throwaway;
		Utils::Matrix4f::Decompose(bindPose, pos, rot, throwaway);
		worldPositions.push_back(pos);
	}

	for (int i = 0; i < worldPositions.size(); i++)
	{
		if (i == 0)
		{
			int apa = 0;
		}
		auto parentIndex = bones[i].Parent;
		auto lookRot = Utils::Quaternion();
		float length = 1;
		auto pos = Utils::Vector3f();
		if (parentIndex != -1)
		{
			auto dir = worldPositions[i] - worldPositions[parentIndex];
			pos = worldPositions[parentIndex];
			length = dir.Length();
			dir.Normalize();
			lookRot = Utils::Quaternion::CreateLookRotation(dir, Utils::Vector3f::Up());
			lookRot = lookRot * Utils::Quaternion::CreateFromEulerAngles({ 90,0,0 });
		}
		/*else
		{
			length = worldPositions[0].Length();
		}*/
		Firefly::MeshSubmitInfo mdl(Utils::Transform(pos, lookRot, Utils::Vector3f(0.01f, length / 100.f, 0.01f)).GetMatrix());
		mdl.Mesh = &myPyramidMesh->GetSubMeshes()[0];
		mdl.Outline = i == myCurrentSelectedBoneIndex;
		mdl.EntityID = static_cast<uint64_t>(i);

		mdl.Material = myActivatedMaterial;
		if (myAvatarMask->GetBonesToIgnore().contains(i))
		{
			mdl.Material = myDeactivatedMaterial;
		}
		else
		{
			auto influence = myAvatarMask->GetInfluence(i);
			//red when below 30%
			if (influence < 0.3f)
			{
				mdl.Material = myRedMaterial;
			}
			//Orange when below 60%
			else if (influence < 0.6f)
			{
				mdl.Material = myOrangeMaterial;
			}
			else if (influence <= 0.99f)
			{
				mdl.Material = myGreenMaterial;
			}
		}

		Firefly::Renderer::Submit(mdl);
	}





	for (auto& submesh : mesh->GetSubMeshes())
	{

		/*Firefly::MeshSubmitInfo mdl(&submesh,
			Utils::Transform(Utils::Vector3f::Zero(), Utils::Quaternion(), Utils::Vector3f::One()).GetMatrix(), std::vector<Utils::Matrix4f>());
		Firefly::Renderer::Submit(mdl);*/
	}
	const auto initialWindowPos = ImGui::GetWindowPos();
	const auto initialMousePos = ImGui::GetMousePos();
	Utils::Vector2f mousePos = Utils::Vector2f(initialMousePos.x - initialWindowPos.x, initialMousePos.y - initialWindowPos.y);
	LOGINFO("MousePos: {}, {}", mousePos.x, mousePos.y);

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
	{
		myCurrentSelectedBoneIndex = static_cast<uint32_t>(Firefly::Renderer::GetEntityFromScreenPos(mousePos.x, mousePos.y));
	}

	Firefly::Renderer::EndScene();

	auto frame = Firefly::Renderer::GetSceneFrameBuffer(myPreviewRenderSceneID);
	ImGui::Image(frame->GetColorAttachment(0).Get(), { contentRegionAvail.x , contentRegionAvail.y });
	//find mouse pos relative to image



}

