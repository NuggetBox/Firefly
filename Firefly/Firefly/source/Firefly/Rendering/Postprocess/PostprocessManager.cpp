#include "FFpch.h"
#include "PostprocessManager.h"

#include "Firefly/Rendering/RenderCommands.h"
#include <Firefly/Rendering/Renderer.h>
#include "Firefly/Components/PostProcessComponent.h"
namespace Firefly
{
	size_t PostprocessManager::AddPostProcess(Ptr<PostProcessComponent> aInfo)
	{
		myID++;
		myPostprocessMap[myID] = aInfo;
		return myID;
	}

	void PostprocessManager::RemovePostProcess(size_t id)
	{
		if (myPostprocessMap.contains(id))
		{
			myPostprocessMap.erase(id);
		}
	}

	void PostprocessManager::FindPostProcessValue()
	{
		Ref<PostProcessComponent> firstPosition = nullptr;
		Ref<PostProcessComponent> secondPosition = nullptr;
		if (!Renderer::GetActiveCamera())
		{
			return;
		}
		Utils::Vec3 camPosition = Renderer::GetActiveCamera()->GetTransform().GetPosition();
		float lerpFactor = 0.f;
		for (auto& [id, postprocessComponent] : myPostprocessMap)
		{
			if (postprocessComponent.expired() == false)
			{

				if (postprocessComponent.lock()->myIsGlobal)
				{
					firstPosition = postprocessComponent.lock();
					continue;
				}

				auto post = postprocessComponent.lock();
				auto& trans = post->GetEntity()->GetTransform();
				Utils::AABB3D<float> aabb;
				Utils::AABB3D<float> outerAabb;
				aabb.InitWithMinAndMax(trans.GetPosition() - (trans.GetScale() / 2.f), trans.GetPosition() + (trans.GetScale() / 2.f));
				outerAabb.InitWithMinAndMax(trans.GetPosition() - ((trans.GetScale() + Utils::Vec3(post->myLerpSize)) / 2.f), trans.GetPosition() + ((trans.GetScale() + Utils::Vec3(post->myLerpSize)) / 2.f));

				lerpFactor = aabb.GetDistanceFromAABB(camPosition);

				if (outerAabb.IsInside(camPosition))
				{
					secondPosition = postprocessComponent.lock();

					if (aabb.IsInside(camPosition))
					{
						lerpFactor = 1.f;
						continue;
					}
					auto innerDistance = aabb.GetDistanceFromAABB(camPosition);

					lerpFactor = innerDistance / post->myLerpSize;
					lerpFactor = 1.f - lerpFactor;
				}
			}
		}
		if (firstPosition == nullptr)
		{
			return;
		}
		if (secondPosition == nullptr)
		{
			secondPosition = firstPosition;
		}

		LerpAndSubmit(firstPosition, secondPosition, lerpFactor);
		myPostprocessMap.clear();
	}

	void PostprocessManager::OverrideVignette(VignetteSettings& aSettings)
	{
		myVignetteSettings = aSettings;
		myOverrideVignette = true;
	}

	void PostprocessManager::LerpAndSubmit(Ref<PostProcessComponent> aFirst, Ref<PostProcessComponent> aSecond, float aFactor)
	{
		PostProcessInfo info = {};

		
		auto& firstdata = aFirst->myInfo.Data;
		auto& seconddata = aSecond->myInfo.Data;

		info.Enable = true;
		info.passes = aFirst->myInfo.passes | aSecond->myInfo.passes;

		info.Data.Enables = Utils::Lerp(firstdata.Enables, seconddata.Enables, aFactor);

		info.Data.OutlineColor = Utils::Lerp(firstdata.OutlineColor, seconddata.OutlineColor, aFactor);

		info.Data.FogColor = Utils::Lerp(firstdata.FogColor, seconddata.FogColor, aFactor);
		info.Data.FogDensity = Utils::Lerp(firstdata.FogDensity, seconddata.FogDensity, aFactor);
		info.Data.Fogthreshold = Utils::Lerp(firstdata.Fogthreshold, seconddata.Fogthreshold, aFactor);
		info.Data.FogWaveFrekvency = Utils::Lerp(firstdata.FogWaveFrekvency, seconddata.FogWaveFrekvency, aFactor);
		info.Data.FogWaveHeight = Utils::Lerp(firstdata.FogWaveHeight, seconddata.FogWaveHeight, aFactor);
		info.Data.LogFog = Utils::Lerp(firstdata.LogFog, seconddata.LogFog, aFactor);

		info.Data.Constrast = Utils::Lerp(firstdata.Constrast, seconddata.Constrast, aFactor);
		info.Data.Saturation = Utils::Lerp(firstdata.Saturation, seconddata.Saturation, aFactor);
		info.Data.Gain = Utils::Lerp(firstdata.Gain, seconddata.Gain, aFactor);
		info.Data.Gamma = Utils::Lerp(firstdata.Gamma, seconddata.Gamma, aFactor);
		info.Data.Intensities = Utils::Lerp(firstdata.Intensities, seconddata.Intensities, aFactor);
		info.Data.Pad = Utils::Lerp(firstdata.Pad, seconddata.Pad, aFactor);
		info.Data.Padding = Utils::Lerp(firstdata.Padding, seconddata.Padding, aFactor);
		info.Data.SSAOSettings = Utils::Lerp(firstdata.SSAOSettings, seconddata.SSAOSettings, aFactor);
		info.Data.WindSpeed = Utils::Lerp(firstdata.WindSpeed, seconddata.WindSpeed, aFactor);

		if (myOverrideVignette)
		{
			info.Data.Padding.x = myVignetteSettings.size;
			info.Data.Padding.y = myVignetteSettings.Intensity;
			info.Data.OutlineColor = myVignetteSettings.color;
			myOverrideVignette = false;
		}

		Firefly::Renderer::Submit(info);


		Firefly::VolumetricFogInfo fog{};
		fog.Enable = aFirst->myEnableVolumetricFog || aSecond->myEnableVolumetricFog;

		fog.Color = Utils::Vec4ToVec3(Utils::Lerp(aFirst->myEnvironmentFogColor, aSecond->myEnvironmentFogColor, aFactor));
		fog.Intensity = Utils::Lerp(aFirst->myEnvironmentFogIntensity, aSecond->myEnvironmentFogIntensity, aFactor);

		fog.ColorGodRay = Utils::Vec4ToVec3(Utils::Lerp(aFirst->myGodRaysColor, aSecond->myGodRaysColor, aFactor));
		fog.IntensityGodray = Utils::Lerp(aFirst->myGodRaysIntensity, aSecond->myGodRaysIntensity, aFactor);

		fog.Density = Utils::Lerp(aFirst->myVolumetricFogDensity, aSecond->myVolumetricFogDensity, aFactor);
		fog.Phase = Utils::Lerp(aFirst->myVolumetricFogPhase, aSecond->myVolumetricFogPhase, aFactor);
		fog.DepthPow = Utils::Lerp(aFirst->myVolumetricFogDepthPow, aSecond->myVolumetricFogDepthPow, aFactor);
		fog.resurve = Utils::Lerp(aFirst->myVolumetricFogSkyInterction, aSecond->myVolumetricFogSkyInterction, aFactor);

		Firefly::Renderer::Submit(fog);

		Firefly::BloomInfo bloomInfo{};
		bloomInfo.BloomThreshhold = Utils::Lerp(aFirst->myBloomThreshhold, aSecond->myBloomThreshhold, aFactor);

		Firefly::Renderer::Submit(bloomInfo);

		bool isSecondCmp = Utils::Tlerp(false, true, aFactor);

		CustomPostprocessInfo custom0 = {};
		CustomPostprocessInfo custom1 = {};
		CustomPostprocessInfo custom2 = {};
		CustomPostprocessInfo custom3 = {};

		custom0.Passorder = PassOrder::First;
		custom1.Passorder = PassOrder::Second;
		custom2.Passorder = PassOrder::Third;
		custom3.Passorder = PassOrder::Forth;

		if (isSecondCmp)
		{
			custom0.Material = aSecond->myPostprocessPass0;
			custom1.Material = aSecond->myPostprocessPass1;
			custom2.Material = aSecond->myPostprocessPass2;
			custom3.Material = aSecond->myPostprocessPass3;	
		}
		else
		{
			custom0.Material = aFirst->myPostprocessPass0;
			custom1.Material = aFirst->myPostprocessPass1;
			custom2.Material = aFirst->myPostprocessPass2;
			custom3.Material = aFirst->myPostprocessPass3;
		}


		Renderer::Submit(custom0);
		Renderer::Submit(custom1);
		Renderer::Submit(custom2);
		Renderer::Submit(custom3);
	}

}