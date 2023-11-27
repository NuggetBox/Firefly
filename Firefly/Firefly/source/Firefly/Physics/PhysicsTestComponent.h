#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "PhysX/PxPhysicsAPI.h"

namespace Firefly
{
	class PhysicsTestComponent : public Component
	{
	public:
		PhysicsTestComponent();
		~PhysicsTestComponent() = default;

		void Initialize() override;


		void OnEvent(Firefly::Event& aEvent) override;
		bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);
		bool OnFixedUpdateEvent(Firefly::AppFixedUpdateEvent& aEvent);
		bool OnPlayEvent(EditorPlayEvent& aEvent);

		static std::string GetFactoryName() { return "PhysicsTestComponent"; }
		static Ref<Component> Create() { return CreateRef<PhysicsTestComponent>(); }
	private:
		Utils::Vec3 myPos;
	};
}
