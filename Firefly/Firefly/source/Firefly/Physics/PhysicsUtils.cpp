#include "FFpch.h"
#include "PhysicsUtils.h"
#include "PhysX/PxPhysicsAPI.h"

namespace Firefly
{
	namespace Physics
	{


		/*physx::PxControllerCollisionFlag::Enum FireflyFlagToPhysXFlag(const CharacterState& state)
		{
			physx::PxControllerCollisionFlag::Enum PxState = physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
			switch (state)
			{
			case CharacterState::Grounded:
				PxState = physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
				break;
			case CharacterState::Sides:
				PxState = physx::PxControllerCollisionFlag::eCOLLISION_SIDES;
				break;
			case CharacterState::HeadStand:
				PxState = physx::PxControllerCollisionFlag::eCOLLISION_UP;
				break;
			default:
				break;
			}
			return PxState;
		}*/
		CharacterState PhysXFlagToFireflyFlag(const physx::PxU32& state)
		{
			CharacterState fireflyState = CharacterState::Grounded;

			switch (state)
			{
			case physx::PxControllerCollisionFlag::eCOLLISION_DOWN:
				fireflyState = CharacterState::Grounded;
				//std::cout << "GROUND" << std::endl;
				break;
			case physx::PxControllerCollisionFlag::eCOLLISION_SIDES:
				fireflyState = CharacterState::Sides;
				//std::cout << "SIDE" << std::endl;
				break;
			case physx::PxControllerCollisionFlag::eCOLLISION_UP:
				fireflyState = CharacterState::HeadStand;
				//std::cout << "UP" << std::endl;
				break;
			default:
				fireflyState = CharacterState::None;
				//std::cout << "NONE" << std::endl;
				break;
			}
			return fireflyState;
		}
		Utils::Vector3f Firefly::Physics::PhysXToFFVec3(const physx::PxVec3& aVec3)
		{
			return Utils::Vector3f(aVec3.x, aVec3.y, aVec3.z);
		}

		physx::PxVec3 Firefly::Physics::FFToPhysXVec3(const Utils::Vector3f& aVec3)
		{
			return { aVec3.x, aVec3.y, aVec3.z };
		}

		Utils::Quaternion Firefly::Physics::PhysXToFFQuat(const physx::PxQuat& aQuat)
		{
			return { aQuat.x, aQuat.y, aQuat.z, aQuat.w };
		}

		physx::PxQuat Firefly::Physics::FFToPhysXQuat(const Utils::Quaternion& aQuat)
		{
			return { aQuat.x, aQuat.y, aQuat.z, aQuat.w };
		}
		Utils::Transform PhysXToFFTransform(const physx::PxTransform& aTransform)
		{
			Utils::Transform trans;
			memcpy(&trans, &aTransform, sizeof(Utils::Transform));
			return trans;
		}
		physx::PxTransform FFToPhysXTransform(const Utils::Transform& aTransform)
		{
			physx::PxTransform trans;
			trans.p = FFToPhysXVec3(aTransform.GetPosition());
			trans.q = FFToPhysXQuat(aTransform.GetQuaternion());
			return trans;
		}
	}
}
