#include "FFpch.h"
#include "FoliagePlaneComponent.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Components/Mesh/MeshComponent.h"

namespace Firefly
{
	REGISTER_COMPONENT(FoliagePlaneComponent);

	FoliagePlaneComponent::FoliagePlaneComponent() : Component("FoliagePlaneComponent")
	{
	}

	void FoliagePlaneComponent::Initialize()
	{
		if (myEntity->HasComponent<MeshComponent>())
		{
			myCompAdded = true;
			myMeshComp = myEntity->GetComponent<MeshComponent>();
		}
		else
		{
			myMeshComp = CreateRef<MeshComponent>();
			myMeshComp.lock()->SetMesh("Plane");
		}
	}

	void FoliagePlaneComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
		{
			if (myMeshComp.expired() || !myMeshComp.lock()->GetMesh() || !myMeshComp.lock()->GetMesh()->IsLoaded())
			{
				return false;
			}

			if (!myCompAdded)
			{
				myEntity->AddComponent(myMeshComp.lock());
				myCompAdded = true;
			}

			if (Application::Get().GetIsInPlayMode())
			{
				myMeshComp.lock()->SetIsRender(false);
				return false;
			}

			const auto& vertices = myMeshComp.lock()->GetMesh()->GetSubMeshes().front().GetVerticesPositions();
			const auto& matrix = myEntity->GetTransform().GetMatrix();

			const Utils::Vector3f firstPoint = Utils::Vec4ToVec3(Utils::Vec3ToVec4(vertices[0]) * matrix);
			const Utils::Vector3f secondPoint = Utils::Vec4ToVec3(Utils::Vec3ToVec4(vertices[1]) * matrix);
			const Utils::Vector3f thirdPoint = Utils::Vec4ToVec3(Utils::Vec3ToVec4(vertices[2]) * matrix);

			myBoundingPlane.InitWithThreePoints(firstPoint, secondPoint, thirdPoint);

			//const Vector4f pink(1.0f, 0.8f, 0.8f, 1.0f);

			/*Renderer::SubmitDebugLine(myBoundingPlane.GetFirstPoint(), myBoundingPlane.GetSecondPoint(), pink);
			Renderer::SubmitDebugLine(myBoundingPlane.GetSecondPoint(), myBoundingPlane.GetFourthPoint(), pink);
			Renderer::SubmitDebugLine(myBoundingPlane.GetFourthPoint(), myBoundingPlane.GetThirdPoint(), pink);
			Renderer::SubmitDebugLine(myBoundingPlane.GetThirdPoint(), myBoundingPlane.GetFirstPoint(), pink);*/

			return false;
		});
	}
}
