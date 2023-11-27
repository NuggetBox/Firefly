#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "PhysX/PxPhysicsAPI.h"


namespace Firefly
{
	class Texture2D;
	struct BillboardInfo;

	class SphereColliderComponent : public Component
	{
		friend class RigidbodyComponent;
	public:
		SphereColliderComponent();
		~SphereColliderComponent() override;

		void Initialize() override;
		void EarlyInitialize() override;
		
		void OnEvent(Firefly::Event& aEvent) override;
		bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);
		bool OnPlayEvent(EditorPlayEvent& aEvent);

		void SetRadius(const float& aRadius);
		float GetRadius() { return myGeometry.radius; }

		bool IsTrigger() const { return myTrigger; }

		static std::string GetFactoryName() { return "SphereColliderComponent"; }
		static Ref<Component> Create() { return CreateRef<SphereColliderComponent>(); }

	private:
		void CleanUp();
		void LoadDebugBillboard();
		void DrawDebugBillboard();
		
	private:
		Ref<Texture2D> myDebugBillboard;
		Ref<BillboardInfo> myDebugBillboardInfo;

		physx::PxShape* myShape = nullptr;
		physx::PxSphereGeometry myGeometry;
		physx::PxMaterial* myMaterial;
		float myRadius =100.f;
		bool myTrigger = false;
		Utils::Vec3 myOffset;

		float myStaticFriction = 0.5f;
		float myDynamicFriction = 0.5f;
		float myBounciness = 0.5f;
	};
}