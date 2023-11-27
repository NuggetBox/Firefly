#include "FFpch.h"
#include "MeshComponent.h"

#include <Utils/Math/Sphere.hpp>

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"

#include "Firefly/Core/Log/DebugLogger.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Mesh/Mesh.h"

#include "Firefly/Asset/Material/MaterialAsset.h"

#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"

namespace Firefly
{
	REGISTER_COMPONENT(MeshComponent);

	MeshComponent::MeshComponent()
		: Component("MeshComponent")
	{
		myModelInfo.CastShadows = true;
		myModelInfo.Outline = false;
		myModelInfo.Material = (Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>("Default"));

		myMaterialPaths.resize(0);
		EditorVariable("Mesh Path", ParameterType::File, &myMeshPath, ".mesh");
		EditorListVariable("Material Paths", ParameterType::File, &myMaterialPaths, ".mat");
		EditorListDefaultOpen();
		EditorVariable("Cast Shadows", ParameterType::Bool, &myModelInfo.CastShadows);
		EditorVariable("Should Render", ParameterType::Bool, &myIsRender);
	}

	void MeshComponent::Initialize()
	{
		myModelInfo.EntityID = myEntity->GetID();
		myModelInfo.IsAnimation = false;
		myModelInfo.CreationTime = myEntity->GetCreationTime();
		LoadMesh();
		LoadMaterials();
	}

	void MeshComponent::OnEvent(Firefly::Event& aEvent)
	{ 
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent&)
			{
				if (myMesh)
				{
					if (myMesh->IsLoaded() && myIsRender)
					{
						if (myCreateBoundingOnce)
						{
							CreateBoundingSphere();
							myCreateBoundingOnce = false;
						}

						{
							myModelInfo.BoundingSphere.SetOrigin(myEntity->GetTransform().GetPosition());

							const float xScale = myEntity->GetTransform().GetScale().x;
							const float yScale = myEntity->GetTransform().GetScale().y;
							const float zScale = myEntity->GetTransform().GetScale().z;
							const float scaleIncrease = sqrtf(xScale * xScale + yScale * yScale + zScale * zScale);

							myModelInfo.BoundingSphere.SetRadius(myInitialRadius * scaleIncrease);
						}

						myModelInfo.Transform = myEntity->GetTransform().GetMatrix();

						if (myShouldResizeMaterials)
						{
							myMaterialPaths.resize(myMesh->GetSubMeshes().size());
							myShouldResizeMaterials = false;
						}


						int index = 0;
						for (auto& submesh : myMesh->GetSubMeshes())
						{
							myModelInfo.Mesh = &submesh;
							if (index < myMaterials.size())
							{
								myModelInfo.Material = myMaterials[index];
							}
							Renderer::Submit(myModelInfo);
							index++;
						}
					}

				}
				return false;
			});

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				if (e.GetParamName() == "Mesh Path")
				{
					LoadMesh();
				}
				else if (e.GetParamName() == "Material Paths")
				{
					LoadMaterials();
				}
				return false;
			});
	}

	void MeshComponent::SetMesh(std::string aMeshPath)
	{
		myMeshPath = aMeshPath;
		LoadMesh();
	}

	void MeshComponent::SetShouldOutline(bool aShouldOutline)
	{
		myModelInfo.Outline = aShouldOutline;
	}

	void MeshComponent::LoadAssets()
	{
		LoadMesh();
		LoadMaterials();
	}

	void MeshComponent::SetMaterialPath(std::string_view aPath)
	{
		if (myMaterialPaths.empty())
		{
			return;
		}
		myMaterialPaths[0] = aPath.data();
		LoadMaterials();
	}

	void MeshComponent::CreateBoundingSphere()
	{
		Utils::Vector3f min(FLT_MAX), max(FLT_MIN);

		const auto& subMeshes = myMesh->GetSubMeshes();

		for (const auto& subMesh : subMeshes)
		{
			const auto& vertices = subMesh.GetVerticesPositions();

			for (const auto& vertex : vertices)
			{
				if (vertex.x < min.x) min.x = vertex.x;
				if (vertex.y < min.y) min.y = vertex.y;
				if (vertex.z < min.z) min.z = vertex.z;

				if (vertex.x > max.x) max.x = vertex.x;
				if (vertex.y > max.y) max.y = vertex.y;
				if (vertex.z > max.z) max.z = vertex.z;
			}
		}

		//Sphere creation
		const float minLength = min.Length();
		const float maxLength = max.Length();
		const float radius = maxLength > minLength ? maxLength : minLength;
		myInitialRadius = radius;
		myModelInfo.BoundingSphere.InitWithCenterAndRadius(Utils::Vector3f::Zero(), radius);
	}

	/*void MeshComponent::SetMaterial(const std::string& aPath)
	{
		myMaterialPath = aPath;
		LoadMaterials();
	}*/

	void MeshComponent::LoadMesh()
	{
		if (myMeshPath != "")
		{
			myMesh = ResourceCache::GetAsset<Mesh>(myMeshPath);

			if (myMesh)
			{
				myShouldResizeMaterials = true;
			}
			else
			{
				auto apa = 1;
			}
		}
		else
		{
			myModelInfo.Mesh = nullptr;
		}
	}

	void MeshComponent::LoadMaterials()
	{
		if (myMaterialPaths.empty())
		{
			return;
		}
		else
		{
			myMaterials.resize(myMaterialPaths.size());
			for (int i = 0; i < myMaterialPaths.size(); i++)
			{
				if (myMaterialPaths[i].empty())
				{
					continue;
				}
				myMaterials[i] = ResourceCache::GetAsset<MaterialAsset>(myMaterialPaths[i]);
			}
		}
	}
}
