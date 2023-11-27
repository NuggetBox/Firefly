#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "PhysX/PxPhysicsAPI.h"

namespace Firefly
{
	struct BillboardInfo;
	class Texture2D;

	class BoxColliderComponent : public Component
	{
		friend class RigidbodyComponent;
	public:
		BoxColliderComponent();
		~BoxColliderComponent() override;

		void EarlyInitialize() override;
		void Initialize() override;

		void OnEvent(Firefly::Event& aEvent) override;
		bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);
		bool OnPlayEvent(EditorPlayEvent& aEvent);
		void SetScale(const Utils::Vec3& aScale);
		void SetOffset(const Utils::Vector3f& aOffset);
		void SetWithMinMax(const Utils::Vector3f& aMin, const Utils::Vector3f& aMax);

		bool IsTrigger() const { return myIsTigger; }
		static std::string GetFactoryName() { return "BoxColliderComponent"; }
		static Ref<Component> Create() { return CreateRef<BoxColliderComponent>(); }

	private:
		void CleanUp();
		void LoadDebugBillboard();
		void DrawDebugBillboard();

		Ref<Texture2D> myDebugBillboard;
		Ref<BillboardInfo> myDebugBillboardInfo;

		physx::PxShape* myShape = nullptr;
		physx::PxBoxGeometry myGeometry;
		physx::PxMaterial* myMaterial;
		bool myIsTigger = false;
		Utils::Vec3 myHalfSize;
		Utils::Vec3 myOffsetSize;
		Utils::Vec3 myOffset;

		float myStaticFriction = 0.5f;
		float myDynamicFriction = 0.5f;
		float myBounciness = 0.5f;
	};
}
