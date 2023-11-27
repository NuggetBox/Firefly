#include "FFpch.h"
#include "CharacterControllerComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Physics/PhysicsActor.h"
#include "Firefly/Physics/PhysicsScene.h"
#include "Firefly/Event/EntityEvents.h"
#include "Utils/Timer.h"
#include "Firefly/Physics/PhysicsLayerHandler.h"
#include "Firefly/ComponentSystem/SceneManager.h"
namespace Firefly
{
	REGISTER_COMPONENT(CharacterControllerComponent);

	CharacterControllerComponent::CharacterControllerComponent() : Component("CharacterControllerComponent")
	{
		myJumpStrength = 0.f;

		EditorVariable("Layer", ParameterType::Int, &myLayer);
		EditorVariable("Debug Lines", ParameterType::Bool, &myDrawDebugLines);
		EditorHeader("Collider Properties");
		EditorVariable("Height", ParameterType::Float, &myHeight);
		EditorVariable("Radius", ParameterType::Float, &myRadius);
		EditorVariable("Offset", ParameterType::Vec3, &myOffset);
		EditorVariable("Density", ParameterType::Float, &myDensity);
		EditorEndHeader();

		EditorHeader("Controller Properties");
		EditorVariable("Slope Limit", ParameterType::Float, &mySlopeLimit);
		EditorVariable("Step Offset", ParameterType::Float, &myStepOffset);
		EditorVariable("Gravity Multiplier", ParameterType::Float, &myGravityMultiplier);
		EditorVariable("Slope Mode", ParameterType::Enum, &myNotWalkableMode, [=](uint32_t index)
			{
				if (index == 0)
				{
					return "Stick";
				}
				else
				{
					return "Slide";
				}

				return "Invalid";
			}, 2);
		EditorVariable("Contact Offset", ParameterType::Float, &myContactOffset);
		EditorEndHeader();

	}

	CharacterControllerComponent::~CharacterControllerComponent()
	{
		if (!Application::Get().GetIsInPlayMode())
		{
			return;
		}

		auto physicsScene = SceneManager::Get().GetPhysicsScene();
		if (!physicsScene)
		{
			LOGERROR("Physics scene was null!");
			return;
		}

		Application::Get().LockPhysXSimulationMutex();

		myController.reset();

		Application::Get().UnlockPhysXSimulationMutex();
	}

	void CharacterControllerComponent::Initialize()
	{
		myController.reset();

		if (!myController)
		{
			if (Application::Get().GetIsInPlayMode())
			{
				CreateControllerObject();
			}
			else
			{
				Application::Get().LockPhysXSimulationMutex();
				myController.reset();
				Application::Get().UnlockPhysXSimulationMutex();
			}
		}
	}
	void CharacterControllerComponent::CreateControllerObject()
	{
		auto physScene = SceneManager::Get().GetPhysicsScene()->GetScenePtr();

		PhysicsCharacterControllerInfo info{};
		info.SpawnPosition = myEntity->GetTransform().GetPosition();
		info.EntityID = myEntity->GetID();
		info.SlopeLimit = cosf(DEGTORAD(mySlopeLimit));

		info.ContactOffset = myContactOffset;
		info.Density = myDensity;
		info.NotWalkableMode = myNotWalkableMode;
		info.Height = myHeight;
		info.Radius = myRadius;
		info.StepOffset = myStepOffset;
		info.Layer = myLayer;

		myController = PhysicsCharacterController::Create(info);

		myController->GetControllerPtr()->getState(myControllerStates);

		SetOffset(myOffset);


	}
	void CharacterControllerComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& aEvent)
			{
				if (aEvent.GetParamName() == "Offset")
					SetOffset(myOffset);
				return false;
			});
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(CharacterControllerComponent::OnUpdateEvent));
		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& aEvent)
			{
#ifndef FF_SHIPIT
				// Niklas: render a capsule :D slow but fine because we prob should not have more than 10 controller ever (surely)
				if (myDrawDebugLines)
				{
					Renderer::SubmitDebugSphere(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, myHeight / 2.f, 0), myRadius);
					Renderer::SubmitDebugSphere(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, -myHeight / 2.f, 0), myRadius);
					Renderer::SubmitDebugLine(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(myRadius, -myHeight / 2.f, 0), myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(myRadius, myHeight / 2.f, 0));
					Renderer::SubmitDebugLine(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(-myRadius, -myHeight / 2.f, 0), myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(-myRadius, myHeight / 2.f, 0));
					Renderer::SubmitDebugLine(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, -myHeight / 2.f, myRadius), myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, myHeight / 2.f, myRadius));
					Renderer::SubmitDebugLine(myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, -myHeight / 2.f, -myRadius), myEntity->GetTransform().GetPosition() + myOffset + Utils::Vec3(0, myHeight / 2.f, -myRadius));
					Renderer::SubmitDebugCircle(myEntity->GetTransform().GetPosition() + myOffset, myRadius);
				}
#endif
				return false;
			});
	}
	bool CharacterControllerComponent::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
	{
		FF_PROFILESCOPE("Char Comp");
		if (aEvent.GetIsInPlayMode() && myController && myEnabled)
		{
			Teleport(myEntity->GetTransform().GetPosition() + myFootOffset);

			auto deltaTime = Utils::Timer::GetDeltaTime();

			if (!IsGrounded())
			{
				myAirTimeTimer += deltaTime;
				myJumpVector = (SceneManager::Get().GetPhysicsScene()->GetGravity() * myGravityMultiplier) * myAirTimeTimer;
			}
			else
			{
				myAirTimeTimer = 0;
			}

			if (myJump)
			{
				myJumpVector.y += myJumpStrength;
			}

			if (myForceVector.Dot(myStartForce) > 0)
			{
				float groundedMod = 1.3f;
				if (!IsGrounded())
				{
					groundedMod = 0.6f;
				}
				myForceVector -= myStartForce * deltaTime * groundedMod;
			}
			else
			{
				myForceVector = Utils::Vec3::Zero();
				myStartForce = Utils::Vec3::Zero();
			}

			constexpr float DeadZone = 0.1f;
			MoveInternal(myJumpVector * deltaTime + myMoveVector + myForceVector * deltaTime, DeadZone);

			SyncTransform();
			myMoveVector = { 0,0,0 };
		}
		return false;
	}

	void CharacterControllerComponent::SyncTransform()
	{
		auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			if (myController && myController->GetControllerPtr())
			{
				scene->GetScenePtr()->lockWrite();
				auto& newPos = myController->GetControllerPtr()->getPosition();
				myEntity->GetTransform().SetPosition({ (float)newPos.x - myOffset.x,(float)newPos.y - myOffset.y,(float)newPos.z - myOffset.z });
				scene->GetScenePtr()->unlockWrite();
			}
		}
	}

	void CharacterControllerComponent::Teleport(const Utils::Vec3& aPosition)
	{
		auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			scene->GetScenePtr()->lockWrite();
			if (myController)
			{
				const auto controller = myController->GetControllerPtr();
				controller->setFootPosition({ aPosition.x , aPosition.y , aPosition.z });
			}
			scene->GetScenePtr()->unlockWrite();
		}
	}

	void CharacterControllerComponent::SetOffset(const Utils::Vec3& aOffset)
	{
		if (!myEnabled || !myController || myController->GetControllerPtr() == nullptr)
			return;
		myOffset = aOffset;
		auto p = myEntity->GetTransform().GetPosition();

		if (myController && myController->GetControllerPtr() != nullptr)
		{
			Application::Get().LockPhysXSimulationMutex();

			myController->GetControllerPtr()->getScene()->lockWrite();
			myController->GetControllerPtr()->setPosition({ p.x + myOffset.x, p.y + myOffset.y, p.z + myOffset.z });
			auto footPos = myController->GetControllerPtr()->getFootPosition();
			myFootOffset = Utils::Vec3(footPos.x, footPos.y, footPos.z) - p;
			myController->GetControllerPtr()->getScene()->unlockWrite();
			Application::Get().UnlockPhysXSimulationMutex();
		}
	}

	void CharacterControllerComponent::Jump(const float& aJumpForce)
	{
		myJump = true;
		myJumpStrength = aJumpForce;
		myAirTimeTimer = 0;
	}
	void CharacterControllerComponent::StopJump()
	{
		myJump = false;
		myJumpStrength = 0;
		myAirTimeTimer = 0;
	}

	Physics::CharacterState CharacterControllerComponent::GetState()
	{
		if (myEnabled && myController)
		{
			return Physics::PhysXFlagToFireflyFlag(myFlag);
		}

		return Physics::CharacterState::None;
	}

	Utils::Vec3 CharacterControllerComponent::GetDeltaXP()
	{
		if (myEnabled && myController)
		{
			const auto controller = myController->GetControllerPtr();
			return Physics::PhysXToFFVec3(myControllerStates.deltaXP);
		}

		return { 0,0,0 };
	}
	void CharacterControllerComponent::MoveInternal(Utils::Vec3 aMoveVector, float aDeadLock)
	{
		if (myEnabled && myController)
		{
			FF_PROFILESCOPE("Move internal");
			const auto controller = myController->GetControllerPtr();

			physx::PxControllerFilters filters{};
			filters.mFilterFlags |= physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;
			filters.mFilterData = &myController->FilterData();
			filters.mFilterCallback = new PhysicQueryFilterCallback();

			auto physicsScene = SceneManager::Get().GetPhysicsScene();
			if (!physicsScene)
			{
				LOGERROR("Physics scene was null!");
				return;
			}
			physicsScene->GetScenePtr()->lockWrite();

			myFlag = controller->move(Physics::FFToPhysXVec3(aMoveVector), aDeadLock, Utils::Timer::GetFixedDeltaTime(), filters);

			controller->getState(myControllerStates);
			if (myJump && IsGrounded())
			{
				StopJump();
			}

			physicsScene->GetScenePtr()->unlockWrite();
			delete filters.mFilterCallback;
			filters.mFilterCallback = nullptr;
		}
	}
	void CharacterControllerComponent::Move(const Utils::Vec3& aMoveVector)
	{
		myMoveVector += aMoveVector;
	}
	Utils::Vec3 CharacterControllerComponent::GetLinearVelocity()
	{
		const auto scene = SceneManager::Get().GetPhysicsScene();
		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->lockRead();
		}

		PxVec3 vel(0, 0, 0);

		if (myController)
		{
			if (const auto ptr = myController->GetControllerPtr())
			{
				if (const auto actor = ptr->getActor())
				{
					vel = actor->getLinearVelocity();
				}
			}
		}

		if (scene)
		{
			const auto ptr = scene->GetScenePtr();
			ptr->unlockRead();
		}

		return Utils::Vec3(vel.x, vel.y, vel.z);
	}
	bool CharacterControllerComponent::IsGrounded()
	{
		if (myFlag.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN) && myControllerStates.touchedActor)
		{
			return true;
		}
		return false;
	}

	bool CharacterControllerComponent::IsMovingUp()
	{
		return myControllerStates.isMovingUp;
	}

	void CharacterControllerComponent::AddForce(const Utils::Vec3& aForce, bool aResetGravity)
	{
		myForceVector = aForce;
		myStartForce = aForce;
		if (aResetGravity)
		{
			myJumpVector = { 0,0,0 };
			myAirTimeTimer = 0;
			myJump = false;
		}
	}

	void CharacterControllerComponent::RemoveForce()
	{
		myForceVector = 0;
		myStartForce = 0;
		myJumpVector = { 0,0,0 };
		myAirTimeTimer = 0;
		myJump = false;
	}

	Utils::Vec3 CharacterControllerComponent::GetMiddlePoint()
	{
		if (myEnabled && myController && myController->GetControllerPtr())
		{
			const auto scene = SceneManager::Get().GetPhysicsScene();
			if (scene)
			{
				const auto ptr = scene->GetScenePtr();
				ptr->lockWrite();
			}

			auto pos = myController->GetControllerPtr()->getPosition();

			if (scene)
			{
				const auto ptr = scene->GetScenePtr();
				ptr->unlockWrite();
			}

			return Utils::Vec3(pos.x, pos.y, pos.z);
		}
		return Utils::Vec3();
	}

	void CharacterControllerComponent::ResetAirTime()
	{
		StopJump();
		myJumpVector = Utils::Vec3::Zero();
	}

	void CharacterControllerComponent::Enable(bool aFlag)
	{
		if (myController && myController->GetActor())
		{
			if (!aFlag)
			{
				myController->Release();
				myController = nullptr;
				myEnabled = false;
			}
		}
	}
}