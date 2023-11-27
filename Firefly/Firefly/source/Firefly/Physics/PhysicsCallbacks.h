#pragma once
#include "PhysX/PxPhysicsAPI.h"
#include <PhysX/foundation/PxErrors.h>

namespace Firefly
{
	class PhysicsErrorCallback : public physx::PxErrorCallback
	{
	public:
		// Inherited via PxErrorCallback
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
	};

	class PhysicsAllocatorCallback : public physx::PxAllocatorCallback
	{
	public:
		// Inherited via PxAllocatorCallback
		virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override;
		virtual void deallocate(void* ptr) override;
	};

	class PhysicQueryFilterCallback : public physx::PxQueryFilterCallback
	{
	public:



		// Inherited via PxQueryFilterCallback
		virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

	};

	class PhysicsCharacterHitCallback : public physx::PxUserControllerHitReport
	{
	public:
		

		// Inherited via PxUserControllerHitReport
		virtual void onShapeHit(const physx::PxControllerShapeHit& hit) override;

		virtual void onControllerHit(const physx::PxControllersHit& hit) override;

		virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;

	};

	class PhysicsCharacterBehaivorCallback : public physx::PxControllerBehaviorCallback
	{
	public:



		// Inherited via PxControllerBehaviorCallback
		virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor) override;

		virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller) override;

		virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle) override;

	};
	using namespace physx;

	class PhyicsSimulationFilterCallback : public physx::PxSimulationFilterCallback
	{
	public:

		// Inherited via PxSimulationFilterCallback
		virtual PxFilterFlags pairFound(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, const PxActor* a0, const PxShape* s0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, const PxActor* a1, const PxShape* s1, PxPairFlags& pairFlags) override;

		virtual void pairLost(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, bool objectRemoved) override;

		virtual bool statusChange(PxU32& pairID, PxPairFlags& pairFlags, PxFilterFlags& filterFlags) override;

	};

	class PhyicsSimulationCallback : public physx::PxSimulationEventCallback
	{
	public:
		
		virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;


		// Inherited via PxSimulationEventCallback
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;

		void onWake(physx::PxActor** actors, physx::PxU32 count) override;

		void onSleep(physx::PxActor** actors, physx::PxU32 count) override;

		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
		
	};


	inline physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags,
		const void* constantBlock,
		physx::PxU32 constantBlockSize)
	{

		// let triggers through
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		// generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT | physx::PxPairFlag::eSOLVE_CONTACT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eSUPPRESS;

	}
}


