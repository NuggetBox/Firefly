#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include <Firefly/Rendering/RenderCommands.h>
#include "Firefly/Rendering/Postprocess/PostprocessManager.h"


class PostProcessComponent : public Firefly::Component
{
	friend class Firefly::PostprocessManager;
public:
	PostProcessComponent();
	~PostProcessComponent();

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

	Ref<Firefly::MaterialAsset> GetCustomPostprocess0() { return myPostprocessPass0; }
	Ref<Firefly::MaterialAsset> GetCustomPostprocess1() { return myPostprocessPass1; }
	Ref<Firefly::MaterialAsset> GetCustomPostprocess2() { return myPostprocessPass2; }
	Ref<Firefly::MaterialAsset> GetCustomPostprocess3() { return myPostprocessPass3; }

	static std::string GetFactoryName() { return "PostProcessComponent"; }
	static Ref<Component> Create() { return CreateRef<PostProcessComponent>(); }

private:
	void SetData();

	bool myIsGlobal = true;
	float myLerpSize;
	Firefly::PostProcessInfo myInfo;
	Vector4f myColor;
	float myThreshHold;
	bool myEnableSun;
	float myWindSpeed;
	float myFogWaveFrekvency;
	Vector4f myEditorOutlineColor;
	Vector4f myEnemyOutlineColor;
	float myVignetteX;
	float myVignetteY;
	bool myShouldBePostProcessing;
	bool myShouldBeFog;
	bool myLockFog;
	float myFogHeightOffsetOnLoad = 0.0f;
	bool myShouldBeVignette;
	float myBloomStrength;
	float myBloomThreshhold;
	bool myEnableLUT = false;
	std::string myLUTtexturePath;
	Vector4f mySaturation;
	Vector4f myConstrast;
	Vector4f myGamma;
	Vector4f myGain;
	Vector4f myIntensities;
	bool myEnableSaturation;
	bool myEnableContrast;
	bool myEnableGamma;
	bool myEnableGain;
	float myLerpPosition;
	float myLerpSpeed;
	Vector4f myEnvironmentFogColor;
	float myEnvironmentFogIntensity = 0.f;
	Vector4f myGodRaysColor;
	float myGodRaysIntensity = 1.f;
	bool myEnableVolumetricFog;
	float myVolumetricFogDensity = 0.01f;
	float myVolumetricFogPhase = 0.670f;
	float myVolumetricFogDepthPow = 1.0f;
	float myVolumetricFogSkyInterction = 1.0f;
	Vector4f mySSAOSettings;
	Ref<Firefly::Texture2D> myLUT;

	std::string myPostprocessPath0;
	std::string myPostprocessPath1;
	std::string myPostprocessPath2;
	std::string myPostprocessPath3;
	
	Ref<Firefly::MaterialAsset> myPostprocessPass0;
	Ref<Firefly::MaterialAsset> myPostprocessPass1;
	Ref<Firefly::MaterialAsset> myPostprocessPass2;
	Ref<Firefly::MaterialAsset> myPostprocessPass3;
	size_t myPostprocessID = 0;
};

