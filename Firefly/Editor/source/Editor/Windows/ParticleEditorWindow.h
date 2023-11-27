#pragma once
#include <Firefly/Asset/Mesh/Mesh.h>
#include <imgradient/ImGradientHDR.h>

#include "EditorWindow.h"
#include "Editor/EditorCamera.h"

#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/ParticleSystem/ParticleEmitter.h"

namespace Firefly
{
	class Animation;
}

class ParticleEditorWindow : public EditorWindow
{
public:
	ParticleEditorWindow();
	~ParticleEditorWindow() override = default;

	void OnImGui() override;
	void OnEvent(Firefly::Event& aEvent) override;
	void WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam) override;
	
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<ParticleEditorWindow>(); }
	static std::string GetFactoryName() { return "ParticleEditorWindow"; }
	std::string GetName() const override { return GetFactoryName(); }

	void SetEmitter(const std::filesystem::path& aPath);

private:
	void MousePick();
	void SubmitRenderData();
	bool UpdateGuizmo();
	void RenderDebugLines();
	void UpdateProperties(std::string& aTexturePath, std::string& aMaterialPath, EmitterSettings& aSettings);
	void SaveEmitter(const std::filesystem::path& aPath);
	void CalcActualPosAndSize();
	void UpdateAnimatedMeshEmission();

	void ResetEditor();

	const char* myDefaultTexturePath = "Assets/Textures/ParticleStar.dds";

	uint32_t mySceneID;
	Ref<EditorCamera> myEditorCamera;

	Ref<Firefly::ParticleEmitter> myParticleEmitter;

	EmitterSettings myEmitterSettings;
	std::string myTexturePath;
	std::string myMeshPath;
	std::string myMaterialPath = Firefly::DefaultParticleMaterialPath;

	std::string myAnimationPath;
	Ref<Firefly::Animation> myAnimation;
	float myAnimationTime = 0.0f;
	std::vector<Utils::Matrix4f> myBoneTransforms;

	bool myDrawDebugLines = true;
	bool myDrawGrid = true;
	bool myDrawAtmosphere = true;

	Ref<Firefly::Texture2D> myForceFieldBillboard;
	Firefly::BillboardInfo myForceFieldBillboardInfo;

	std::string myInputName;

	/*ForceFieldType myInputForceFieldType = ForceFieldType::Point;
	Utils::Vector3f myInputDirection = Utils::Vector3f(1.0f, 0.0f, 0.0f);
	float myInputForce = 500.0f;
	float myInputRange = 50.0f;
	bool myLerpForceFields = false;
	Utils::LerpType myInputLerpType = Utils::LerpType::Lerp;
	float myInputLerpPower = 2.0f;*/

	int mySelectedForceField = 0;

	ImVec2 myActualWindowPos;
	ImVec2 myActualWindowSize;
	ImVec2 myActualMousePos;

	ImGui::ImGradientHDRTemporaryState myColorGradientEditorState;
};