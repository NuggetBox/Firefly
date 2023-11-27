#pragma once
#include "Firefly/ComponentSystem/Component.h"

#include "Firefly/Physics/PhysicsActor.h"

namespace Firefly
{
	class RigidbodyComponent;
	class AppUpdateEvent;
	class AppFixedUpdateEvent;
	class EntityPropertyUpdatedEvent;

	class FixedJointComponent : public Component
	{
	public:
		FixedJointComponent();
		~FixedJointComponent();

		void Initialize() override;

		void OnEvent(Event& aEvent) override;
		bool OnUpdateEvent(AppUpdateEvent& aEvent);
		bool OnFixedUpdateEvent(AppFixedUpdateEvent& aEvent);
		bool PropertyUpdatedEvent(EntityPropertyUpdatedEvent& aEvent);

		static std::string GetFactoryName() { return "FixedJointComponent"; }
		static Ref<Component> Create() { return CreateRef<FixedJointComponent>(); }

		void SetHitPoint(const Utils::Vec3& aHitPoint);

		void SetConnectedEntity(Ptr<Entity> aEntity);
		Ptr<Entity> GetConnectedEntity() { return myConnectedEntity; }

		physx::PxFixedJoint* GetJoint() { return myJoint; }

		float myBreakForce = FLT_MAX;
		float myBreakTorque = FLT_MAX;

		bool myShouldCollide = false;

	private:
		Utils::Vec3 myHitPoint;

		physx::PxTransform myConnectedTransform;
		physx::PxTransform myTransform;
		Ptr<Entity> myConnectedEntity;
		Ptr<RigidbodyComponent> myConnectedRigidBody;
		physx::PxFixedJoint* myJoint = nullptr;

		void SetVariables();
		physx::PxRigidActor* GenerateActor();
		void CreateJoint();
	};
}