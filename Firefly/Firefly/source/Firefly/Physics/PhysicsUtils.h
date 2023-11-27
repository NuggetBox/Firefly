#pragma once
#include "Utils/Math/Vector.h"
#include "Utils/Math/Quaternion.h"
#include "Utils/Math/Transform.h"
#include <PhysX/foundation/Px.h>

#define PHYSX_SAFE_DESTROY(X) {if(X){X->release(); X = nullptr;}}

namespace Firefly
{

	namespace Physics
	{
		enum class CharacterState
		{
			Grounded, // when the controller connecting on the bottom.
			Sides, // when the controller is connecting on a side.
			HeadStand, // when the top of the controller is connecting with something.
			None,
		};

		enum class ForceType
		{
			Force,				//!< parameter has unit of mass * length / time^2, i.e., a force
			Impulse,			//!< parameter has unit of mass * length / time, i.e., force * time
			VelocityChange,	//!< parameter has unit of length / time, i.e., the effect is mass independent: a velocity change.
			Acceleration		//!< parameter has unit of length/ time^2, i.e., an acceleration. It gets treated just like a force except the mass is not divided out before integration.
		};

		enum class ShapeType
		{
			Box,
			Sphere
		};
		
		CharacterState PhysXFlagToFireflyFlag(const physx::PxU32& state);

		Utils::Vector3f PhysXToFFVec3(const physx::PxVec3& aVec3);
		physx::PxVec3 FFToPhysXVec3(const Utils::Vector3f& aVec3);

		Utils::Quaternion PhysXToFFQuat(const physx::PxQuat& aQuat);
		physx::PxQuat FFToPhysXQuat(const Utils::Quaternion& aQuat);

		Utils::Transform PhysXToFFTransform(const physx::PxTransform& aTransform);
		physx::PxTransform FFToPhysXTransform(const Utils::Transform& aTransform);
	}
}