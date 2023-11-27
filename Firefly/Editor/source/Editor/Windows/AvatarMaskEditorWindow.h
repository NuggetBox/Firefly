#pragma once
#include "Firefly/Core/Core.h"
#include "Editor/Windows/EditorWindow.h"
#include "Editor/EditorCamera.h"
namespace Firefly
{
	class AvatarMask;
	class Mesh;
	class MaterialAsset;
	struct Skeleton;
}
class AvatarMaskEditorWindow : public EditorWindow
{
public:
	AvatarMaskEditorWindow();
	virtual ~AvatarMaskEditorWindow() = default;
	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<AvatarMaskEditorWindow>(); }
	static std::string GetFactoryName() { return "AvatarMaskEditorWindow"; }
	std::string GetName() const override { return GetFactoryName(); }

	void SetAvatarMask(Ref<Firefly::AvatarMask> aAvatarMask) { myAvatarMask = aAvatarMask; }
	void SetAvatarMask(const std::string& aAvatarMaskPath);
	
private:
	void Save();
	void Load();

	void DrawInfoPanel();
	void DrawViewport();

	void DrawBoneHierarchyRecursive(Firefly::Skeleton& aSkeleton, int aBoneIndex);

	Ref<Firefly::AvatarMask> myAvatarMaskPtr;
	Ref<Firefly::AvatarMask> myAvatarMask;
	Ref<Firefly::Mesh> myPyramidMesh;
	Ref<Firefly::MaterialAsset> myDeactivatedMaterial;
	Ref<Firefly::MaterialAsset> myGreenMaterial;
	Ref<Firefly::MaterialAsset> myOrangeMaterial;
	Ref<Firefly::MaterialAsset> myRedMaterial;
	Ref<Firefly::MaterialAsset> myActivatedMaterial;
	

	uint32_t myPreviewRenderSceneID;
	Ref<EditorCamera> myEditorCamera;

	uint32_t myCurrentSelectedBoneIndex;
	
	bool myIsFirstFrame;
};