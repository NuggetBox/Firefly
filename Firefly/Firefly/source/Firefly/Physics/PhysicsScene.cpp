#include "FFpch.h"
#include "PhysicsScene.h"
#include "Firefly/Physics/PhysicsImplementation.h"

#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/ComponentSystem/Entity.h"
#include <Firefly/ComponentSystem/ComponentSystemUtils.h>
#include "Utils/Math/Transform.h"

#include "PhysicsLayerHandler.h"
#include <Utils/Timer.h>

namespace Firefly
{
#define FF_LOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockWrite();
#define FF_UNLOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockWrite();

#define FF_LOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockRead();
#define FF_UNLOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockRead();

	PhysicsScene::PhysicsScene(const PhysicsSceneInfo& aInfo)
	{
		CreateScene(aInfo);

		myPhysXScene->lockWrite();
		myPhysXScene->setSimulationEventCallback(mySimulationCallback);
		myPhysXScene->unlockWrite();

		CreateControllerManager();

		ConnectDebugger();
	}
	void PhysicsScene::CreateScene(const Firefly::PhysicsSceneInfo& aInfo)
	{
		auto physics = PhysicsImplementation::myPhysics;
		auto cpuDispatcher = PhysicsImplementation::myCpuDispatcher;
		physx::PxSceneDesc sceneDesc = { physics->getTolerancesScale() };
		sceneDesc.cpuDispatcher = cpuDispatcher;
		sceneDesc.gravity = Physics::FFToPhysXVec3(aInfo.Gravity); //default gravity
		mySimulationCallback = new PhyicsSimulationCallback();
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS | physx::PxSceneFlag::eDISABLE_CONTACT_CACHE | physx::PxSceneFlag::eREQUIRE_RW_LOCK;
		sceneDesc.filterShader = contactReportFilterShader;
		sceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
		sceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
		sceneDesc.dynamicBVHBuildStrategy = physx::PxBVHBuildStrategy::Enum::eFAST;
		sceneDesc.dynamicTreeSecondaryPruner = physx::PxDynamicTreeSecondaryPruner::eBVH;

		sceneDesc.staticStructure = physx::PxPruningStructureType::Enum::eDYNAMIC_AABB_TREE;
		sceneDesc.limits.setToDefault();

		sceneDesc.sceneQueryUpdateMode = physx::PxSceneQueryUpdateMode::eBUILD_DISABLED_COMMIT_DISABLED;


		myPhysXScene = physics->createScene(sceneDesc);
	}
	void PhysicsScene::CreateControllerManager()
	{
		myPhysXControllerManager = PxCreateControllerManager(*myPhysXScene);
		myPhysXControllerManager->setOverlapRecoveryModule(true);
		myPhysXControllerManager->setTessellation(true, 50.f);
	}
	void PhysicsScene::ConnectDebugger()
	{
		PhysicsImplementation::ConnectToDebugger();
		myCilent = myPhysXScene->getScenePvdClient();
		if (myCilent)
		{
			myCilent->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			myCilent->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			myCilent->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}
	void PhysicsScene::SetGravity(const Utils::Vector3f& aGravity)
	{
		FF_LOCK_WRITE;
		myPhysXScene->setGravity(Physics::FFToPhysXVec3(aGravity));
		FF_UNLOCK_WRITE;
	}
	Utils::Vector3f PhysicsScene::GetGravity()
	{
		FF_LOCK_READ;
		const auto gravity = Physics::PhysXToFFVec3(myPhysXScene->getGravity());
		FF_UNLOCK_READ;
		return gravity;
	}

	std::vector<Ptr<Entity>> PhysicsScene::BoxOverlap(const Utils::Transform& aTransform, const Utils::Vec3& aSize, const int aLayer)
	{
		std::vector<Ptr<Entity>> hitEntities;
		if (!myPhysXScene)
		{
			return hitEntities;
		}
		physx::PxOverlapBufferN<20> callback;

		PxQueryFilterData filter{};
		filter.data.word0 = aLayer;
		filter.data.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aLayer);
		filter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eNO_BLOCK;

		physx::PxTransform trans(Physics::FFToPhysXVec3(aTransform.GetPosition()),
			Physics::FFToPhysXQuat(aTransform.GetQuaternion()));

		physx::PxBoxGeometry box;
		box.halfExtents = { aSize.x, aSize.y, aSize.z };
		if (!myPhysXScene->overlap(box, trans, callback, filter, &myQueryFilterCallback))
		{
			return hitEntities;
		}
		hitEntities.reserve(callback.nbTouches);
		for (size_t i = 0; i < callback.nbTouches; i++)
		{
			auto hitEntity = (size_t)callback.getTouch(i).actor->userData;
			hitEntities.push_back(GetEntityWithID(hitEntity));
		}
		return hitEntities;
	}

	std::vector<Ptr<Entity>> PhysicsScene::SphereOverlap(const Utils::Transform& aTransform, const float aRadius, const int aLayer)
	{
		std::vector<Ptr<Entity>> hitEntities;
		if (!myPhysXScene)
		{
			return hitEntities;
		}
		myPhysXScene->lockWrite();
		physx::PxOverlapBufferN<20> callback;

		PxQueryFilterData filter{};
		filter.data.word0 = aLayer;
		filter.data.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aLayer);
		filter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eNO_BLOCK;

		physx::PxTransform trans(Physics::FFToPhysXVec3(aTransform.GetPosition()),
			Physics::FFToPhysXQuat(aTransform.GetQuaternion()));

		physx::PxSphereGeometry sphere;
		sphere.radius = aRadius;
		if (!myPhysXScene->overlap(sphere, trans, callback, filter, &myQueryFilterCallback))
		{
			myPhysXScene->unlockWrite();
			return hitEntities;
		}

		hitEntities.reserve(callback.nbTouches);
		for (size_t i = 0; i < callback.nbTouches; i++)
		{
			auto hitEntity = (size_t)callback.getTouch(i).actor->userData;
			hitEntities.push_back(GetEntityWithID(hitEntity));
		}
		myPhysXScene->unlockWrite();
		return hitEntities;
	}

	RayResult PhysicsScene::Raycast(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength, const int aLayer)
	{
		if (Utils::IsAlmostEqual(aDirection.LengthSqr(), 0.0f, 0.0001f))
		{
			return {};
		}

		RayResult result;
		if (!myPhysXScene)
		{
			return result;
		}

		physx::PxRaycastBufferN<1> callback;
		float length = aLength;
		if (aLength < 0.f)
		{
			length = std::numeric_limits<float>().max();
		}
		PxQueryFilterData filter{};
		filter.data.word0 = aLayer;
		filter.data.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aLayer);
		filter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;
		myPhysXScene->lockWrite();
		bool hit = myPhysXScene->raycast(Physics::FFToPhysXVec3(aOrigin),
			Physics::FFToPhysXVec3(aDirection), length, callback, PxHitFlag::eDEFAULT, filter, &myQueryFilterCallback);
		if (!hit)
		{
			myPhysXScene->unlockWrite();
			return result;
		}
		result.Position = Physics::PhysXToFFVec3(callback.hits[0].position);
		result.Normal = Physics::PhysXToFFVec3(callback.hits[0].normal);
		result.Distance = callback.hits[0].distance;
		result.HitEntity = GetEntityWithID((size_t)callback.getTouch(0).actor->userData);
		myPhysXScene->unlockWrite();
		return result;
	}

	std::vector<RayResult> PhysicsScene::RaycastAll(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength, const int aLayer)
	{
		if (Utils::IsAlmostEqual(aDirection.LengthSqr(), 0.0f, 0.0001f))
		{
			return {};
		}

		std::vector<RayResult> results;
		if (!myPhysXScene)
		{
			return results;
		}

		const physx::PxU32 bufferSize = 32;
		physx::PxRaycastHit hitBuffer[bufferSize];
		physx::PxRaycastBuffer callback(hitBuffer, bufferSize);
		float length = aLength;
		if (aLength < 0.f)
		{
			length = std::numeric_limits<float>().max();
		}
		PxQueryFilterData filter{};
		filter.data.word0 = aLayer;
		filter.data.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aLayer);
		filter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eNO_BLOCK;

		myPhysXScene->lockWrite();
		bool hit = myPhysXScene->raycast(Physics::FFToPhysXVec3(aOrigin),
			Physics::FFToPhysXVec3(aDirection), length, callback, PxHitFlag::eDEFAULT, filter, &myQueryFilterCallback);
		if (!hit)
		{
			myPhysXScene->unlockWrite();
			return results;
		}
		std::vector<Ptr<Entity>> entities;
		results.resize(callback.nbTouches);
		for (int i = 0; i < results.size(); i++)
		{
			auto& hit = callback.touches[i];
			results[i].HitEntity = GetEntityWithID((uint64_t)hit.actor->userData);
			results[i].Position = Physics::PhysXToFFVec3(hit.position);
			results[i].Normal = Physics::PhysXToFFVec3(hit.normal);
			results[i].Distance = hit.distance;
		}
		myPhysXScene->unlockWrite();
		std::sort(results.begin(), results.end());
		return results;
	}

	std::vector<RayResult> PhysicsScene::RaycastNoTrigger(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength, const int aLayer)
	{
		if (Utils::IsAlmostEqual(aDirection.LengthSqr(), 0.0f, 0.0001f))
		{
			return {};
		}

		std::vector<RayResult> results;
		if (!myPhysXScene)
		{
			return results;
		}
		physx::PxRaycastBuffer callback;
		float length = aLength;
		if (aLength < 0.f)
		{
			length = std::numeric_limits<float>().max();
		}
		PxQueryFilterData filter{};
		filter.data.word0 = aLayer;
		filter.data.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aLayer);
		filter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;
		myPhysXScene->lockWrite();
		bool hit = myPhysXScene->raycast(Physics::FFToPhysXVec3(aOrigin),
			Physics::FFToPhysXVec3(aDirection), length, callback, PxHitFlag::eDEFAULT, filter, &myQueryFilterCallback);
		if (!hit)
		{
			myPhysXScene->unlockWrite();
			return results;
		}
		results.resize(callback.getNbAnyHits());
		for (int i = 0; i < results.size(); i++)
		{
			auto& hit = callback.getAnyHit(i);
			results[i].HitEntity = GetEntityWithID((uint64_t)hit.actor->userData);
			results[i].Position = Physics::PhysXToFFVec3(hit.position);
			results[i].Normal = Physics::PhysXToFFVec3(hit.normal);
			results[i].Distance = hit.distance;
		}
		myPhysXScene->unlockWrite();
		return results;

	}

	void PhysicsScene::Simulate(const float aFixedDeltaTime)
	{
		myPhysXScene->lockWrite();
		const bool simulateResult = myPhysXScene->simulate(aFixedDeltaTime);

		if (simulateResult)
		{

			if (!myPhysXScene->fetchResults(true))
			{
				LOGERROR("PhysX FETCH RESULTS FAILED!!!");
			}

		}
		else
		{
			LOGERROR("PhysX SIMULATE FAILED!!!");
		}

		myPhysXScene->unlockWrite();
	}

	void PhysicsScene::UpdateQuery()
	{
		myPhysXScene->lockWrite();
		myPhysXScene->sceneQueriesUpdate();

		if (!myPhysXScene->fetchQueries(true))
		{
			LOGERROR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
		}

		myPhysXScene->unlockWrite();
	}

	void PhysicsScene::UpdateControllerManager()
	{
		myPhysXControllerManager->computeInteractions(Utils::Timer::GetDeltaTime());
	}

	Ref<PhysicsScene> PhysicsScene::Create(const PhysicsSceneInfo& aInfo)
	{
		return CreateRef<PhysicsScene>(aInfo);
	}

	PhysicsScene::~PhysicsScene()
	{
		PHYSX_SAFE_DESTROY(myPhysXScene);
		FF_SAFE_DESTROY(mySimulationCallback);
		PhysicsImplementation::DisconnectDebugger();
	}
}
