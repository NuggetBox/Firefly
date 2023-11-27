#pragma once
#include "EditorWindow.h"
#include "Firefly/Core/Core.h"
#include "Utils/Math/BasicTransform.h"
#include "Firefly/Asset/Animation.h"

namespace Firefly
{
	class Mesh;
	class MaterialAsset;
	class Texture2D;
}
class EditorCamera;
class AnimationEditor : public EditorWindow
{
public:
	AnimationEditor();

	static std::string GetFactoryName() { return "AnimationEditor"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<AnimationEditor>(); }

	void SetAnimation(const std::string& aPath);
	void SetAnimation(Ref<Firefly::Animation> aAnimation) {
		myAnimation = aAnimation;
	}



protected:
	void OnImGui() override;

private:
	uint32_t myNextID = 1;
	enum class SelectType
	{
		KeyFrame,
		TangentIn,
		TangentOut
	} mySelectedType;


	void Save();



	void DrawHierarchy();
	void DrawBoneHierarchyRecursive(Firefly::Skeleton& aSkeleton, int aBoneIndex);

	void DrawViewport();
	void HandleGizmo();
	void DrawTimeline();
	void DrawKeyframeGraph();

	void DrawDragKeyData(const Utils::Vector2f& aPos, KeyFrame& aKeyframe);

	std::string FormatTimeToFrameAndTimeText(float aTime);

	void AddKeyAtMouse();
	void AddKey(float aTime, float aValue, TrackType aTrackType, int aBoneIndex);
	//careful when using this as it is a pointer to an element in a vector
	std::optional<KeyFrame*> GetKeyWithID(uint32_t aID);
	void SelectKey(KeyFrame& aKey, SelectType aSelectType);

	//Takes a float since it can be between frames
	float FrameToTime(float aFrame);
	float TimeToFrame(float aTime);

	float GetTimeFromXPos(float aXPos);
	float GetValueFromYPos(float aYPos);
	float GetXPosOnTimeline(float aTime);
	float GetXPosOnGraph(float aTime);
	float GetYPosOnGraph(float aValue);



	int GetTimelineLineCount();

	Utils::BasicTransform GetGlobalBoneTransform(int aIndex, const Firefly::Frame& aFrame, const Firefly::Skeleton& aSkeleton);

	bool myIsFirstFrame;
	bool myIsPlaying;
	bool myWasPlayingWhenStartedUsingGizmo;
	bool myIsDraggingTimeline;
	bool myIsUsingGizmo;
	Ref<Firefly::Animation> myAnimation;
	Utils::BasicTransform myInitialGizmoLocalTransform;
	Utils::BasicTransform myInitialGizmoGlobalTransform;


	int myAnimationFrameCount;
	int myFramesDisplayedCount;
	int myFrameStepSize;
	int myFrameSecondaryStepSize;

	int myValueDisplayMin;
	int myValueDisplayMax;
	int myValueDisplayStep;
	Utils::Vector2f myKeyframeGraphStartPos;
	Utils::Vector2f myKeyframeGraphSize;
	Utils::Vector2f myTimelineStartPos;
	float myTimelineWidth;


	const float myTimelineHeight = 30;
	const float myTimelineHandleWidth = 20;
	const float myTimelineHandleHeight = 30;
	const float myKeyframeRadius = 5.0f;
	const float myCurveLineThickness = 3.f;
	const Utils::Vector4f myKeyColor= { 1.0f, 1.0f, 1.0f, 1.0f };
	const Utils::Vector4f myKeySelectedColor = { 0.2f, 0.25f, 0.84f, 1.0f };
	const Utils::Vector4f myKeyframeGraphLineColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	const Utils::Vector4f myKeyframeGraphBackgroundColor = { 0.1f, 0.1f, 0.1f , 1.f };
	const Utils::Vector4f myTimelineBackgroundColor = { 0.05f, 0.05f, 0.05f , 1.f };
	const Utils::Vector4f myTimelineHandleColor = { 146.f / 255.f,63.f / 255.f,63.f / 255.f,1 };
	const Utils::Vector4f myCurveColor = { 230.f / 255.f,10.f / 255.f,15.f / 255.f,1 };



	//bone index, frame, keyframe

	TrackType mySelectedTrackTypes;
	bool IsTrackSelected(TrackType aTrackType);
	void SelectTrack(TrackType aTrackType, bool aAdd);


	Ref<Firefly::Mesh> myPyramidMesh;
	Ref<Firefly::Mesh> myCubeMesh;
	Ref<Firefly::Mesh> mySphereMesh;

	Ref<Firefly::MaterialAsset> myGroundMaterial;
	Ref<Firefly::MaterialAsset> myBoneMaterial;

	Ref<Firefly::Texture2D> myPlayButtonTexture;
	Ref<Firefly::Texture2D> myPauseButtonTexture;

	Ref<EditorCamera> myEditorCamera;
	uint32_t myPreviewRenderSceneID;

	float myTime;

	int myCurrentSelectedBoneIndex = -1;


	uint32_t mySelectedID;
	bool myIsDragging;

	bool myHasInitialized = false;
};





