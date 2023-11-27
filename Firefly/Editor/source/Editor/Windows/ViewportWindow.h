#pragma once
#include "EditorWindow.h"
#include "Firefly/Core/Core.h"

#include "Editor/EditorCamera.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Utils/Math/Matrix3x3.hpp"

namespace Firefly
{
	class Entity;
	class Texture2D;
}

class ViewportWindow : public EditorWindow
{
public:
	ViewportWindow();
	~ViewportWindow() override = default;

	void OnImGui() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<ViewportWindow>(); }
	static std::string GetFactoryName() { return "Viewport"; }
	std::string GetName() const override { return GetFactoryName(); }
	static float GetDragDropDistance() { return myDragDropSpawnDistance; }

	const Ref<EditorCamera>& GetEditorCamera() const { return myEditorCamera; }

	void EnableMousePicking(bool aEnabled = true);

private:
	void DrawPlayButtonWindow();

	bool InputGuizmoMode(ImGuizmo::OPERATION& aOutOperation, ImGuizmo::MODE& aOutMode, Utils::Vector3f& aOutSnapping);
	void OnBeginGuizmo(const ImGuizmo::OPERATION& aOperation);
	void OnEndGuizmo();
	void UpdateGuizmo();
	void HandleSingleObjectGuizmo(const Utils::Matrix4f& aModifiedMatrix, const ImGuizmo::OPERATION& aOperation);
	void HandleMultipleObjectGuizmo(const Utils::Matrix4f& aOriginMatrix, const Utils::Matrix4f& aModifiedMatrix, const ImGuizmo::OPERATION& aOperation);

	void UpdateSquareSelection();
	void UpdateMousePicking();

	void CalcActualPosAndSize();
	void ApplyDebugChanges();

	Utils::Vector3f mySnapSize;
	Utils::Vector3f myRotSnapSize;
	Utils::Vector3f myScaleSnapSize;

	Ref<EditorCamera> myEditorCamera;

	//Editor Icons
	Ref<Firefly::Texture2D> mySettingsIcon;
	Ref<Firefly::Texture2D> myGridEnabledIcon;
	Ref<Firefly::Texture2D> myGridDisabledIcon;
	Ref<Firefly::Texture2D> myDebugLinesEnabledIcon;
	Ref<Firefly::Texture2D> myDebugLinesDisabledIcon;
	Ref<Firefly::Texture2D> myGuizmoEnabledIcon;
	Ref<Firefly::Texture2D> myGuizmoDisabledIcon;
	Ref<Firefly::Texture2D> mySixteenNineEnabledIcon;
	Ref<Firefly::Texture2D> mySixteenNineDisabledIcon;
	Ref<Firefly::Texture2D> myOutlineEnabledIcon;
	Ref<Firefly::Texture2D> myOutlineDisabledIcon;
	Ref<Firefly::Texture2D> myBackIcon;
	Ref<Firefly::Texture2D> myPlayIcon;
	Ref<Firefly::Texture2D> myStopIcon;
	//

	bool myGridEnabled;
	bool myDebugLinesEnabled;
	bool myGuizmoEnabled;
	bool mySixteenNineEnabled;
	bool myOutlineEnabled;
	bool mySettingsCollapsed;
	bool myGuizmoWorldEnabled = false;

	bool myMousePickingEnabled = true;

	uint64_t myLatestClickedEntity = 0;
	bool myIsSquareSelecting = false;
	bool myShouldAddSquareSelected = false;

	bool myIsUsingGuizmo = false;
	bool myJustStartedUsingGuizmo = false;
	std::vector<Utils::BasicTransform> myInitialGuizmoTransforms;

	constexpr static float myDragDropSpawnDistance = 750.0f;
	float mySettingsChildLerpedWidth;
	ImVec2 myActualWindowPos;
	ImVec2 myActualWindowSize;
	ImVec2 myActualMousePos;

	ImVec2 myClickedPos;
};