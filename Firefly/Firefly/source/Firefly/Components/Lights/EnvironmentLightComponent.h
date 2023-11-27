#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Asset/Texture/Texture2D.h"
namespace Firefly
{
	class EnvironmentLightComponent : public Component
	{
	public:
		EnvironmentLightComponent();

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;
		void SetIntensity(float aIntensity) { myIntensity = aIntensity; };
		float GetIntensity() { return myIntensity; }
		void SetCubeMap(std::string aPath);
		void SetSkyBox(std::string_view aPath);

		static std::string GetFactoryName() { return "EnvironmentLightComponent"; }
		static Ref<Component> Create() { return CreateRef<EnvironmentLightComponent>(); }
	private:
		std::string myPath;
		std::string mySkyboxPath;
		Ref<Texture2D> myEnvironmentMap;
		Ref<Texture2D> mySkyboxMap;
		Utils::Vec3 mySkyColorMod;
		float mySkyModIntensity;
		Utils::Vec3 myGroundColor;
		float mySunRadius;
		float myIntensity;
		int32_t myEnvironmentMipMap = 0;
	};
}