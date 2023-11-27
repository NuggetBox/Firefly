#include "FFpch.h"
#include "PhysicsActor.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/Physics/PhysicsScene.h"
#include "Firefly/Physics/PhysicsLayerHandler.h"
namespace Firefly
{
#define FF_LOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockWrite();
#define FF_UNLOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockWrite();

#define FF_LOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockRead();
#define FF_UNLOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockRead();

	PhysicsActor::PhysicsActor(const PhysicsActorInfo& aInfo)
	{
		auto physicsScene = SceneManager::Get().GetPhysicsScene();
		if (!physicsScene)
		{
			LOGERROR("Physics scene was null!");
			return;
		}

		Application::Get().LockPhysXSimulationMutex();
		physicsScene->GetScenePtr()->lockWrite();

		auto physics = PhysicsImplementation::GetPhysics();

		myTransform.p = Physics::FFToPhysXVec3(aInfo.Transform.GetPosition());
		myTransform.q = Physics::FFToPhysXQuat(aInfo.Transform.GetQuaternion());

		switch (aInfo.Type)
		{
		case ActorType::Dynamic:
		{
			myActor = physics->createRigidDynamic(myTransform);
			myActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !aInfo.UseGravity);
			myActor->userData = (void*)aInfo.EntityID;
		}
			break;
		case ActorType::Static:
		{
			myActor = physics->createRigidStatic(myTransform);
			myActor->userData = (void*)aInfo.EntityID;
		}
			break;
		default:
			LOGERROR("PhysicsActor: Type does not exist!");
			break;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		rigidBody->setMassSpaceInertiaTensor({ 100.f,100.f,100.f });
		physx::PxFilterData filterData;
		filterData.word0 = aInfo.Filter; // word0 = own ID
		filterData.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aInfo.Filter);
		

		physx::PxVec3 averageCenter =  physx::PxVec3(0,0,0);

		for (auto& shape : aInfo.PhysXShapes)
		{
			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
			myActor->attachShape(*shape);
			averageCenter += shape->getLocalPose().p;
			
		}
		averageCenter = averageCenter / static_cast<float>(aInfo.PhysXShapes.size());
		
		if (aInfo.Type == ActorType::Dynamic)
		{
			physx::PxTransform pxTrans{};
			pxTrans.p = averageCenter;
			pxTrans.q = physx::PxQuat(0, 0, 0, 1);
			rigidBody->setCMassLocalPose(pxTrans);
		}
		myInfo = aInfo;

		myActor->setName(myInfo.name.c_str());

		physicsScene->GetScenePtr()->unlockWrite();
		Application::Get().UnlockPhysXSimulationMutex();

		myCurrentID = ourCurrentID++;
		//LOGWARNING("Creating PhysicsActor: {}", myCurrentID);

	}
	void PhysicsActor::AddForce(const Utils::Vec3& aForce, Physics::ForceType aForceType)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->addForce(*(physx::PxVec3*)(&aForce), *(physx::PxForceMode::Enum*)(&aForceType));

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}

	void PhysicsActor::ResetForce(Physics::ForceType aForceType)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->clearForce(*(physx::PxForceMode::Enum*)(&aForceType));
		rigidBody->addForce(rigidBody->getLinearVelocity() * -1, *(physx::PxForceMode::Enum*)(&aForceType));

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}
	void PhysicsActor::AddTorque(const Utils::Vec3& aTorque, Physics::ForceType aForceType)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->addTorque(*(physx::PxVec3*)(&aTorque), *(physx::PxForceMode::Enum*)(&aForceType));

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}
	void PhysicsActor::ResetTorque(Physics::ForceType aForceType)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->clearTorque(*(physx::PxForceMode::Enum*)(&aForceType));

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}

	void PhysicsActor::SetForceAndTorque(const Utils::Vec3& aForce, const Utils::Vec3& aTorque, Physics::ForceType aMode)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->setForceAndTorque(*(physx::PxVec3*)(&aForce), *(physx::PxVec3*)(&aTorque), *(physx::PxForceMode::Enum*)(&aMode));

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}

	void PhysicsActor::AddVelocity(const Utils::Vec3& aVelocity)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
	}

	Utils::Vec3 PhysicsActor::GetLinearVelocity()
	{
		if (myInfo.Type == ActorType::Static)
		{
			return {};
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		FF_LOCK_READ;
		const auto linear = Physics::PhysXToFFVec3(rigidBody->getLinearVelocity());
		FF_UNLOCK_READ;

		return linear;
	}

	Utils::Vec3 PhysicsActor::GetAngularVelocity()
	{
		if (myInfo.Type == ActorType::Static)
		{
			return {};
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		return Physics::PhysXToFFVec3(rigidBody->getAngularVelocity());
	}

	void PhysicsActor::SetMass(const float& aMass)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		if (myInfo.Type == ActorType::Dynamic)
		{
			auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

			const auto scene = SceneManager::Get().GetPhysicsScene();
			if (scene)
			{
				const auto ptr = scene->GetScenePtr();
				ptr->lockWrite();
			}

			rigidBody->setMassSpaceInertiaTensor({ aMass,aMass,aMass });

			if (scene)
			{
				const auto ptr = scene->GetScenePtr();
				ptr->unlockWrite();
			}
		}
	}

	float PhysicsActor::GetMass()
	{
		if (myInfo.Type == ActorType::Static)
		{
			return 0.f;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		return rigidBody->getMass();
	}

	void PhysicsActor::AddToScene()
	{
		auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			
			auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
			ptr->addActor(*myActor);
			ptr->unlockWrite();
		}
	}

	void PhysicsActor::SetPosition(const Utils::Vec3& aPos)
	{
		const auto scene = SceneManager::Get().GetPhysicsScene();
		myTransform.p = Physics::FFToPhysXVec3(aPos);

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		myActor->setGlobalPose(myTransform);

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}

	void PhysicsActor::SetRotation(const Utils::Quaternion& aQuat)
	{
		const auto scene = SceneManager::Get().GetPhysicsScene();
		myTransform.q = *(physx::PxQuat*)&aQuat;

		if (scene)
		{
			scene->GetScenePtr()->lockWrite();
		}

		myActor->setGlobalPose(myTransform);

		if (scene)
		{
			scene->GetScenePtr()->unlockWrite();
		}
	}

	void PhysicsActor::SetAngularDamping(const float& aAngularDamping)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);

		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockWrite();
		}

		rigidBody->setAngularDamping(aAngularDamping);

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockWrite();
		}
	}

	float PhysicsActor::GetAngularDamping()
	{
		if (myInfo.Type == ActorType::Static)
		{
			return 0.f;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		return rigidBody->getAngularDamping();
	}

	void PhysicsActor::SetDamping(const float& aDamping)
	{
		if (myInfo.Type == ActorType::Static)
		{
			return;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		rigidBody->setLinearDamping(aDamping);
	}

	float PhysicsActor::GetDamping()
	{
		if (myInfo.Type == ActorType::Static)
		{
			return 0.f;
		}
		auto rigidBody = static_cast<physx::PxRigidBody*>(myActor);
		return rigidBody->getLinearDamping();
	}

	void PhysicsActor::SetRigidFlags(physx::PxRigidDynamicLockFlag::Enum aFlag, bool aValue)
	{
		if (myInfo.Type == ActorType::Dynamic)
		{
			auto rigidBody = static_cast<physx::PxRigidDynamic*>(myActor);

			FF_LOCK_WRITE;
			rigidBody->setRigidDynamicLockFlag(aFlag, aValue);
			FF_UNLOCK_WRITE;
		}
	}

	void PhysicsActor::SyncTransform(Utils::Transform& aTranform)
	{
		if (myInfo.Type == ActorType::Static)
		{
			SetPosition(aTranform.GetPosition());
			SetRotation(aTranform.GetQuaternion());
		}
		else
		{
			const auto scene = SceneManager::Get().GetPhysicsScene();
			const auto ptr = scene->GetScenePtr();

			if (scene)
			{
				ptr->lockRead();
			}

			aTranform.SetPosition(Physics::PhysXToFFVec3(myActor->getGlobalPose().p));
			aTranform.SetRotation(Physics::PhysXToFFQuat(myActor->getGlobalPose().q));

			if (scene)
			{
				ptr->unlockRead();
			}
		}

		const auto scene = SceneManager::Get().GetPhysicsScene();
		const auto ptr = scene->GetScenePtr();

		if (scene)
		{
			ptr->lockRead();
		}

		myTransform = myActor->getGlobalPose();

		if (scene)
		{
			ptr->unlockRead();
		}
	}

	Ref<PhysicsActor> PhysicsActor::Create(const PhysicsActorInfo& aInfo)
	{
		return CreateRef<PhysicsActor>(aInfo);
	}

	PhysicsActor::~PhysicsActor()
	{
		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			Application::Get().LockPhysXSimulationMutex();
			ptr->lockWrite();
			ptr->removeActor(*myActor);
			PHYSX_SAFE_DESTROY(myActor);
			ptr->unlockWrite();
			Application::Get().UnlockPhysXSimulationMutex();

#ifndef FF_INLAMNING
			LOGWARNING("Destroying Actor: {}", myCurrentID);
#endif
		}
	}
}