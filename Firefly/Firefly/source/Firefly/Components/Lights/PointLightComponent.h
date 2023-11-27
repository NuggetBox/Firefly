#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Asset/Texture/Texture2D.h"
namespace Firefly
{
	class PointLightComponent : public Component
	{
	public:
		PointLightComponent();

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;
		void SetIntensity(float aIntensity);
		void SetRadius(float aRadius);

		static std::string GetFactoryName() { return "PointLightComponent"; }
		static Ref<Component> Create() { return CreateRef<PointLightComponent>(); }
	private:
		Utils::Vector4f myColor = { 0,0,0,0 };
		float myIntensity  = 0; 
		float myRadius = 0;
		Ref<Texture2D> myIcon;
		bool myCastShadows = false;
	};
}