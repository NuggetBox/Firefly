#include "FFpch.h"
#include "PhysicsCallbacks.h"
#include <Firefly/Core/Log/DebugLogger.h>
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include <Firefly/Event/EntityEvents.h>

#include "PhysicsUtils.h"

#include "Firefly/Physics/PhysicsImplementation.h"

#define PHYSICS_LOG 0
#define PHYSICS_DEBUG_LOG 1

#if PHYSICS_LOG
#define PhysLog(...) LOGINFO(__VA_ARGS__)
#define PhysLogWarn(...) LOGWARNING(__VA_ARGS__)
#define PhysLogError(...) LOGERROR(__VA_ARGS__)
#else
#define PhysLog(...)
#define PhysLogWarn(...) 
#define PhysLogError(...)
#endif // PHYSICS_LOG


#if PHYSICS_DEBUG_LOG
#define DebugLog(...) LOGINFO(__VA_ARGS__)
#define DebugLogWarn(...) LOGWARNING(__VA_ARGS__)
#define DebugLogError(...) LOGERROR(__VA_ARGS__)
#else
#define DebugLog(...)
#define DebugLogWarn(...) 
#define DebugLogError(...)
#endif // PHYSICS_LOG


namespace Firefly
{
	void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		switch (code)
		{
		case physx::PxErrorCode::eNO_ERROR:
			DebugLog("PhysX: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eDEBUG_INFO:
			DebugLog("PhysX info: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eDEBUG_WARNING:
			DebugLogWarn("PhysX warning: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eINVALID_PARAMETER:
			DebugLogError("PhysX invalid parameter: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eINVALID_OPERATION:
			DebugLogError("PhysX invalid operation: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eOUT_OF_MEMORY:
			DebugLogError("PhysX out of memory: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eINTERNAL_ERROR:
			DebugLogError("PhysX internal error: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eABORT:
			DebugLogError("PhysX ABORT: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::ePERF_WARNING:
			PhysLogWarn("PhysX perf waring: {0}, in: {1}, at: {2}", message, file, line);
			break;
		case physx::PxErrorCode::eMASK_ALL:
			break;
		default:
			break;
		}
	}

	void* PhysicsAllocatorCallback::allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* allocatedMem = _aligned_malloc(size, 16);
		auto fullAllocSize = size * sizeof(uint8_t);
		PhysLog("PhysX: Allocating: {0} bytes, From file: {1}, On line: {2}", fullAllocSize, filename, line);
		return allocatedMem;
	}
	void PhysicsAllocatorCallback::deallocate(void* ptr)
	{
		PhysLog("PhysX: Destroying memoryblock: {0}", ptr);
		_aligned_free(ptr);
	}

	void PhyicsSimulationCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		using namespace physx;

		int32_t statusOfCollision = 1;
		if (pairs->flags & PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			statusOfCollision = 1;
		}
		if (pairs->flags & PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			statusOfCollision = 2;
		}

		for (physx::PxU32 i = 0; i < nbPairs; ++i)
		{
			const auto entityAID = (size_t)pairHeader.actors[0]->userData;
			const auto entityBID = (size_t)pairHeader.actors[1]->userData;

			// Extract contact points, impulses and internal face indices
			PxVec3 contactPoint, impulse, normal;
			constexpr PxU32 bufferSize = 64;
			PxContactPairPoint contacts[bufferSize];
			PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);

			//Add if we need impulse or more info
			/*for (PxU32 j = 0; j < nbContacts; j++)
			{
				PxVec3 point = contacts[j].position;
				PxVec3 impulse = contacts[j].impulse;
				PxU32 internalFaceIndex0 = contacts[j].internalFaceIndex0;
				PxU32 internalFaceIndex1 = contacts[j].internalFaceIndex1;
			}*/

			if (nbContacts > 0)
			{
				contactPoint = contacts[0].position;
				impulse = contacts[0].impulse;
				normal = contacts[0].normal;
			}

			PhysicsImplementation::AddActorCallback([entityAID, entityBID, statusOfCollision, contactPoint, impulse, normal]()
				{
					auto entityA = GetEntityWithID(entityAID);
					auto entityB = GetEntityWithID(entityBID);
					//if (entityA == nullptr || entityB == nullptr)
					//	return;

					if (statusOfCollision == 1)
					{
						if (entityA.lock() && entityB.lock())
						{
							EntityOnCollisionEnterEvent onCollisionA(entityB, Physics::PhysXToFFVec3(contactPoint), Physics::PhysXToFFVec3(impulse), Physics::PhysXToFFVec3(normal));
							EntityOnCollisionEnterEvent onCollisionB(entityA, Physics::PhysXToFFVec3(contactPoint), Physics::PhysXToFFVec3(impulse), Physics::PhysXToFFVec3(normal));
							entityA.lock()->OnEvent(onCollisionA);
							entityB.lock()->OnEvent(onCollisionB);
						}
					}
					else if (statusOfCollision == 2)
					{
						if (entityA.lock() && entityB.lock())
						{
							EntityOnCollisionExitEvent onCollisionA(entityB);
							EntityOnCollisionExitEvent onCollisionB(entityA);
							entityA.lock()->OnEvent(onCollisionA);
							entityB.lock()->OnEvent(onCollisionB);
						}
					}
				});
		}

	}
	void PhyicsSimulationCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		using namespace physx;
		for (physx::PxU32 i = 0; i < count; ++i)
		{
			if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			{
				continue;
			}

			if (!((pairs[i].otherShape->getSimulationFilterData().word0 & pairs[i].triggerShape->getSimulationFilterData().word1)
				&& (pairs[i].triggerShape->getSimulationFilterData().word0 & pairs[i].otherShape->getSimulationFilterData().word1)))
			{
				continue;
			}

			int32_t statusOfCollision = 0;
			if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				statusOfCollision = 1;
			}
			if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				statusOfCollision = 2;
			}
			const auto entitySolid = (size_t)pairs[i].otherActor->userData;
			const auto entityTrigger = (size_t)pairs[i].triggerActor->userData;

			PhysicsImplementation::AddActorCallback([entitySolid, entityTrigger, statusOfCollision]()
				{
					auto entityS = GetEntityWithID(entitySolid).lock();
					auto entityB = GetEntityWithID(entityTrigger).lock();

					if (statusOfCollision == 1)
					{
						if (entityS && entityB)
						{
							EntityOnTriggerEnterEvent onCollisionB(entityS);
							EntityOnTriggerEnterEvent onCollisionS(entityB);
							entityB->OnEvent(onCollisionB);
							entityS->OnEvent(onCollisionS);
						}
					}
					else if (statusOfCollision == 2)
					{
						if (entityS && entityB)
						{
							EntityOnTriggerExitEvent onCollisionB(entityS);
							EntityOnTriggerExitEvent onCollisionS(entityB);
							entityB->OnEvent(onCollisionB);
							entityS->OnEvent(onCollisionS);
						}
					}
				});
		}
	}
	void PhyicsSimulationCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		LOGINFO("onConstraintBreak");

	}
	void PhyicsSimulationCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		LOGINFO("onWake");

	}
	void PhyicsSimulationCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		LOGINFO("onSleep");
	}
	void PhyicsSimulationCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		LOGINFO("onAdvance");

	}
	void PhysicsCharacterHitCallback::onShapeHit(const physx::PxControllerShapeHit& hit)
	{
		using namespace physx;
		const auto controllerID = (size_t)hit.controller->getUserData();
		const auto hitID = (size_t)hit.actor->userData;
		auto flags = hit.shape->getFlags();

	}
	void PhysicsCharacterHitCallback::onControllerHit(const physx::PxControllersHit& hit)
	{
		const auto controllerID = (size_t)hit.controller->getActor()->userData;
		const auto hitID = (size_t)hit.controller->getActor()->userData;

		PhysicsImplementation::AddActorCallback([controllerID, hitID]()
			{
				auto entityA = GetEntityWithID(controllerID);
				auto entityB = GetEntityWithID(hitID);

				if (!entityA.expired() && !entityB.expired())
				{
					EntityOnCollisionEnterEvent onCollisionA(entityB);
					EntityOnCollisionEnterEvent onCollisionB(entityA);
					entityA.lock()->OnEvent(onCollisionA);
					entityB.lock()->OnEvent(onCollisionB);
				}
			});
	}
	void PhysicsCharacterHitCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit)
	{
		LOGINFO("dlagjklDG");
	}
	physx::PxControllerBehaviorFlags PhysicsCharacterBehaivorCallback::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
	{
		return physx::PxControllerBehaviorFlags();
	}
	physx::PxControllerBehaviorFlags PhysicsCharacterBehaivorCallback::getBehaviorFlags(const physx::PxController& controller)
	{
		return physx::PxControllerBehaviorFlags();
	}
	physx::PxControllerBehaviorFlags PhysicsCharacterBehaivorCallback::getBehaviorFlags(const physx::PxObstacle& obstacle)
	{
		return physx::PxControllerBehaviorFlags();
	}
	physx::PxQueryHitType::Enum PhysicQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
	{
		// check if the two shapes should be considered for collision
		if ((shape->getQueryFilterData().word0 & filterData.word1) && (filterData.word0 & shape->getQueryFilterData().word1))
		{
			if (shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
			{
				return physx::PxQueryHitType::eTOUCH;
			}
			return  physx::PxQueryHitType::eBLOCK;
		}

		return  physx::PxQueryHitType::eNONE;
	}
	PxFilterFlags PhyicsSimulationFilterCallback::pairFound(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, const PxActor* a0, const PxShape* s0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, const PxActor* a1, const PxShape* s1, PxPairFlags& pairFlags)
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
		}

		return physx::PxFilterFlag::eDEFAULT;
	}
	void PhyicsSimulationFilterCallback::pairLost(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, bool objectRemoved)
	{
	}
	bool PhyicsSimulationFilterCallback::statusChange(PxU32& pairID, PxPairFlags& pairFlags, PxFilterFlags& filterFlags)
	{
		return false;
	}
}
