#pragma once
#include <string>

#include "Firefly/Rendering/Mesh/SubMesh.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Asset/Asset.h"

#include "Utils/Math/Matrix4x4.hpp"
#include "Utils/Math/Sphere.hpp"

#include <unordered_map>

namespace TGA
{
	struct FBXModel;
}

namespace Firefly
{
	struct Skeleton
	{
		std::string Name;

		struct Bone
		{
			Utils::Matrix4f BindPoseInverse;
			int Parent;
			std::vector<unsigned int> Children;
			std::string Name;
		};

		std::vector<Bone> Bones;
		std::unordered_map<std::string, size_t> BoneNameToIndex;

	};

	class AnimatedMesh :public Asset
	{
	public:
		AnimatedMesh() = default;
		~AnimatedMesh() = default;
		static AssetType GetStaticType() { return AssetType::AnimatedMesh; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		void Init(std::vector<SubMesh>& someSubMeshes, Skeleton& aSkeleton);
		
		FORCEINLINE std::vector<SubMesh>& GetSubMeshes() { return mySubMeshes; }

		FORCEINLINE Skeleton& GetSkeleton()  { return mySkeleton; }

		Utils::Sphere<float> GetBoundingSphere() const { return myBoundingSphere; }
	private:
		Skeleton mySkeleton;
		std::vector<SubMesh> mySubMeshes;

		Utils::Sphere<float> myBoundingSphere;
	};
}
