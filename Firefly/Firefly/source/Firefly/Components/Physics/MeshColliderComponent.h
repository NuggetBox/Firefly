#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Firefly/Physics/PhysicsCharacterController.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Physics/PhysicsCallbacks.h"
namespace Firefly
{
	struct BillboardInfo;
	class Texture2D;
	class Mesh;
	class MeshColliderComponent : public Component
	{
		friend class RigidbodyComponent;
	public:
		MeshColliderComponent();
		~MeshColliderComponent();

		void Initialize() override;
		void EarlyInitialize() override;

		void OnEvent(Firefly::Event& aEvent) override;
		//bool OnFixedUpdateEvent(Firefly::AppFixedUpdateEvent& aEvent);

		void SetScale(const Utils::Vec3& aScale);
		
		static std::string GetFactoryName() { return "MeshColliderComponent"; }
		static Ref<Component> Create() { return CreateRef<MeshColliderComponent>(); }

	private:
		void LoadDebugBillboard();
		void DrawDebugBillboard();

		Ref<Texture2D> myDebugBillboard;
		Ref<BillboardInfo> myDebugBillboardInfo;

		std::vector<physx::PxGeometry*> myMeshGeometry;
		std::vector<physx::PxShape*> myShapes;
		physx::PxMaterial* myMaterial = nullptr;
		Ref<Mesh> myMesh;
		std::string myMeshPath;
		bool myIsConvex;

		float myStaticFriction = 0.5f;
		float myDynamicFriction = 0.5f;
		float myBounciness = 0.5f;
		float myWeldTolerance = 0.f;
	};
}
