#pragma once
#include <Firefly/Asset/Mesh/AnimatedMesh.h>
#include <Firefly/Asset/Mesh/Mesh.h>
#include <Firefly/Core/Core.h>
#include <Utils/Math/Matrix4x4.hpp>

struct RandomMeshPointInfo
{
	Utils::Vector3f Position;
	Utils::Vector3f Normal;
};

class MeshEmissionController
{
public:
	MeshEmissionController() = default;

	void SetNewMesh(const std::filesystem::path& aPath, bool aLoadNow = false);
	void SetNewAnimatedMesh(const std::filesystem::path& aPath, bool aLoadNow = false);
	void UpdateAnimatedBoneTransforms(const Utils::Matrix4f* someBoneTransforms, int aCount = 128);

	RandomMeshPointInfo GetRandomPointOnMesh();
	RandomMeshPointInfo GetRandomVertex() const;

	bool IsInitialized() const;
	
	const Ref<Firefly::Mesh>& GetMesh() const { return myMesh; }

private:
	inline size_t GetRandomIndex() const;
	size_t GetIndexRecursiveBinary(size_t aLow, size_t aHigh, float aValue) const;

	bool IsLoaded() const;
	void CacheTris();

	Ref<Firefly::Mesh> myMesh;
	Ref<Firefly::AnimatedMesh> myAnimatedMesh;

	struct InternalVertex
	{
		Utils::Vector3f Position;
		Utils::Vector3f Normal;
		unsigned int BoneIDs[4] = { 0, 0, 0, 0 };
		float BoneWeights[4] = { 0, 0, 0, 0 };
	};
	std::vector<InternalVertex> myCachedVertices;
	std::vector<uint32_t> myCachedIndices;

	std::vector<float> myTriAreas;
	std::vector<float> myCumulativeTriAreas;

	std::vector<Utils::Matrix4f> myBoneTransforms;
	std::vector<Utils::Vector3f> mySkinnedPositions;

	float myTriSum = 0.0f;
	int myTriCount = 0;
	bool myIsMeshEmitter = true; //As opposed to animated mesh
};