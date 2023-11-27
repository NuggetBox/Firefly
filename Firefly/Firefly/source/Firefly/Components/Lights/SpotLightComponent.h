#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Asset/Texture/Texture2D.h"
namespace Firefly
{

	class SpotLightComponent : public Component
	{
	public:
		SpotLightComponent();

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		void SetIntensity(float aIntensity);

		static std::string GetFactoryName() { return "SpotLightComponent"; }
		static Ref<Component> Create() { return CreateRef<SpotLightComponent>(); }
	private:
		Utils::Vector4f myColor = { 1,1,1,1 };
		float myIntensity = 1000;
		float myRange = 500;
		float myNearRadius = 10.f;
		float myFarRadius = 30.f;

		bool myShouldCastShadows = false;
	};
}