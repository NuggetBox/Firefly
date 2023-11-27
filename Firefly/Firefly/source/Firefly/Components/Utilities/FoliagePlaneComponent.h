#pragma once
#include "Firefly/ComponentSystem/Component.h"

#include "Utils/Math/BoundingPlane.hpp"

namespace Firefly
{
	class MeshComponent;
	class Mesh;

	class FoliagePlaneComponent : public Component
	{
	public:
		FoliagePlaneComponent();

		void Initialize() override;
		void OnEvent(Event& aEvent) override;

		static std::string GetFactoryName() { return "FoliagePlaneComponent"; }
		static Ref<Component> Create() { return CreateRef<FoliagePlaneComponent>(); }

		const Utils::BoundingPlane& GetBoundingPlane() const { return myBoundingPlane; }

	private:
		Utils::BoundingPlane myBoundingPlane;

		Ptr<MeshComponent> myMeshComp;
		bool myCompAdded = false;
	};
}