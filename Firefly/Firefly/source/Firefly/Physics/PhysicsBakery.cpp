#include "FFpch.h"
#include "PhysicsBakery.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include <Firefly/Physics/PhysicsUtils.h>
namespace Firefly
{
	physx::PxCookingParams cookerParams{ physx::PxTolerancesScale(100.f, 1000.f) };
	void PhysicsBakery::Initialize()
	{

		cookerParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
		cookerParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
		myCooker = PxCreateCooking(PX_PHYSICS_VERSION, *PhysicsImplementation::myFoundation, cookerParams);
	}
	physx::PxTriangleMesh* PhysicsBakery::CreateConcaveMeshCollider(SubMesh* aMesh, float aWeldTolerance)
	{
		if (aWeldTolerance != cookerParams.meshWeldTolerance)
		{
			cookerParams.meshWeldTolerance = aWeldTolerance;
			myCooker->setParams(cookerParams);
		}

		physx::PxTriangleMeshDesc desc{};
		auto& verties = aMesh->GetVertices();
		auto& indices = aMesh->GetIndices();
		desc.points.count = verties.size();
		desc.points.stride = sizeof(Vertex); 
		desc.points.data = verties.data();
		desc.triangles.count = indices.size() / 3;
		desc.triangles.stride = 3 * sizeof(uint32_t);
		desc.triangles.data = indices.data();

		physx::PxDefaultMemoryOutputStream writeBuffer;
		myCooker->cookTriangleMesh(desc, writeBuffer);
		physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		auto tMesh = PhysicsImplementation::myPhysics->createTriangleMesh(readBuffer);
		return tMesh;
	}
	physx::PxConvexMesh* PhysicsBakery::CreateConvexMeshCollider(SubMesh* aMesh)
	{
		physx::PxConvexMeshDesc desc{};
		auto& verties = aMesh->GetVertices();
		auto& indices = aMesh->GetIndices();
		desc.points.count = verties.size();
		desc.points.stride = sizeof(Vertex);
		desc.points.data = verties.data();
		desc.indices.count = indices.size();
		desc.indices.stride = sizeof(uint32_t);
		desc.indices.data = indices.data();
		desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;


		physx::PxDefaultMemoryOutputStream writeBuffer;
		physx::PxConvexMeshCookingResult::Enum result;
		myCooker->cookConvexMesh(desc, writeBuffer, &result);
		physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		auto tMesh = PhysicsImplementation::myPhysics->createConvexMesh(readBuffer);
		return tMesh;
	}
	void PhysicsBakery::Shutdown()
	{
		PHYSX_SAFE_DESTROY(myCooker);
	}
}