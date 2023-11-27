#pragma once
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class PointLightComponent;
	class SpotLightComponent;
	class DirectionalLightComponent;
}

class LightIntensityComponent : public Firefly::Component
{
public:
	LightIntensityComponent();
	~LightIntensityComponent() override = default;

	void Initialize() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::string GetFactoryName() { return "LightIntensityComponent"; }
	static Ref<Component> Create() { return CreateRef<LightIntensityComponent>(); }

private:
	
	float myMinValue = 0;
	float myMinValue2 = 0;
	float myMaxValue = 1;
	float myMaxValue2 = 1;
	float myCurrentTime = 0;
	float myMaxTime = 1;
	bool myTimeDirection = false;

	Ptr<Firefly::PointLightComponent> myPointLight;
	Ptr<Firefly::SpotLightComponent> mySpotLight;
	Ptr<Firefly::DirectionalLightComponent> myDirectionalLight;
};