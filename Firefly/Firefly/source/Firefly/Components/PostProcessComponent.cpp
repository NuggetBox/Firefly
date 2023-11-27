#include "FFpch.h"
#include "PostProcessComponent.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Rendering/Renderer.h"
#include <Firefly/Application/Application.h>
#include <Firefly/Asset/ResourceCache.h>
#include <Utils/Timer.h>


REGISTER_COMPONENT(PostProcessComponent);

PostProcessComponent::PostProcessComponent() : Firefly::Component("PostProcessComponent")
{
	myColor = { 1,1,1,1 };
	myEnableSun = true;
	myThreshHold = 100;
	myWindSpeed = 0;
	myFogWaveFrekvency = 0;
	myVignetteX = 0;
	myVignetteY = 0;
	myBloomStrength = 0.1f;
	myBloomThreshhold = 0.f;
	myEnableLUT = false;
	myInfo = {};
	myInfo.passes |= Firefly::PostProcessPass::Vignette;
	myLerpSpeed = 1.f;
	myEnableSaturation = false;
	myEnableContrast = false;
	myEnableGamma = false;
	myEnableGain = false;
	myShouldBeFog = false;
	myGodRaysColor = 1.f;
	myEnvironmentFogColor = 1.f;
	myLerpSize = 0;

	EditorVariable("Is Global", Firefly::ParameterType::Bool, &myIsGlobal);
	EditorVariable("Start FadeOff", Firefly::ParameterType::Float, &myLerpSize);
	EditorVariable("enable postprocessing", Firefly::ParameterType::Bool, &myShouldBePostProcessing);
	//EditorVariable("SSAO Settings",				Firefly::ParameterType::Vec4,  &mySSAOSettings);
	EditorVariable("enable fog", Firefly::ParameterType::Bool, &myShouldBeFog);
	EditorHeader("Fog");
	EditorVariable("Fog color", Firefly::ParameterType::Color, &myColor);
	EditorVariable("Fog threshold", Firefly::ParameterType::Float, &myThreshHold);
	EditorVariable("Use sun color", Firefly::ParameterType::Bool, &myEnableSun);
	EditorEndHeader();
	EditorHeader("Volumetric Fog");
	EditorVariable("Enable Volumetric fog", Firefly::ParameterType::Bool, &myEnableVolumetricFog);
	EditorVariable("Ambient fog Color", Firefly::ParameterType::Color, &myEnvironmentFogColor, false);
	EditorVariable("Ambient fog intensity", Firefly::ParameterType::Float, &myEnvironmentFogIntensity);
	EditorVariable("Godray Color", Firefly::ParameterType::Color, &myGodRaysColor, false);
	EditorVariable("Godray intensity", Firefly::ParameterType::Float, &myGodRaysIntensity);
	EditorVariable("Fog Density", Firefly::ParameterType::Float, &myVolumetricFogDensity);
	EditorVariable("Fog Phase", Firefly::ParameterType::Float, &myVolumetricFogPhase);
	EditorVariable("Fog Depth power", Firefly::ParameterType::Float, &myVolumetricFogDepthPow);
	EditorVariable("Fog Interaction Procentage", Firefly::ParameterType::Float, &myVolumetricFogSkyInterction);
	EditorEndHeader();

	EditorVariable("enable vignette", Firefly::ParameterType::Bool, &myShouldBeVignette);
	EditorHeader("Vignette");
	EditorVariable("Vignette color", Firefly::ParameterType::Color, &myEditorOutlineColor, false);
	EditorVariable("Vignette cutoff", Firefly::ParameterType::Float, &myVignetteX);
	EditorVariable("Vignette fadeoff", Firefly::ParameterType::Float, &myVignetteY);
	EditorEndHeader();

	EditorHeader("Bloom");
	EditorVariable("Bloom strength", Firefly::ParameterType::Float, &myBloomStrength);
	EditorVariable("Bloom threshold", Firefly::ParameterType::Float, &myBloomThreshhold);
	EditorEndHeader();

	EditorHeader("Color correction");
	EditorVariable("Enable Saturation", Firefly::ParameterType::Bool, &myEnableSaturation);
	EditorVariable("Saturation", Firefly::ParameterType::Color, &mySaturation);
	EditorVariable("Saturation Intensity", Firefly::ParameterType::Float, &myIntensities.x);
	EditorVariable("Enable Constrast", Firefly::ParameterType::Bool, &myEnableContrast);
	EditorVariable("Constrast", Firefly::ParameterType::Color, &myConstrast);
	EditorVariable("Constrast Intensity", Firefly::ParameterType::Float, &myIntensities.y);
	EditorVariable("Enable Gamma", Firefly::ParameterType::Bool, &myEnableGamma);
	EditorVariable("Gamma", Firefly::ParameterType::Color, &myGamma);
	EditorVariable("Gamma Intensity", Firefly::ParameterType::Float, &myIntensities.z);
	EditorVariable("Enable Gain", Firefly::ParameterType::Bool, &myEnableGain);
	EditorVariable("Gain", Firefly::ParameterType::Color, &myGain);
	EditorVariable("Gain Intensity", Firefly::ParameterType::Float, &myIntensities.w);
	EditorVariable("enable cc LUT", Firefly::ParameterType::Bool, &myEnableLUT);
	EditorVariable("LUT Texture", Firefly::ParameterType::File, &myLUTtexturePath, ".dds");
	EditorEndHeader();

	EditorHeader("Custom Postprocess materials");
	EditorVariable("Postprocess0", Firefly::ParameterType::File, &myPostprocessPath0, ".mat");
	EditorVariable("Postprocess1", Firefly::ParameterType::File, &myPostprocessPath1, ".mat");
	EditorVariable("Postprocess2", Firefly::ParameterType::File, &myPostprocessPath2, ".mat");
	EditorVariable("Postprocess3", Firefly::ParameterType::File, &myPostprocessPath3, ".mat");
	EditorEndHeader();
}

PostProcessComponent::~PostProcessComponent()
{
}

void PostProcessComponent::Initialize()
{
	SetData();
}

void PostProcessComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(PostProcessComponent::OnUpdateEvent));
	dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](Firefly::EntityPropertyUpdatedEvent& e)
		{
			SetData();
			if (!myLUTtexturePath.empty() && e.GetParamName() == "LUT Texture")
			{
				myLUT = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(myLUTtexturePath);
			}

			if (!myPostprocessPath0.empty() && e.GetParamName() == "Postprocess0")
			{
				myPostprocessPass0 = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>(myPostprocessPath0);
			}
			else if (!myPostprocessPath1.empty() && e.GetParamName() == "Postprocess1")
			{
				myPostprocessPass1 = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>(myPostprocessPath1);
			}
			else if (!myPostprocessPath2.empty() && e.GetParamName() == "Postprocess2")
			{
				myPostprocessPass2 = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>(myPostprocessPath2);
			}
			else if (!myPostprocessPath3.empty() && e.GetParamName() == "Postprocess3")
			{
				myPostprocessPass3 = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>(myPostprocessPath3);
			}
			return false;
		});
}


bool PostProcessComponent::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
{
	Firefly::PostprocessManager::RemovePostProcess(myPostprocessID);

	if (!myIsGlobal)
	{
		Firefly::Renderer::SubmitDebugCuboid(myEntity->GetTransform().GetPosition(), myEntity->GetTransform().GetScale());
		Firefly::Renderer::SubmitDebugCuboid(myEntity->GetTransform().GetPosition(), myEntity->GetTransform().GetScale() + Utils::Vec3(myLerpSize), Utils::Vec4(0, 1, 0, 1));
	}

	myInfo.Enable = Firefly::Application::Get().GetIsInPlayMode();
	if (myShouldBePostProcessing)
	{
		if (Firefly::Renderer::GetActiveCamera())
		{
			myLerpPosition = Utils::Lerp(myLerpPosition, Firefly::Renderer::GetActiveCamera()->GetTransform().GetPosition().y, myLerpSpeed * Utils::Timer::GetDeltaTime());
			myInfo.Data.LogFog.z = myLerpPosition;
			myInfo.passes = Firefly::PostProcessPass::ToneMapping;

			if (myShouldBeFog)
			{
				myInfo.passes |= Firefly::PostProcessPass::Fog;
			}
			if (myShouldBeVignette)
			{
				myInfo.passes |= Firefly::PostProcessPass::Vignette;
			}
		}
	}
	else
	{
		myInfo.passes = Firefly::PostProcessPass::ToneMapping;
	}
	myPostprocessID = Firefly::PostprocessManager::AddPostProcess(GetComponent<PostProcessComponent>());

	return false;
}

void PostProcessComponent::SetData()
{
	if (Firefly::Application::Get().GetIsInPlayMode())
	{
		myInfo.Data.OutlineColor = { 1,0,0,1 };
	}
	else
	{
		myInfo.Data.OutlineColor = { 1,0.5f,0,1 };
	}

	// fog settings
	myInfo.Data.OutlineColor = myEditorOutlineColor;
	myInfo.Data.LogFog.x = (float)myLockFog;
	myInfo.Data.LogFog.y = myLerpSpeed;
	myInfo.Data.LogFog.z = myLerpPosition;
	myInfo.Data.FogColor = myColor;
	myInfo.Data.Fogthreshold = myThreshHold;
	myInfo.Data.FogDensity = myEnableSun;
	myInfo.Data.WindSpeed = myWindSpeed;
	myInfo.Data.FogWaveFrekvency = myFogWaveFrekvency;

	myInfo.Data.SSAOSettings = mySSAOSettings;

	// vignette settings
	myInfo.Data.Padding.x = myVignetteX;
	myInfo.Data.Padding.y = myVignetteY;

	// bloom settings
	myInfo.Data.Padding.z = myBloomStrength;

	// color correction
	myInfo.Data.Saturation = mySaturation;
	myInfo.Data.Constrast = myConstrast;
	myInfo.Data.Gamma = myGamma;
	myInfo.Data.Gain = myGain;
	myInfo.Data.Intensities = myIntensities;
	myInfo.Data.Enables.x = (float)myEnableSaturation;
	myInfo.Data.Enables.y = (float)myEnableContrast;
	myInfo.Data.Enables.z = (float)myEnableGamma;
	myInfo.Data.Enables.w = (float)myEnableGain;
	myInfo.Data.EnableLUT = static_cast<float>(myEnableLUT);
}
