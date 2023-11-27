#pragma once
#include "Firefly/ComponentSystem/Component.h"

namespace Firefly
{
	class MaterialAsset;

	class DecalComponent : public Component
	{
	public:
		DecalComponent();
		~DecalComponent() = default;

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		static std::string GetFactoryName() { return "DecalComponent"; }
		static Ref<Component> Create() { return CreateRef<DecalComponent>(); }
	private:
		void LoadMaterial();
		std::string myMaterialPath;
		Ref<MaterialAsset> myMaterial;
	};
}
