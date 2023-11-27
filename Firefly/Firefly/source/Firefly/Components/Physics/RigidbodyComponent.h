#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include <Firefly/Physics/PhysicsActor.h>

namespace Firefly
{
	class RigidbodyComponent : public Component
	{
	public:
		RigidbodyComponent();
		~RigidbodyComponent() override;

		void EarlyInitialize() override;
		void Initialize() override;

		void OnEvent(Firefly::Event& aEvent) override;
		bool OnFixedUpdateEvent(AppFixedUpdateEvent& aEvent);
		bool OnUpdateEvent(AppUpdateEvent& aEvent);

		void AddForce(const Utils::Vec3& aForce, Physics::ForceType aForceType = Physics::ForceType::Force);
		void AddTorque(const Utils::Vec3& aTorque, Physics::ForceType aForceType = Physics::ForceType::Force);

		void Teleport(const Utils::Vec3& aPosition);
		void SetRotation(const Utils::Quaternion& aRotation);
		void SetScale(const Utils::Vec3& aScale);
		void SetAngularDamping(const float& aAngularDamping);
		float GetAngularDamping();
		void Enable(const bool aActive);
		void SetDamping(const float& aDamping);
		float GetDamping();
		const uint32_t GetLayer() { return myLayer; } const

		Ref<PhysicsActor> GetActor() { return myActor; }

		void SetGravity(const bool& aBool);
		void SetStatic(bool aBool);

		static std::string GetFactoryName() { return "RigidbodyComponent"; }
		static Ref<Component> Create() { return CreateRef<RigidbodyComponent>(); }
	private:
		void GetShapesFromColliderComponents(Firefly::PhysicsActorInfo& info, bool& canConstruct);

		void InitializeActor(Firefly::PhysicsActorInfo& info);

		void CleanUp();
		Ref<PhysicsActor> myActor = nullptr; 
		float myAngularDrag = 0.05f;
		float myMass = 100.f;
		uint32_t myLayer;
		bool myIsStatic = false;
		bool myUseGravity = true;
		bool myUseChildrenColliders = true;

		Utils::Vector3<bool> myLockPosition;
		Utils::Vector3<bool> myLockRotation;
	};
}