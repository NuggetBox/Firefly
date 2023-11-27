#pragma once

#include "Firefly/Core/Core.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Utils/Math/Transform.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Physics/PhysicsCallbacks.h"

namespace Firefly
{
	enum class NonWalkableMode
	{
		NotAllow,
		Slide,
	};

	struct PhysicsCharacterControllerInfo
	{
		Utils::Vec3 SpawnPosition;
		int32_t Layer = 1;
		float Density = 10.f;
		float Height = 100.f;
		float Radius = 50.f;
		NonWalkableMode NotWalkableMode = NonWalkableMode::Slide;
		float MaxJumpHeight = 100.f;
		float SlopeLimit = 45;
		float StepOffset = 1.f;
		float ContactOffset = 5.f;
		size_t EntityID = 0;
	};
	class PhysicsCharacterController
	{
	public:
		PhysicsCharacterController(const PhysicsCharacterControllerInfo& aInfo);
		physx::PxController* GetControllerPtr() { return myPhysXController; }
		physx::PxFilterData& FilterData() { return myFilterData; }
		physx::PxRigidDynamic* GetActor() { if (myPhysXController) return myPhysXController->getActor(); return nullptr; }
		static Ref<PhysicsCharacterController> Create(const PhysicsCharacterControllerInfo& aInfo);
		void Release();
		~PhysicsCharacterController();
	private:
		physx::PxController* myPhysXController = nullptr;
		PhysicsCharacterHitCallback* myHitCallback;
		physx::PxFilterData myFilterData;
		PhysicsCharacterBehaivorCallback myCallback;
	};
}