#pragma once
#include "Firefly/Core/Core.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Utils/Math/Transform.h"
#include "Firefly/Physics/PhysicsUtils.h"

namespace Firefly
{
	enum class ActorType
	{
		Dynamic = (1 << 0),
		Static = (1 << 1),
		Kinematic = (1 << 2),
	};

	struct PhysicsActorInfo
	{
		ActorType Type;
		uint32_t Filter;
		Utils::Transform Transform;
		std::vector<physx::PxShape*> PhysXShapes;
		bool UseGravity = true;
		bool IsTrigger = false;
		size_t EntityID = 0;
		std::string name;
		Utils::Vector3<bool> LockPosition;
		Utils::Vector3<bool> LockRotation;
	};

	class PhysicsActor
	{
	public:
		PhysicsActor(const PhysicsActorInfo& aInfo);

		// force functions
		void AddForce(const Utils::Vec3& aForce, Physics::ForceType aForceType = Physics::ForceType::Force);
		void ResetForce(Physics::ForceType aForceType = Physics::ForceType::Force);

		// Torque functions
		void AddTorque(const Utils::Vec3& aTorque, Physics::ForceType aForceType = Physics::ForceType::Force);
		void ResetTorque(Physics::ForceType aForceType = Physics::ForceType::Force);

		void SetForceAndTorque(const Utils::Vec3& aForce, const Utils::Vec3& aTorque, Physics::ForceType aMode = Physics::ForceType::Force);

		void AddVelocity(const Utils::Vec3& aVelocity);
		Utils::Vec3 GetLinearVelocity();
		Utils::Vec3 GetAngularVelocity();

		void SetMass(const float& aMass);
		float GetMass();

		void AddToScene();

		void SetPosition(const Utils::Vec3& aPos);
		void SetRotation(const Utils::Quaternion& aQuat);

		void SetAngularDamping(const float& aAngularDamping);
		float GetAngularDamping();

		void SetDamping(const float& aDamping);
		float GetDamping();

		void SetRigidFlags(physx::PxRigidDynamicLockFlag::Enum aFlag, bool aValue);

		physx::PxRigidActor* GetActor() { return myActor; }

		void SyncTransform(Utils::Transform& aTranform);
		static Ref<PhysicsActor> Create(const PhysicsActorInfo& aInfo);
		~PhysicsActor();

	private:
		physx::PxRigidActor* myActor;
		physx::PxTransform myTransform;
		PhysicsActorInfo myInfo;
		inline static uint32_t ourCurrentID = 0;
		uint32_t myCurrentID = 0;
	};
}
