#pragma once
#include "Firefly/ComponentSystem/Component.h"

#include "Firefly/Physics/PhysicsActor.h"

namespace Firefly
{
	class RigidbodyComponent;
	class AppUpdateEvent;
	class AppFixedUpdateEvent;
	class EntityPropertyUpdatedEvent;

	class DistanceJointComponent : public Component
	{
	public:
		DistanceJointComponent();
		~DistanceJointComponent();

		void Initialize() override;

		void OnEvent(Event& aEvent) override;
		bool OnUpdateEvent(AppUpdateEvent& aEvent);
		bool OnFixedUpdateEvent(AppFixedUpdateEvent& aEvent);
		bool PropertyUpdatedEvent(EntityPropertyUpdatedEvent& aEvent);

		static std::string GetFactoryName() { return "DistanceJointComponent"; }
		static Ref<Component> Create() { return CreateRef<DistanceJointComponent>(); }
		
		void SetHitPoint(const Utils::Vec3& aHitPoint);

		void SetConnectedEntity(Ptr<Entity> aEntity);
		Ptr<Entity> GetConnectedEntity() { return myConnectedEntity; }

		physx::PxDistanceJoint* GetJoint() { return myJoint; }
		
		float mySpring = 10.f;
		float myDamper = 0.2f;
		float myTolerance = 0.025f;
		float myMassScale = 1.f;

		float myMinDistance = 0;
		float myMaxDistance = 500;

		float myBreakForce = FLT_MAX;
		float myBreakTorque = FLT_MAX;

		bool myShouldCollide = false;

	private:
		Utils::Vec3 myHitPoint;

		physx::PxTransform myConnectedTransform;
		physx::PxTransform myTransform;
		Ptr<Entity> myConnectedEntity;
		Ptr<RigidbodyComponent> myConnectedRigidBody;
		physx::PxDistanceJoint* myJoint = nullptr;

		void SetVariables();
		physx::PxRigidActor* GenerateActor();
		void CreateJoint();
	};
}