#include "FFpch.h"
#include "FixedJointComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/EntityEvents.h"

namespace Firefly
{
	REGISTER_COMPONENT(FixedJointComponent);

	FixedJointComponent::FixedJointComponent() : Component("FixedJointComponent")
	{
		EditorVariable("Connected Entity", ParameterType::Entity, &myConnectedEntity);
		EditorVariable("Break Force", ParameterType::Float, &myBreakForce);
		EditorVariable("Break Torque", ParameterType::Float, &myBreakTorque);
		EditorVariable("Should Collide", ParameterType::Bool, &myShouldCollide);
	}

	FixedJointComponent::~FixedJointComponent()
	{
		if (myJoint)
		{
			myJoint->release();
		}
	}

	void FixedJointComponent::Initialize()
	{
		myTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		myConnectedTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		CreateJoint();
	}

	void Firefly::FixedJointComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(FixedJointComponent::OnUpdateEvent));
		dispatcher.Dispatch<AppFixedUpdateEvent>(BIND_EVENT_FN(FixedJointComponent::OnFixedUpdateEvent));
		dispatcher.Dispatch<EntityPropertyUpdatedEvent>(BIND_EVENT_FN(FixedJointComponent::PropertyUpdatedEvent));
	}

	bool FixedJointComponent::OnUpdateEvent(AppUpdateEvent& aEvent)
	{
		if (!myJoint && aEvent.GetIsInPlayMode())
			CreateJoint();
		return false;
	}

	bool Firefly::FixedJointComponent::OnFixedUpdateEvent(Firefly::AppFixedUpdateEvent& aEvent)
	{
		if (!aEvent.GetIsInPlayMode())
		{
			return false;
		}

		if (!myConnectedRigidBody.expired())
		{
			myConnectedRigidBody.lock()->AddForce({ 0,0,0 });
		}

		return false;
	}

	bool FixedJointComponent::PropertyUpdatedEvent(Firefly::EntityPropertyUpdatedEvent& aEvent)
	{
		if (Application::Get().GetIsInPlayMode())
			SetVariables();
		return false;
	}

	void FixedJointComponent::SetHitPoint(const Utils::Vec3& aHitPoint)
	{
		myConnectedTransform.p = Physics::FFToPhysXVec3(aHitPoint);
		myConnectedEntity = {};
		CreateJoint();
	}

	void FixedJointComponent::SetConnectedEntity(Ptr<Entity> aEntity)
	{
		myConnectedEntity = aEntity;
		myConnectedTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		CreateJoint();
	}

	void FixedJointComponent::SetVariables()
	{
		if (!myJoint)
			return;

		myJoint->setBreakForce(myBreakForce, myBreakTorque);
		myJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, myShouldCollide);
	}

	physx::PxRigidActor* FixedJointComponent::GenerateActor()
	{
		if (!myConnectedEntity.expired())
		{
			myConnectedRigidBody = myConnectedEntity.lock()->GetComponent<RigidbodyComponent>();
			if (!myConnectedRigidBody.expired() && myConnectedRigidBody.lock()->GetActor() && myConnectedRigidBody.lock()->GetActor()->GetActor())
			{
				return myConnectedRigidBody.lock()->GetActor()->GetActor();
			}
		}
		return nullptr;
	}

	void FixedJointComponent::CreateJoint()
	{
		if (!Application::Get().GetIsInPlayMode() || myEntity->GetComponent<RigidbodyComponent>().expired())
		{
			return;
		}

		auto actor = GenerateActor();
		if (!myConnectedEntity.expired() && !actor)
		{
			return;
		}

		if (myJoint)
		{
			myJoint->release();
		}
		myConnectedTransform.p = Physics::FFToPhysXVec3(myEntity->GetTransform().GetPosition() - myConnectedEntity.lock()->GetTransform().GetPosition());

		myJoint = physx::PxFixedJointCreate(*PhysicsImplementation::GetPhysics(), myEntity->GetComponent<RigidbodyComponent>().lock()->GetActor()->GetActor(),
			myTransform, actor, myConnectedTransform);
		SetVariables();
	}
}