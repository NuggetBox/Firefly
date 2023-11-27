#include "FFpch.h"
#include "RigidbodyComponent.h"

#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Application/Application.h"
#include <Firefly/Components/Physics/SphereColliderComponent.h>
#include <Firefly/Components/Physics/BoxColliderComponent.h>
#include <Firefly/Components/Physics/MeshColliderComponent.h>
#include <Firefly/Physics/PhysicsActor.h>

#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Physics/PhysicsScene.h"

#include "Utils/Timer.h"

namespace Firefly
{
#define FF_LOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockWrite();
#define FF_UNLOCK_WRITE if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockWrite();

#define FF_LOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->lockRead();
#define FF_UNLOCK_READ if(const auto scene = SceneManager::Get().GetPhysicsScene()) if(auto ptr = scene->GetScenePtr()) ptr->unlockRead();

	REGISTER_COMPONENT(RigidbodyComponent);

	RigidbodyComponent::RigidbodyComponent() : Component("RigidbodyComponent")
	{
		EditorVariable("Layer", ParameterType::Int, &myLayer);
		EditorVariable("Mass", ParameterType::Float, &myMass);
		EditorVariable("Angular Drag", ParameterType::Float, &myAngularDrag);
		EditorVariable("Gravity", ParameterType::Bool, &myUseGravity);
		EditorVariable("Static", ParameterType::Bool, &myIsStatic);
		EditorVariable("Use Children Colliders", ParameterType::Bool, &myUseChildrenColliders);

		EditorHeader("Lock Transformations");
		EditorVariable("Lock Position X", ParameterType::Bool, &myLockPosition.x);
		EditorVariable("Lock Position Y", ParameterType::Bool, &myLockPosition.y);
		EditorVariable("Lock Position Z", ParameterType::Bool, &myLockPosition.z);
		EditorVariable("Lock Rotation X", ParameterType::Bool, &myLockRotation.x);
		EditorVariable("Lock Rotation Y", ParameterType::Bool, &myLockRotation.y);
		EditorVariable("Lock Rotation Z", ParameterType::Bool, &myLockRotation.z);
		EditorEndHeader();

		myLayer = 1;
		myUseChildrenColliders = false;
	}

	RigidbodyComponent::~RigidbodyComponent()
	{
		CleanUp();
	}

	void RigidbodyComponent::EarlyInitialize()
	{

	}

	void RigidbodyComponent::Initialize()
	{
#ifndef FF_SHIPIT
		if (Application::Get().GetIsInPlayMode())
		{
#endif // FF_SHIPIT
			PhysicsActorInfo info{};

			info.Transform = myEntity->GetTransform();
			bool canConstruct = false;

			GetShapesFromColliderComponents(info, canConstruct);
			info.UseGravity = myUseGravity;
			info.EntityID = myEntity->GetID();
			info.Filter = myLayer;
			info.LockPosition = myLockPosition;
			info.LockRotation = myLockRotation;
			if (canConstruct)
			{
 				InitializeActor(info);
			}
#ifndef FF_SHIPIT
		}

		else
		{
			CleanUp();
		}
#endif // FF_SHIPIT
	}

	void RigidbodyComponent::InitializeActor(Firefly::PhysicsActorInfo& info)
	{
		info.Type = myIsStatic ? ActorType::Static : ActorType::Dynamic;
		myActor = PhysicsActor::Create(info);
		myActor->AddToScene();
		myActor->SetMass(myMass);
		myActor->SetAngularDamping(myAngularDrag);

		if (info.LockPosition.x)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_X, true);
		}
		if (info.LockPosition.y)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_Y, true);
		}
		if (info.LockPosition.z)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_Z, true);
		}
		if (info.LockRotation.x)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_X, true);
		}
		if (info.LockRotation.y)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_Y, true);
		}
		if (info.LockRotation.z)
		{
			myActor->SetRigidFlags(PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_Z, true);
		}
	}

	void RigidbodyComponent::GetShapesFromColliderComponents(Firefly::PhysicsActorInfo& info, bool& canConstruct)
	{
		auto collectAndEmplaceFromEntity = [&](Entity* aEntity)
		{
			auto sphereCollider = aEntity->GetComponent<SphereColliderComponent>().lock();
			auto boxCollider = aEntity->GetComponent<BoxColliderComponent>().lock();
			auto meshCollider = aEntity->GetComponent<MeshColliderComponent>().lock();

			auto posDiff = aEntity->GetTransform().GetPosition() - myEntity->GetTransform().GetPosition();
			auto rotDiff = aEntity->GetTransform().GetQuaternion() * myEntity->GetTransform().GetQuaternion().GetInverse();
			auto localPose = physx::PxTransform(Physics::FFToPhysXVec3(posDiff), Physics::FFToPhysXQuat(rotDiff));

			if (sphereCollider)
			{
				auto& shape = info.PhysXShapes.emplace_back();
				localPose.p = Physics::FFToPhysXVec3(posDiff + sphereCollider->myOffset);
				localPose.q = Physics::FFToPhysXQuat(rotDiff);
				sphereCollider->myShape->setLocalPose(localPose);
				shape = sphereCollider->myShape;
				canConstruct = true;
			}

			if (boxCollider)
			{
				auto& shape = info.PhysXShapes.emplace_back();
				localPose.p = Physics::FFToPhysXVec3(posDiff + boxCollider->myOffset * boxCollider->myEntity->GetTransform().GetScale());
				localPose.q = Physics::FFToPhysXQuat(rotDiff);
				boxCollider->myShape->setLocalPose(localPose);
				shape = boxCollider->myShape;
				canConstruct = true;
			}

			if (meshCollider)
			{
				for (auto& shape : meshCollider->myShapes)
				{
					auto& rigidShape = info.PhysXShapes.emplace_back();
					rigidShape = shape;
					canConstruct = true;
				}
			}
		};

		if (myUseChildrenColliders)
		{
			auto children = myEntity->GetChildrenRecursive();
			for (auto child : children)
			{
				if (!child.expired())
				{
					collectAndEmplaceFromEntity(child.lock().get());
				}
			}
		}

		collectAndEmplaceFromEntity(myEntity);
	}

	void RigidbodyComponent::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppFixedUpdateEvent>(BIND_EVENT_FN(RigidbodyComponent::OnFixedUpdateEvent));
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(RigidbodyComponent::OnUpdateEvent));
		dispatcher.Dispatch<EntityOnComponentEnableEvent>([&](EntityOnComponentEnableEvent& aEvent)
			{
				Enable(aEvent.GetStatus());
				return false;
			});
		dispatcher.Dispatch<EntityOnEnableEvent>([&](EntityOnEnableEvent& aEvent)
			{
				const auto active = GetIsActive() ? aEvent.GetStatus() : false;
				
				Enable(active);
				return false;
			});
	}

	bool RigidbodyComponent::OnFixedUpdateEvent(AppFixedUpdateEvent& aEvent)
	{
		if (Application::Get().GetIsInPlayMode())
		{
			if (myActor && myEntity)
			{
				myActor->SyncTransform(myEntity->GetTransform());
			}
		}

		return false;
	}

	bool RigidbodyComponent::OnUpdateEvent(AppUpdateEvent& aEvent)
	{
		return false;
	}

	void RigidbodyComponent::AddForce(const Utils::Vec3& aForce, Physics::ForceType aForceType)
	{
		if (myActor)
		{
			myActor->AddForce(aForce, aForceType);
		}
	}

	void RigidbodyComponent::AddTorque(const Utils::Vec3& aTorque, Physics::ForceType aForceType)
	{
		if (myActor)
		{
			myActor->AddTorque(aTorque, aForceType);
		}
	}

	void RigidbodyComponent::Teleport(const Utils::Vec3& aPosition)
	{
		if (myActor)
		{
			myActor->SetPosition(aPosition);
		}
	}

	void RigidbodyComponent::SetRotation(const Utils::Quaternion& aRotation)
	{
		if (myActor)
		{
			myActor->SetRotation(aRotation);
		}
	}

	void RigidbodyComponent::SetScale(const Utils::Vec3& aScale)
	{
		if (myActor)
		{
			auto meshCollider = GetComponent<MeshColliderComponent>().lock();
			auto boxCollider = GetComponent<BoxColliderComponent>().lock();
			auto sphereCollider = GetComponent<SphereColliderComponent>().lock();

			myEntity->GetTransform().SetScale(aScale);

			if (meshCollider)
			{
				meshCollider->SetScale(aScale);
			}
			if (boxCollider)
			{
				boxCollider->SetScale(aScale);
			}
			if (sphereCollider)
			{
				sphereCollider->SetRadius(Utils::Max(aScale.x, Utils::Max(aScale.y, aScale.z)));
			}
		}
	}

	void RigidbodyComponent::SetAngularDamping(const float& aAngularDamping)
	{
		if (myActor)
		{
			myActor->SetAngularDamping(aAngularDamping);
		}
	}

	float RigidbodyComponent::GetAngularDamping()
	{
		if (myActor)
		{
			FF_LOCK_READ;
			const auto angularDamp = myActor->GetAngularDamping();
			FF_UNLOCK_READ;
			return angularDamp;
		}
		return 0.0f;
	}

	void RigidbodyComponent::Enable(const bool aActive)
	{
		if (myActor)
		{
			auto pxActor = myActor->GetActor();

			FF_LOCK_WRITE;
			pxActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !aActive);
			FF_UNLOCK_WRITE;
		}
	}

	void RigidbodyComponent::SetDamping(const float& aDamping)
	{
		if (myActor)
		{
			FF_LOCK_WRITE;
			myActor->SetDamping(aDamping);
			FF_UNLOCK_WRITE;
		}
	}

	float RigidbodyComponent::GetDamping()
	{
		if (myActor)
		{
			FF_LOCK_READ;
			const auto damping = myActor->GetDamping();
			FF_UNLOCK_READ;

			return damping;
		}
		return 0.0f;
	}

	void RigidbodyComponent::SetGravity(const bool& aBool)
	{
		if (myActor)
		{
			FF_LOCK_WRITE;
			myActor->GetActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !aBool);
			FF_UNLOCK_WRITE;
		}
	}

	void RigidbodyComponent::SetStatic(bool aBool)
	{
		myIsStatic = aBool;
	}

	void RigidbodyComponent::CleanUp()
	{
		if (myActor)
		{
			myActor.reset();
		}
	}
}
