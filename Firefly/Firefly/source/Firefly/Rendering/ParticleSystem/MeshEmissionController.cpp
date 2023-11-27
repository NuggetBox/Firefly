#include "FFpch.h"
#include "MeshEmissionController.h"

#include <Firefly/Asset/ResourceCache.h>
#include <Utils/Math/Random.hpp>

void MeshEmissionController::SetNewMesh(const std::filesystem::path& aPath, bool aLoadNow)
{
	myIsMeshEmitter = true;
	myMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>(aPath, aLoadNow);
	CacheTris();
}

void MeshEmissionController::SetNewAnimatedMesh(const std::filesystem::path& aPath, bool aLoadNow)
{
	myIsMeshEmitter = false;
	myAnimatedMesh = Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(aPath, aLoadNow);
	CacheTris();
}

void MeshEmissionController::UpdateAnimatedBoneTransforms(const Utils::Matrix4f* someBoneTransforms, int aCount)
{
	FF_PROFILESCOPE("Particles: Update Bone Transforms");

	myIsMeshEmitter = false;

	if (myBoneTransforms.size() < aCount)
	{
		myBoneTransforms.resize(aCount);
	}

	memcpy_s(myBoneTransforms.data(), myBoneTransforms.size() * sizeof(Utils::Matrix4f), someBoneTransforms, aCount * sizeof(Utils::Matrix4f));

	if (mySkinnedPositions.size() < myCachedVertices.size())
	{
		mySkinnedPositions.resize(myCachedVertices.size());
	}

	for (size_t i = 0; i < myCachedVertices.size(); ++i)
	{
		Utils::Matrix4f skinningMatrixBuffer =
			myBoneTransforms[myCachedVertices[i].BoneIDs[0]] * myCachedVertices[i].BoneWeights[0];
		skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[1]] * myCachedVertices[i].BoneWeights[1];
		skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[2]] * myCachedVertices[i].BoneWeights[2];
		skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[3]] * myCachedVertices[i].BoneWeights[3];

		mySkinnedPositions[i] = Utils::Vec4ToVec3(Utils::Vec3ToVec4(myCachedVertices[i].Position) * Utils::Matrix4f::Transpose(skinningMatrixBuffer));
	}

	//Threaded solution, slower with player character
	/*constexpr size_t numThreads = 2;
	const size_t count = myCachedVertices.size();
	std::array<std::thread, numThreads> threads;

	for (size_t n = 0; n < numThreads; ++n)
	{
		threads[n] = std::thread([this, count, n]() 
		{
			for (size_t i = n * (count / numThreads); i < (n + 1) * (count / numThreads); i++)
			{
				Utils::Matrix4f skinningMatrixBuffer = myBoneTransforms[myCachedVertices[i].BoneIDs[0]] * myCachedVertices[i].BoneWeights[0];
				skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[1]] * myCachedVertices[i].BoneWeights[1];
				skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[2]] * myCachedVertices[i].BoneWeights[2];
				skinningMatrixBuffer += myBoneTransforms[myCachedVertices[i].BoneIDs[3]] * myCachedVertices[i].BoneWeights[3];

				mySkinnedPositions[i] = Utils::Vec4ToVec3(Utils::Vec3ToVec4(myCachedVertices[i].Position) * Utils::Matrix4f::Transpose(skinningMatrixBuffer));
			}
		});
	}

	for (auto& thread : threads)
	{
		thread.join();
	}*/
}

RandomMeshPointInfo MeshEmissionController::GetRandomVertex() const
{
	const int randomVertexIndex = Utils::RandomInt(0, static_cast<int>(myCachedVertices.size()));

	RandomMeshPointInfo randomVertex;
	randomVertex.Position = myCachedVertices[randomVertexIndex].Position;
	randomVertex.Normal = myCachedVertices[randomVertexIndex].Normal;
	return randomVertex;
}

bool MeshEmissionController::IsInitialized() const
{
	if (IsLoaded())
	{
		return myTriCount > 0 && !myCachedVertices.empty() && !myCachedIndices.empty() && !myTriAreas.empty();
	}

	return false;
}

RandomMeshPointInfo MeshEmissionController::GetRandomPointOnMesh()
{
	if (!IsInitialized())
	{
		LOGERROR("MeshEmissionController::GetRandomPointOnMesh: Mesh Emission not initialized");
		return {};
	}

	const size_t firstIndex = GetRandomIndex();

	if (firstIndex + 2 >= myCachedIndices.size())
	{
		LOGERROR("MeshEmissionController::GetRandomPointOnMesh: Vertex out of range: {} Cached Indices Count: {}", firstIndex + 2, myCachedIndices.size());
		return {};
	}

	if (!myIsMeshEmitter)
	{
		if (mySkinnedPositions.size() < myCachedVertices.size())
		{
			mySkinnedPositions.resize(myCachedVertices.size());
		}
	}

	const Utils::Vector3 a = myIsMeshEmitter ? myCachedVertices[myCachedIndices[firstIndex]].Position : mySkinnedPositions[myCachedIndices[firstIndex]];
	const Utils::Vector3 b = myIsMeshEmitter ? myCachedVertices[myCachedIndices[firstIndex + 1]].Position : mySkinnedPositions[myCachedIndices[firstIndex + 1]];
	const Utils::Vector3 c = myIsMeshEmitter ? myCachedVertices[myCachedIndices[firstIndex + 2]].Position : mySkinnedPositions[myCachedIndices[firstIndex + 2]];

	float randomAlongAB = Utils::RandomFloat();
	float randomAlongAC = Utils::RandomFloat();

	if (randomAlongAB + randomAlongAC >= 1)
	{
		randomAlongAB = 1 - randomAlongAB;
		randomAlongAC = 1 - randomAlongAC;
	}

	RandomMeshPointInfo randomPointOnMesh;
	randomPointOnMesh.Position = a + randomAlongAB * (b - a) + randomAlongAC * (c - a);
	randomPointOnMesh.Normal = myCachedVertices[myCachedIndices[firstIndex]].Normal;
	return randomPointOnMesh;
}

size_t MeshEmissionController::GetRandomIndex() const
{
	const float randomSizeRange = Utils::RandomFloat() * myTriSum;
	size_t randomTriIndex = 0;

	if (randomSizeRange <= myCumulativeTriAreas[0])
	{
		randomTriIndex = 0;
	}
	else
	{
		//new recursive binary search
		randomTriIndex = GetIndexRecursiveBinary(0, myCumulativeTriAreas.size() - 1, randomSizeRange);

		//Old linear search
		/*for (int i = 0; i < myCumulativeTriAreas.size(); i++)
		{
			if (randomSizeRange <= myCumulativeTriAreas[i])
			{
				randomTriIndex = i;
				break;
			}
		}*/
	}

	if (randomTriIndex == -1)
	{
		LOGERROR("MeshEmissionController::GetRandomPointOnMesh: Something went wrong while generating a random point");
		return {};
	}

	return randomTriIndex * 3;
}

size_t MeshEmissionController::GetIndexRecursiveBinary(size_t aLow, size_t aHigh, float aValue) const
{
	if (aLow + 1 >= aHigh)
	{
		return aHigh;
	}

	const size_t middle = (aLow + aHigh + 1) / 2;

	if (aValue <= myCumulativeTriAreas[middle])
	{
		return GetIndexRecursiveBinary(aLow, middle, aValue);
	}
	else
	{
		return GetIndexRecursiveBinary(middle, aHigh, aValue);
	}
}

bool MeshEmissionController::IsLoaded() const
{
	if (myIsMeshEmitter)
	{
		return myMesh && myMesh->IsLoaded();
	}
	else
	{
		return myAnimatedMesh && myAnimatedMesh->IsLoaded();
	}
}

void MeshEmissionController::CacheTris()
{
	FF_PROFILESCOPE("Cache Tris");

	if (!IsLoaded())
	{
		LOGERROR("MeshEmissionController::CacheTris: Mesh not loaded");
		return;
	}

	myCachedVertices.clear();
	myCachedIndices.clear();

	myCumulativeTriAreas.clear();
	myTriAreas.clear();
	myTriSum = 0;
	myTriCount = 0;

	const auto& submeshes = myIsMeshEmitter ? myMesh->GetSubMeshes() : myAnimatedMesh->GetSubMeshes();

	for (const auto& submesh : submeshes)
	{
		const auto& vertices = submesh.GetVertices();
		auto indices = submesh.GetIndices();

		myTriAreas.resize(myTriAreas.size() + indices.size() / 3);

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			const Utils::Vector3f a = { vertices[indices[i]].Position[0], vertices[indices[i]].Position[1], vertices[indices[i]].Position[2] };
			const Utils::Vector3f b = { vertices[indices[i + 1]].Position[0], vertices[indices[i + 1]].Position[1], vertices[indices[i + 1]].Position[2] };
			const Utils::Vector3f c = { vertices[indices[i + 2]].Position[0], vertices[indices[i + 2]].Position[1], vertices[indices[i + 2]].Position[2] };

			//Offset the submesh-indices so they refer to indices for the whole mesh
			indices[i] += myCachedVertices.size();
			indices[i + 1] += myCachedVertices.size();
			indices[i + 2] += myCachedVertices.size();

			//Calculate the Tri-Area for the 3 vertices
			const float triArea = 0.5f * (b - a).Cross(c - a).Length();
			myTriAreas[myTriCount] = triArea;
			myTriCount++;
		}

		myCachedVertices.reserve(myCachedVertices.size() + vertices.size());
		for (const auto& vertex : vertices)
		{
			InternalVertex insert;
			insert.Position = { vertex.Position[0], vertex.Position[1], vertex.Position[2] };
			insert.Normal = { vertex.Normal[0], vertex.Normal[1], vertex.Normal[2] };
			memcpy(insert.BoneIDs, vertex.BoneIDs, sizeof(unsigned) * 4);
			memcpy(insert.BoneWeights, vertex.BoneWeights, sizeof(float) * 4);
			myCachedVertices.push_back(insert);
		}

		myCachedIndices.insert(myCachedIndices.end(), indices.begin(), indices.end());
	}

	myCumulativeTriAreas.resize(myTriAreas.size());

	for (size_t i = 0; i < myTriAreas.size(); ++i)
	{
		myTriSum += myTriAreas[i];
		myCumulativeTriAreas[i] = myTriSum;
	}
}
