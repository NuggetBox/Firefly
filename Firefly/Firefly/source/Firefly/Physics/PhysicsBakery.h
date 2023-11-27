#pragma once
#include "PhysX/PxPhysicsAPI.h"
#include <Firefly/Asset/Mesh/Mesh.h>
namespace Firefly
{
	class PhysicsBakery
	{
	public:
		void Initialize();
		physx::PxTriangleMesh* CreateConcaveMeshCollider(SubMesh* aMesh, float aWeldTolerance = 0.f);
		physx::PxConvexMesh* CreateConvexMeshCollider(SubMesh* aMesh);
		void Shutdown();
	private:
		physx::PxCooking* myCooker;
	};
}