#pragma once
#include "Firefly/Core/Core.h"
#include "Editor/Windows/EditorWindow.h"
#include "Utils/Math/Vector2.hpp"
#include "Firefly/Asset/Animations/BlendSpace.h"

#include "Utils/Math/Transform.h"

namespace Firefly
{
	class AnimatedMesh;
	class Mesh;
	class Animation;
	class MaterialAsset;
	class Texture2D;
	class BlendSpace;
}
class EditorCamera;
class BlendSpaceEditorWindow : public EditorWindow
{
public:
	BlendSpaceEditorWindow();
	virtual ~BlendSpaceEditorWindow() = default;

	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<BlendSpaceEditorWindow>(); }
	static std::string GetFactoryName() { return "Blendspace Editor"; }
	std::string GetName() const override { return GetFactoryName(); }

	void SetBlendSpace(const std::filesystem::path& aPath);
	void SetBlendSpace(const Ref<Firefly::BlendSpace>& aBlendSpace);


private:

	void Save();
	void DrawInspector();
	void DrawBlendspaceViewSettings();
	void DrawBlendspaceView();
	void DrawViewport();
	void DrawTimeline();

	void Triangulate();
	
	Utils::Vector2f GetMouseGridPosition();
	Utils::Vector2f ScreenToGridPosition(Utils::Vector2f aScreenPosition);
	Utils::Vector2f GridToScreenPosition(Utils::Vector2f aGridPosition);

	bool myIsFirstFrame = true;
	bool myIsPlaying = false;
	bool myIsDraggingTimeline = false;
	bool myIsDraggingPoint = false;

	const float myBlendSpaceViewSettingsHeight = 40.0f;
	const float myTimelineHeight = 50.0f;
	const float myEntryDiamondRadius = 5;
	const int myTimelineButtonCount = 1;

	int mySelectedPointIndex = -1;

	float myTime;
	float myMaxTime;
	Utils::Vector2f myPreviewGridPosition;
	Utils::Vector2f myBlendSpaceGridPos;
	Utils::Vector2f myBlendSpaceGridSize;
	Utils::Vector2f myStartDragMousePos;

	Ref<Firefly::Mesh> myCubeMesh;
	Ref<Firefly::BlendSpace> myBlendSpace;

	Utils::Transform myCubeTransform;
	
	const char* myBlendspaceTypesStrings[2];
	
	uint32_t myPreviewRenderSceneID;
	Ref<EditorCamera> myEditorCamera;

	std::vector<Ref<Firefly::MaterialAsset>> myPreviewMeshMaterials;

	Ref<Firefly::Texture2D> myPlayButtonTexture;
	Ref<Firefly::Texture2D> myPauseButtonTexture;



};