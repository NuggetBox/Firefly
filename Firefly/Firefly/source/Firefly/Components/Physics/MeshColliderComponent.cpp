#include "FFpch.h"
#include "MeshColliderComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Rendering/Renderer.h"
#include <Firefly/Event/EntityEvents.h>
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Physics/PhysicsScene.h"

namespace Firefly
{
	REGISTER_COMPONENT(MeshColliderComponent);
	MeshColliderComponent::MeshColliderComponent() : Component("MeshColliderComponent")
	{
		EditorVariable("Mesh Path", ParameterType::File, &myMeshPath, ".mesh");
		EditorVariable("Convex", ParameterType::Bool, &myIsConvex);
		EditorHeader("physics material");
		EditorVariable("Static friction", ParameterType::Float, &myStaticFriction);
		EditorVariable("Dynamic friction", ParameterType::Float, &myDynamicFriction);
		EditorVariable("Bounciness", ParameterType::Float, &myBounciness);
		EditorVariable("Weld Tolerance", ParameterType::Float, &myWeldTolerance);
		EditorEndHeader();
	}

	MeshColliderComponent::~MeshColliderComponent()
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

		for (auto& geo : myMeshGeometry)
		{
			FF_SAFE_DESTROY(geo);
		}
		PHYSX_SAFE_DESTROY(myMaterial);

		scenePtr->unlockWrite();
		Application::Get().UnlockPhysXSimulationMutex();
	}

	void MeshColliderComponent::Initialize()
	{
		LoadDebugBillboard();
	}

	void MeshColliderComponent::EarlyInitialize()
	{
#ifndef FF_SHIPIT 
		if (Application::Get().GetIsInPlayMode())
		{
#endif
			if (!myMeshPath.empty())
			{
				myMesh = ResourceCache::GetAsset<Mesh>(myMeshPath, true);
			}
			auto physics = PhysicsImplementation::GetPhysics();

			myMaterial = physics->createMaterial(myStaticFriction, myDynamicFriction, myBounciness);
			for (size_t i = 0; i < myMesh->GetSubMeshes().size(); ++i)
			{
				auto& submesh = myMesh->GetSubMeshes()[i];

				myIsConvex = true; //All mesh colliders should be convex, concave only introduces problems with character controller

				if (myIsConvex)
				{
					auto cooked = PhysicsImplementation::GetBakery().CreateConvexMeshCollider(&submesh);
					physx::PxConvexMeshGeometry* gem = new physx::PxConvexMeshGeometry();
					gem->convexMesh = cooked;
					gem->scale = {};
					gem->scale.scale = { myEntity->GetTransform().GetScale().x,myEntity->GetTransform().GetScale().y ,myEntity->GetTransform().GetScale().z };
					myMeshGeometry.push_back(gem);

				}
				else
				{
					auto cooked = PhysicsImplementation::GetBakery().CreateConcaveMeshCollider(&submesh , std::max(myWeldTolerance, 0.001f));
					physx::PxTriangleMeshGeometry* gem = new physx::PxTriangleMeshGeometry();
					gem->triangleMesh = cooked;
					gem->scale = {};
					gem->scale.scale = { myEntity->GetTransform().GetScale().x,myEntity->GetTransform().GetScale().y ,myEntity->GetTransform().GetScale().z };
					myMeshGeometry.push_back(gem);
				}
				auto& shape = myShapes.emplace_back();
				shape = physics->createShape(*myMeshGeometry[i], &myMaterial, 1);

			}
#ifndef FF_SHIPIT 
		}
		else
		{
		}
#endif
	}

	void MeshColliderComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				myIsConvex = true;
				if (e.GetParamName() == "Mesh Path")
				{
					if (!myMeshPath.empty())
					{
						myMesh = ResourceCache::GetAsset<Mesh>(myMeshPath, true);
					}
				}

				return false;
			});

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& e)
			{
				DrawDebugBillboard();
				return false;
			});
	}

	void MeshColliderComponent::SetScale(const Utils::Vec3& aScale)
	{
		for (auto& shape : myShapes)
		{
			if (shape)
			{
				if (myIsConvex)
				{
					physx::PxConvexMeshGeometry gem{};
					shape->getConvexMeshGeometry(gem);
					gem.scale.scale = *(physx::PxVec3*)&aScale;
				}
				else
				{
					physx::PxTriangleMeshGeometry gem{};
					shape->getTriangleMeshGeometry(gem);
					gem.scale.scale = *(physx::PxVec3*)&aScale;
				}
			}
		}
	}

	void MeshColliderComponent::LoadDebugBillboard()
	{
#ifndef FF_SHIPIT
		myDebugBillboard = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_meshcollider.dds", true);
		myDebugBillboardInfo = CreateRef<BillboardInfo>();
		myDebugBillboardInfo->Texture = myDebugBillboard;
		myDebugBillboardInfo->Color = { 1, 1, 1, 1 };
#endif
	}

	void MeshColliderComponent::DrawDebugBillboard()
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
