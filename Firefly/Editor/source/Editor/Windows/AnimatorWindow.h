#pragma once

#include "EditorWindow.h"

#include "Utils/Math/Matrix3x3.hpp"
#include "Firefly/Asset/Animations/Animator.h"
#include "Firefly/Asset/Animations/AnimatorLayer.h"

namespace Firefly
{
	class AnimatedMesh;
	class MaterialAsset;
}
class EditorCamera;
class AnimatorWindow : public EditorWindow
{
public:
	AnimatorWindow();
	~AnimatorWindow() override = default;

	void OnImGui() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<AnimatorWindow>(); }
	static std::string GetFactoryName() { return "Animator"; }
	std::string GetName() const override { return GetFactoryName(); }
	
	void SetAnimator(std::filesystem::path aPath);

private:
	__forceinline Firefly::AnimatorLayer& GetCurrentLayer();
	__forceinline Firefly::AnimatorState& GetSelectedState();
	__forceinline Firefly::AnimatorTransition& GetSelectedTransition();
	__forceinline Firefly::AnimatorState& GetState(uint64_t aID);
	__forceinline Firefly::AnimatorTransition& GetTransition(uint64_t aID);
	
	enum class LeftPanelTab
	{
		Parameters,
		Layers
	} myLeftPanelTab = LeftPanelTab::Parameters;

	void AcceptAnimationFileToAddState();
	void DrawLeftPanel();
	void DrawLeftPanelTopBar();
	void DrawParametersPanel();
	void OnRightClickParameter(Firefly::AnimatorParameter& aParameter);
	void DrawLayersPanel();


	void DrawViewport();
	void DrawGrid();
	void CameraMovement();
	void DrawStates();
	void OnRightClickState(Firefly::AnimatorState& aState);
	void DragState();

	void DrawTransitionPreview();
	void DrawTransitions();
	std::vector<Firefly::AnimatorTransition> CollectTransitionChunk(uint64_t aFromID, uint64_t aToID);
	void OnRightClickTransition(Firefly::AnimatorTransition& aTransition);
	void OnRightClickTransitionChunk(std::vector<Firefly::AnimatorTransition>& aTransitions);

	void DrawTransitionArrow(Utils::Vector2f aStart, Utils::Vector2f aEnd, float aArrowSize, Utils::Vector4f aColor, bool aTrippleTriangleFlag = false);

	void DrawInspector();
	void DrawStateInspector();
	void DrawTransitionInspector();
	void DrawParameterInstanceInInspector(Firefly::AnimatorParameterInstance& aInstance);
	void DrawInspectorMeshPreview();


	Firefly::AnimatorState& CreateState(Utils::Vector2f aPos, Firefly::AnimatorLayer& aLayer);
	Firefly::AnimatorParameter& CreateParameter(Firefly::AnimatorParameterType aType);
	Firefly::AnimatorTransition& CreateTransition(uint64_t aFromState, uint64_t aToState, Firefly::AnimatorLayer& aLayer);
	Firefly::AnimatorLayer& CreateLayer();
	
	void RemoveState(uint64_t aID);
	void RemoveParameter(uint64_t aID);
	void RemoveTransition(uint64_t aID);
	void RemoveLayer(size_t aIndex);

	void SelectState(uint64_t aID);
	void SelectTransition(uint64_t aID);
	void SelectLayer(size_t aIndex);
	void DeselectAll();
	

	void PushColorAccordingToParameterType(Firefly::AnimatorParameterType aType, int aImGuiCol);

	Utils::Vector2f WorldToScreenSpace(Utils::Vector2f aPos);
	Utils::Vector2f ScreenToWorldSpace(Utils::Vector2f aPos);

	float GetClampedCameraScale();

	uint64_t GenerateRandomID();

	void New();
	void Save(const std::filesystem::path& aPath);
	void SaveAs();
	void DoLoad();

	struct LayerInfo
	{
		uint64_t mySelectedStateID;
		uint64_t mySelectedTransitionID;
	};
	std::vector<LayerInfo> myLayerInfoList;

	std::vector<uint64_t> myStatesMarkedForDelete;
	std::vector<uint64_t> myTransitionsMarkedForDelete;


	ImFont* myTransitionBlendTimeFont;

	std::filesystem::path myCurrentPath = "";
	
	uint64_t mySelectedStateID;
	uint64_t mySelectedTransitionID;

	uint64_t myCurrentLayerIndex;


	Ref<Firefly::Animator> myAnimator;
	Ref<Firefly::Animator> myAnimatorFilePtr;
	
	Ref<Firefly::AnimatedMesh> myPreviewMesh;
	std::vector<Ref<Firefly::MaterialAsset>> myPreviewMeshMaterials;
	Ref<EditorCamera> myEditorCamera;
	

	uint32_t myPreviewRenderSceneID;


	float myStateHeight;
	float myStateWidth;

	float myLeftPanelSize;
	float myInspectorWidth;

	float myMeshPreviewTime;

	Utils::Vector2f myViewportPos;
	Utils::Vector2f myViewportSize;

	Utils::Vector4f myStateColor;
	Utils::Vector4f mySelectedStateColorMultiplier;

	Utils::Vector4f myIntColor;
	Utils::Vector4f myFloatColor;
	Utils::Vector4f myBoolColor;
	Utils::Vector4f myTriggerColor;

	Utils::Vector4f myEntryStateColor;
	Utils::Vector4f myAnyStateColor;

	Utils::Vector4f myViewportBackgroundColor;

	bool myMovingCameraFlag;
	Utils::Vector2f myCameraMovementStartPos;
	//Utils::Vector2f myCameraPos;
	Utils::Matrix3x3<float> myCameraMatrix;

	bool myDraggingStateFlag;
	Utils::Vector2f myStateDragStartPos;

	bool myMakingTransitionFlag = false;
	uint64_t myMakingTransitionStartStateID = 0;
	uint64_t myMakingTransitionEndStateID = 0;

	uint64_t myRenamingTransitionID = 0;
	

};