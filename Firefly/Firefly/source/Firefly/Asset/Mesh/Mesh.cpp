#include "FFpch.h"
#include "Mesh.h"

namespace Firefly
{
	void Mesh::Init(const std::vector<SubMesh>& someSubMeshes)
	{
		mySubMeshes = someSubMeshes;

		//calculate bounding sphere
		Utils::Vector3f min(FLT_MAX), max(FLT_MIN);

		for (const auto& subMesh : mySubMeshes)
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
		myBoundingSphere.InitWithCenterAndRadius(Utils::Vector3f::Zero(), radius);
		//
		//
		for (auto& sub : mySubMeshes)
		{
			sub.GetBound() = myBoundingSphere;
		}
	}
}