#include "FFpch.h"
#include "EnvironmentLightComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Rendering/Renderer.h"
#include <Firefly/Event/EntityEvents.h>

namespace Firefly
{
	REGISTER_COMPONENT(EnvironmentLightComponent);
	EnvironmentLightComponent::EnvironmentLightComponent() : Component("EnvironmentLightComponent")
	{
		myIntensity = 1.f;
		mySkyColorMod = { 1, 1, 1 };
		mySkyModIntensity = 20.f;
		mySunRadius = 0.4675f;
		myGroundColor = { 0,0,0 };
		myEnvironmentMipMap = 0;
		EditorVariable("Envi Path", ParameterType::File, &myPath, ".dds");

		EditorVariable("skybox", ParameterType::File, &mySkyboxPath, ".dds");

		EditorVariable("Intensity", ParameterType::Float, &myIntensity);

		EditorVariable("Sky Color", Firefly::ParameterType::Color, &mySkyColorMod, false);

		EditorVariable("Sky Color Intensity", ParameterType::Float, &mySkyModIntensity);

		EditorVariable("Sky Ground Color", Firefly::ParameterType::Color, &myGroundColor, false);
		EditorTooltip("this is the color that will bounce up in to the atmosphere from the ground");

		EditorVariable("Sun Radius", ParameterType::Float, &mySunRadius);
		EditorTooltip("Size of the sun, \n NOTE: this should not be larger than 1");

		EditorVariable("Skybox blur strength", ParameterType::Int, &myEnvironmentMipMap);
	}

	void EnvironmentLightComponent::Initialize()
	{
		if (!myPath.empty())
		{
			myEnvironmentMap = ResourceCache::GetAsset<Texture2D>(myPath);
		}
		if (!mySkyboxPath.empty())
		{
			mySkyboxMap = ResourceCache::GetAsset<Texture2D>(mySkyboxPath);
		}
	}

	void EnvironmentLightComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& e)
			{
				if (myEnvironmentMap && myEnvironmentMap->IsLoaded())
				{
					EnvironmentData data;
					data.EnvironmentMap = myEnvironmentMap;
					data.SkyBoxMap = mySkyboxMap;
					data.Intensity = myIntensity;
					data.SkyLightIntensity = mySkyColorMod * mySkyModIntensity;
					data.SunRadius = mySunRadius / 100.f;
					data.GroundColor = myGroundColor;
					data.EnvironmentMipMap = myEnvironmentMipMap;
					Renderer::Submit(data);
				}
				return false;
			});
		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				if (!myPath.empty())
				{
					myEnvironmentMap = ResourceCache::GetAsset<Texture2D>(myPath);
				}
				if (!mySkyboxPath.empty())
				{
					mySkyboxMap = ResourceCache::GetAsset<Texture2D>(mySkyboxPath);
				}
				return false;
			});
	}

	void EnvironmentLightComponent::SetCubeMap(std::string aPath)
	{
		myPath = aPath;
		if (!myPath.empty())
		{
			myEnvironmentMap = ResourceCache::GetAsset<Texture2D>(myPath);
		}
	}

	void EnvironmentLightComponent::SetSkyBox(std::string_view aPath)
	{
		mySkyboxPath = aPath;
		if (!mySkyboxPath.empty())
		{
			mySkyboxMap = ResourceCache::GetAsset<Texture2D>(mySkyboxPath);
		}
	}
}