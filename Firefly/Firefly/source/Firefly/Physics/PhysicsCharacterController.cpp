#include "FFpch.h"
#include "PhysicsCharacterController.h"

#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsLayerHandler.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/Physics/PhysicsActor.h"
#include "Firefly/Physics/PhysicsScene.h"
#include "Firefly/Application/Application.h"

namespace Firefly
{
#define FF_LOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockWrite();
#define FF_UNLOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockWrite();

#define FF_LOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockRead();
#define FF_UNLOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockRead();

	Firefly::PhysicsCharacterController::PhysicsCharacterController(const PhysicsCharacterControllerInfo& aInfo)
	{
		auto physicsScene = SceneManager::Get().GetPhysicsScene();
		if (!physicsScene)
		{
			LOGERROR("Physics scene was null!");
			return;
		}
		Application::Get().LockPhysXSimulationMutex();
		physicsScene->GetScenePtr()->lockWrite();
		physx::PxCapsuleControllerDesc desc;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
		desc.density = aInfo.Density;
		physx::PxExtendedVec3 ex = { aInfo.SpawnPosition.x, aInfo.SpawnPosition.y, aInfo.SpawnPosition.z};
		desc.position = ex;
		desc.height = aInfo.Height;
		desc.nonWalkableMode = (physx::PxControllerNonWalkableMode::Enum)aInfo.NotWalkableMode;
		desc.maxJumpHeight = aInfo.MaxJumpHeight;
		desc.scaleCoeff = 0.8;
		desc.slopeLimit = aInfo.SlopeLimit;
		desc.invisibleWallHeight = 0.5f;
		desc.contactOffset = aInfo.ContactOffset;
		desc.stepOffset = aInfo.StepOffset;
		desc.userData = (void*)aInfo.EntityID;
		desc.radius = aInfo.Radius;
		desc.behaviorCallback = &myCallback;
		myHitCallback = new PhysicsCharacterHitCallback();
		desc.registerDeletionListener = true;
		desc.reportCallback = myHitCallback;
		auto physics = PhysicsImplementation::GetPhysics();
		desc.material = physics->createMaterial(0.5f, 0.5f, 0.5f);
		if (!desc.isValid())
		{
			LOGERROR("Invalid Parameters for Character controller!");
		}

		myPhysXController = SceneManager::Get().GetPhysicsScene()->GetCharacterControllerPtr()->createController(desc);
		//myPhysXController->getActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
		myPhysXController->getActor()->setContactReportThreshold(0);
		physx::PxShape* p;
		myPhysXController->getActor()->getShapes(&p, 1);
		myPhysXController->getActor()->userData = (void*)aInfo.EntityID;
		
		myFilterData.word0 = aInfo.Layer; // word0 = own ID
		myFilterData.word1 = PhysicsLayerHandler::GetLayersToCollideWith(aInfo.Layer);
		p->setSimulationFilterData(myFilterData);
		p->setQueryFilterData(myFilterData);

		physicsScene->GetScenePtr()->unlockWrite();
		Application::Get().UnlockPhysXSimulationMutex();
	}

	Ref<PhysicsCharacterController> Firefly::PhysicsCharacterController::Create(const PhysicsCharacterControllerInfo& aInfo)
	{
		return CreateRef<PhysicsCharacterController>(aInfo);
	}

	void PhysicsCharacterController::Release()
	{
		if (myPhysXController)
		{
			FF_LOCK_WRITE;
			PHYSX_SAFE_DESTROY(myPhysXController);
			if (myHitCallback)
			{
				delete myHitCallback;
				myHitCallback = nullptr;
			}
			FF_UNLOCK_WRITE;
		}
	}

	PhysicsCharacterController::~PhysicsCharacterController()
	{
		if (!Application::Get().GetIsInPlayMode())
		{
			return;
		}

		const auto& phys = SceneManager::Get().GetPhysicsScene();
		if (!phys)
		{
			LOGERROR("PhysX scene NULL");
			return;
		}

		const auto scenePtr = phys->GetScenePtr();
		if (!scenePtr)
		{
			LOGERROR("PhysX pointer NULL");
			return;
		}

		scenePtr->lockWrite();

		PHYSX_SAFE_DESTROY(myPhysXController);
		if (myHitCallback)
		{
			delete myHitCallback;
			myHitCallback = nullptr;
		}

		scenePtr->unlockWrite();
	}
}