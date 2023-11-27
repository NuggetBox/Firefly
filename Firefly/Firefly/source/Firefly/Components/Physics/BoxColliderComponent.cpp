#include "FFpch.h"
#include "BoxColliderComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Physics/PhysicsScene.h"

namespace Firefly
{
	REGISTER_COMPONENT(BoxColliderComponent);
	BoxColliderComponent::BoxColliderComponent() : Component("BoxColliderComponent")
	{
		myMaterial = nullptr;
		myShape = nullptr;
		myHalfSize = { 50, 50, 50 };

		EditorVariable("Half size", ParameterType::Vec3, &myOffsetSize);
		EditorVariable("Offset", ParameterType::Vec3, &myOffset);
		EditorVariable("Trigger", ParameterType::Bool, &myIsTigger);
		EditorHeader("physics material");
		EditorVariable("Static friction", ParameterType::Float, &myStaticFriction);
		EditorVariable("Dynamic friction", ParameterType::Float, &myDynamicFriction);
		EditorVariable("Bounciness", ParameterType::Float, &myBounciness);
		EditorEndHeader();
		myOffsetSize = { 1,1,1 };
		myIsTigger = false;
	}

	BoxColliderComponent::~BoxColliderComponent()
	{
		CleanUp();
	}

	void BoxColliderComponent::EarlyInitialize()
	{
#ifndef FF_SHIPIT 
		if (Application::Get().GetIsInPlayMode())
		{
#endif
			myHalfSize = myEntity->GetTransform().GetScale() * 50.f;
			auto physics = PhysicsImplementation::GetPhysics();
			myMaterial = physics->createMaterial(myStaticFriction, myDynamicFriction, myBounciness);
			myGeometry = physx::PxBoxGeometry(Physics::FFToPhysXVec3(myHalfSize * myOffsetSize));
			myShape = physics->createShape(myGeometry, &myMaterial, 1, false, physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
			myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !myIsTigger);
			myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, myIsTigger);
			physx::PxTransform pxTrans{};
			pxTrans.p = Physics::FFToPhysXVec3(myOffset * myEntity->GetTransform().GetScale());
			pxTrans.q = physx::PxQuat(0, 0, 0, 1);
			myShape->setLocalPose(pxTrans);
#ifndef FF_SHIPIT 
		}
		else
		{
			CleanUp();
		}
#endif
	}

	void BoxColliderComponent::Initialize()
	{
		LoadDebugBillboard();
	}

	void BoxColliderComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<EditorPlayEvent>(BIND_EVENT_FN(BoxColliderComponent::OnPlayEvent));
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(BoxColliderComponent::OnUpdateEvent));

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& e)
			{
				DrawDebugBillboard();
				return false;
			});
	}

	bool BoxColliderComponent::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
	{
		myHalfSize = myEntity->GetTransform().GetScale() * 50.f;
		Renderer::SubmitDebugCuboid(myEntity->GetTransform(), myOffset * myEntity->GetTransform().GetScale(), (myHalfSize) * (myOffsetSize * 2.f), { 1,1,1,1 });
		return false;
	}

	bool BoxColliderComponent::OnPlayEvent(EditorPlayEvent& aEvent)
	{
		return false;
	}

	void BoxColliderComponent::SetScale(const Utils::Vec3& aScale)
	{
		if (myShape)
		{
			myShape->getBoxGeometry(myGeometry);
			myHalfSize = aScale * 50.f;
			physx::PxTransform pxTrans{};
			pxTrans.p = Physics::FFToPhysXVec3(myOffset * aScale);
			pxTrans.q = physx::PxQuat(0, 0, 0, 1);
			myShape->setLocalPose(pxTrans);
			myGeometry.halfExtents = Physics::FFToPhysXVec3(myHalfSize * myOffsetSize);
			myShape->setGeometry(myGeometry);
		}
	}

	void BoxColliderComponent::SetOffset(const Utils::Vector3f& aOffset)
	{
		myOffset = aOffset;
	}

	void BoxColliderComponent::CleanUp()
	{
		if (!Application::Get().GetIsInPlayMode())
		{
			return;
		}

		const auto& phys = SceneManager::Get().GetPhysicsScene();
		if (!phys)
		{
			LOGERROR("PhysX scene NULL");
			return;
		}

		const auto scenePtr = phys->GetScenePtr();
		if (!scenePtr)
		{
			LOGERROR("PhysX pointer NULL");
			return;
		}

		Application::Get().LockPhysXSimulationMutex();
		scenePtr->lockWrite();

		PHYSX_SAFE_DESTROY(myShape);
		myMaterial = nullptr;
		//PHYSX_SAFE_DESTROY(myMaterial);

		scenePtr->unlockWrite();
		Application::Get().UnlockPhysXSimulationMutex();
	}

	void BoxColliderComponent::LoadDebugBillboard()
	{
#ifndef FF_SHIPIT
		myDebugBillboard = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_boxcollider.dds", true);
		myDebugBillboardInfo = CreateRef<BillboardInfo>();
		myDebugBillboardInfo->Texture = myDebugBillboard;
		myDebugBillboardInfo->Color = { 1, 1, 1, 1 };
#endif
	}

	void BoxColliderComponent::DrawDebugBillboard()
	{
#ifndef FF_SHIPIT
		if (!Application::Get().GetIsInPlayMode())
		{
			if (myDebugBillboardInfo)
			{
				/*myDebugBillboardInfo->Position = myEntity->GetTransform().GetPosition();
				myDebugBillboardInfo->EntityID = myEntity->GetID();
				Renderer::Submit(*myDebugBillboardInfo);*/
			}
		}
#endif
	}

	void Firefly::BoxColliderComponent::SetWithMinMax(const Utils::Vector3f& aMin, const Utils::Vector3f& aMax)
	{
		//myOffsetSize = { 1,1,1 } means it is 100 units wide, 100 units tall and 100 units deep
		myOffsetSize = (aMax - aMin) * 0.01f;
		myOffset = (aMax + aMin) * 0.5f;
		
	}
}
