#pragma once
#include "Editor/Windows/EditorWindow.h"
#include "Firefly.h"
#include <Firefly/Rendering/Pipeline/GraphicsPipeline.h>
class MaterialEditorWindow : public EditorWindow
{
public:
	MaterialEditorWindow();
	virtual ~MaterialEditorWindow() = default;

	void OnImGui() override;

	void RenderScene();

	void RenderMaterialProperties();
	
	void OnEvent(Firefly::Event& aEvent) override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<MaterialEditorWindow>(); }
	static std::string GetFactoryName() { return "MaterialEditor"; }
	 std::string GetName() const override { return GetFactoryName(); }
	 static void Reset();

	 void SetMaterial(const std::filesystem::path& aPath);
private:
	static void Refire(const std::string path, Ref<Firefly::MaterialAsset> aMat);
	void ArcBallCamera();

	void RenderMeshMaterial();
	void RenderPostProcessMaterial();
	void RenderDecalMaterial();

	uint32_t myRenderSceneId;
	uint32_t myCullStateIndex;
	uint32_t myDepthStateIndex;
	uint32_t myBlendStateIndex;
	float myBloomStrength = 0.3f;
	Utils::Vector3f myAnkerPoint;
	std::array<std::string, 20> myTexturePath;
	inline static std::filesystem::path myInputPath;
	inline static Ref<Firefly::MaterialAsset> myMaterial;
	std::string myCurrentPrimative;
	size_t myCurrentShader = 0;
	std::string mySkyLightPath;
	float myFOV;
	float myCameraSpeed;
	float myZoomSpeed = 7.f;
	Ref<Firefly::Texture2D> mySkyLight;
	std::filesystem::path myCurrentMesh;
	std::filesystem::path myPipelinePath;
	Ref<Firefly::Camera> myCamera;
	Ref<Firefly::Mesh> myMesh;
	Utils::Transform myTransform;
	bool myShouldRotate;
	float myYawdelta;
	float myPitchDelta;
	float myZoom;

	bool myWaitOnShaderCompletedCompiling = false;

	Firefly::PipelineType myMaterialType;

	Ref<Firefly::Texture2D> mySaveIcon;
};

