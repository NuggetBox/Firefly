#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Firefly/Physics/PhysicsCharacterController.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Physics/PhysicsCallbacks.h"

namespace Firefly
{
	class CharacterControllerComponent : public Component
	{
	public:

		CharacterControllerComponent();
		~CharacterControllerComponent() override;

		void Initialize() override;

		void OnEvent(Firefly::Event& aEvent) override;
		bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

		void Teleport(const Utils::Vec3& aPosition);

		void SetOffset(const Utils::Vec3& aOffset);
		Utils::Vec3 GetOffset() { return myOffset; }

		void Jump(const float& aJumpForce);
		void StopJump();

		Physics::CharacterState GetState();

		Utils::Vec3 GetDeltaXP(); //!< delta position vector for the object the CCT is standing/riding on. Not always match the CCT delta when variable timesteps are used.

		void Move(const Utils::Vec3& aMoveVector);

		Utils::Vec3 GetLinearVelocity();

		bool IsGrounded();
		bool IsMovingUp();
		bool GetJumping() { return myJump; }

		void AddForce(const Utils::Vec3& aForce, bool aResetGravity = true);
		void RemoveForce();

		Ref<PhysicsCharacterController> GetCCT() { return myController; }

		float GetStepOffset() { return myStepOffset; }

		float GetRadius() { return myRadius; }

		Utils::Vec3 GetMiddlePoint();

		void ResetAirTime();

		void Enable(bool aFlag);

		static std::string GetFactoryName() { return "CharacterControllerComponent"; }
		static Ref<Component> Create() { return CreateRef<CharacterControllerComponent>(); }
	private:
		void MoveInternal(Utils::Vec3 aMoveVector, float aDeadLock = 0.1f);
		// creates the Controller.
		void CreateControllerObject();

		// sync graphics and physics
		void SyncTransform();

		int32_t myLayer = 1;
		float myDensity = 10.f;
		float myHeight = 100.f;
		float myRadius = 50.f;
		NonWalkableMode myNotWalkableMode = NonWalkableMode::Slide;
		float myMaxJumpHeight = 100.f;
		float mySlopeLimit = 45;
		float myStepOffset = 1.f;
		float myContactOffset = 5.f;
		float myGravityMultiplier = 1.f;
		Ref<PhysicsCharacterController> myController;
		physx::PxControllerFilters myFilter;
		physx::PxFilterData myFilterData;
		float myJumpStrength;
		Utils::Vec3 myOffset = 0.0f;
		Utils::Vec3 myFootOffset;
		Utils::Vec3 myJumpVector;
		Utils::Vec3 myMoveVector;
		Utils::Vec3 myForceVector;
		Utils::Vec3 myStartForce;
		physx::PxControllerCollisionFlags myFlag;

		physx::PxControllerState myControllerStates;

		float myAirTimeTimer = 0;
		bool myJump = false;
		bool myDrawDebugLines = true;
		bool myEnabled = true;
		PhysicQueryFilterCallback myFilterCallback;
	};
}