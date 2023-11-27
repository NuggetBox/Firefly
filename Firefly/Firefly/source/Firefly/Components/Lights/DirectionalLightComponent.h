#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
namespace Firefly
{
	class DirectionalLightComponent : public Component
	{
	public:
		DirectionalLightComponent();

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		void SetIntensity(float aIntensity);

		static std::string GetFactoryName() { return "DirectionalLightComponent"; }
		static Ref<Component> Create() { return CreateRef<DirectionalLightComponent>(); }
	private:
		Utils::Vector4f myColor;
		std::vector<std::string> myEnumNames;
		bool myShouldCastShadows = true;
		bool myShouldSoftShadows = true;
		ShadowResolutions myShadowResolutions = ShadowResolutions::res2048;
		float myIntensity;

	};
}