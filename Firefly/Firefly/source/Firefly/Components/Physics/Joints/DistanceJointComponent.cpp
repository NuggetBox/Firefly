#include "FFpch.h"
#include "DistanceJointComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"

#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/EntityEvents.h"

namespace Firefly
{
	REGISTER_COMPONENT(DistanceJointComponent);

	DistanceJointComponent::DistanceJointComponent() : Component("DistanceJointComponent")
	{
		EditorVariable("Connected Entity", ParameterType::Entity, &myConnectedEntity);
		EditorVariable("Spring", ParameterType::Float, &mySpring);
		EditorVariable("Damper", ParameterType::Float, &myDamper);
		EditorVariable("Tolerance", ParameterType::Float, &myTolerance);
		EditorVariable("Mass Scale", ParameterType::Float, &myMassScale);
		EditorVariable("Min Distance", ParameterType::Float, &myMinDistance);
		EditorVariable("Max Distance", ParameterType::Float, &myMaxDistance);
		EditorVariable("Break Force", ParameterType::Float, &myBreakForce);
		EditorVariable("Break Torque", ParameterType::Float, &myBreakTorque);
		EditorVariable("Should Collide", ParameterType::Bool, &myShouldCollide);
	}

	DistanceJointComponent::~DistanceJointComponent()
	{
		if (myJoint)
		{
			myJoint->release();
		}
	}

	void DistanceJointComponent::Initialize()
	{
		myTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		myConnectedTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		CreateJoint();
	}

	void Firefly::DistanceJointComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(DistanceJointComponent::OnUpdateEvent));
		dispatcher.Dispatch<AppFixedUpdateEvent>(BIND_EVENT_FN(DistanceJointComponent::OnFixedUpdateEvent));
		dispatcher.Dispatch<EntityPropertyUpdatedEvent>(BIND_EVENT_FN(DistanceJointComponent::PropertyUpdatedEvent));
	}

	bool DistanceJointComponent::OnUpdateEvent(AppUpdateEvent& aEvent)
	{
		if (!myJoint && aEvent.GetIsInPlayMode())
			CreateJoint();
		return false;
	}

	bool Firefly::DistanceJointComponent::OnFixedUpdateEvent(Firefly::AppFixedUpdateEvent& aEvent)
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

	bool DistanceJointComponent::PropertyUpdatedEvent(Firefly::EntityPropertyUpdatedEvent& aEvent)
	{
		if (Application::Get().GetIsInPlayMode())
			SetVariables();
		return false;
	}

	void DistanceJointComponent::SetHitPoint(const Utils::Vec3& aHitPoint)
	{
		myConnectedTransform.p = Physics::FFToPhysXVec3(aHitPoint);
		myConnectedEntity = {};
		CreateJoint();
	}

	void DistanceJointComponent::SetConnectedEntity(Ptr<Entity> aEntity)
	{
		myConnectedEntity = aEntity;
		myConnectedTransform = physx::PxTransform(PxIDENTITY::PxIdentity);
		CreateJoint();
	}

	void DistanceJointComponent::SetVariables()
	{
		if (!myJoint)
			return;

		myJoint->setStiffness(mySpring);
		myJoint->setTolerance(myTolerance);
		myJoint->setDamping(myDamper);
		myJoint->setBreakForce(myBreakForce, myBreakTorque);
		myJoint->setMaxDistance(myMaxDistance);
		myJoint->setMinDistance(myMinDistance);
		myJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, myShouldCollide);
	}

	physx::PxRigidActor* DistanceJointComponent::GenerateActor()
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

	void DistanceJointComponent::CreateJoint()
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

		myJoint = physx::PxDistanceJointCreate(*PhysicsImplementation::GetPhysics(), myEntity->GetComponent<RigidbodyComponent>().lock()->GetActor()->GetActor(),
			myTransform, actor, myConnectedTransform);
		SetVariables();
	}
}