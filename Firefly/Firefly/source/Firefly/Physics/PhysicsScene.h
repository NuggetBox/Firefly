#pragma once
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Firefly/Physics/PhysicsCallbacks.h"

namespace Utils
{
	class Transform;
}

namespace Firefly
{
	class Entity;
	namespace Physics
	{
		enum class ShapeType;
	}

	struct RayResult
	{
		Ptr<Entity> HitEntity;
		Utils::Vec3 Normal;
		Utils::Vec3 Position;
		float Distance = -1;
		bool IsValid()
		{
			return !HitEntity.expired();
		};
		bool operator<(RayResult aOther)
		{
			if (this->Distance < aOther.Distance)
			{
				return true;
			}
			return false;
		}
	};

	struct PhysicsSceneInfo
	{
		Utils::Vector3f Gravity = { 0, -981.f, 0 };
	};

	class PhysicsScene
	{
	public:
		PhysicsScene(const PhysicsSceneInfo& aInfo);
		void CreateScene(const Firefly::PhysicsSceneInfo& aInfo);
		void CreateControllerManager();
		void ConnectDebugger();
		void SetGravity(const Utils::Vector3f& aGravity);
		Utils::Vector3f GetGravity();
		std::vector<Ptr<Entity>> BoxOverlap(const Utils::Transform& aTransform, const Utils::Vec3& aSize, const int aLayer = 0);
		std::vector<Ptr<Entity>> SphereOverlap(const Utils::Transform& aTransform, const float aRadius, const int aLayer = 0);
		//raycast all
		RayResult Raycast(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength = -1.f, const int aLayer = 0);
		std::vector<RayResult> RaycastAll(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength = -1.f, const int aLayer = 0);
		std::vector<RayResult> RaycastNoTrigger(const Utils::Vec3& aOrigin, const Utils::Vec3& aDirection, const float aLength = -1.f, const int aLayer = 0);

		physx::PxScene* GetScenePtr() { return myPhysXScene; }
		physx::PxControllerManager* GetCharacterControllerPtr() { return myPhysXControllerManager; }
		PhysicsCharacterBehaivorCallback& CallBack() { return myCallback; }
		PhysicsCharacterHitCallback& HitCallBack() { return myHitCallBack; };

		void Simulate(const float aFixedDeltaTime);
		void UpdateQuery();
		void UpdateControllerManager();
		static Ref<PhysicsScene> Create(const PhysicsSceneInfo& aInfo);
		~PhysicsScene();
	private:
		PhyicsSimulationCallback* mySimulationCallback;
		PhysicQueryFilterCallback myQueryFilterCallback;
		physx::PxControllerManager* myPhysXControllerManager;
		physx::PxScene* myPhysXScene;
		physx::PxPvdSceneClient* myCilent;
		PhysicsCharacterBehaivorCallback myCallback;
		PhysicsCharacterHitCallback myHitCallBack;
		PhyicsSimulationFilterCallback myFilterCallback;
	};
}
