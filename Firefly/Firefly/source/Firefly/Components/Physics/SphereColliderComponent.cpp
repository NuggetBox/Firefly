#include "FFpch.h"
#include "SphereColliderComponent.h"

#include <Firefly/Physics/PhysicsScene.h>

#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Rendering/Renderer.h"
namespace Firefly
{
	REGISTER_COMPONENT(SphereColliderComponent);

	SphereColliderComponent::SphereColliderComponent() : Component("SphereColliderComponent")
	{
		myMaterial = nullptr;
		myShape = nullptr;
		EditorVariable("Offset", ParameterType::Vec3, &myOffset);
		EditorVariable("Radius", ParameterType::Float, &myRadius);
		EditorVariable("Trigger", ParameterType::Bool, &myTrigger);
		EditorHeader("physics material");
		EditorVariable("Static friction", ParameterType::Float, &myStaticFriction);
		EditorVariable("Dynamic friction", ParameterType::Float, &myDynamicFriction);
		EditorVariable("Bounciness", ParameterType::Float, &myBounciness);
		EditorEndHeader();
	}

	SphereColliderComponent::~SphereColliderComponent()
	{
		CleanUp();
	}

	void SphereColliderComponent::Initialize()
	{
		LoadDebugBillboard();
	}

	void SphereColliderComponent::EarlyInitialize()
	{
#ifndef FF_SHIPIT 
		if (Application::Get().GetIsInPlayMode())
		{
#endif
			auto physics = PhysicsImplementation::GetPhysics();
			myMaterial = physics->createMaterial(myStaticFriction, myDynamicFriction,myBounciness);
			myGeometry = physx::PxSphereGeometry(myRadius);
			myShape = physics->createShape(myGeometry, &myMaterial, 1, physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
			myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !myTrigger);
			myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, myTrigger);
			physx::PxTransform pxTrans{};
			pxTrans.p = Physics::FFToPhysXVec3(myOffset);
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
	
	void SphereColliderComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(SphereColliderComponent::OnUpdateEvent));

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& e)
			{
				DrawDebugBillboard();
				return false;
			});
	}

	bool SphereColliderComponent::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
	{
		Renderer::SubmitDebugSphere(myEntity->GetTransform().GetPosition() + myOffset, myRadius);
		return false;
	}

	void SphereColliderComponent::SetRadius(const float& aRadius)
	{
		myRadius = aRadius;
		myGeometry.radius = myRadius;
		myShape->setGeometry(myGeometry);
	}

	void SphereColliderComponent::CleanUp()
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
		//PHYSX_SAFE_DESTROY(myMaterial);

		scenePtr->unlockWrite();
		Application::Get().UnlockPhysXSimulationMutex();
	}

	void SphereColliderComponent::LoadDebugBillboard()
	{
#ifndef FF_SHIPIT
		myDebugBillboard = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_spherecollider.dds", true);
		myDebugBillboardInfo = CreateRef<BillboardInfo>();
		myDebugBillboardInfo->Texture = myDebugBillboard;
		myDebugBillboardInfo->Color = { 1, 1, 1, 1 };
#endif
	}

	void SphereColliderComponent::DrawDebugBillboard()
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
}
