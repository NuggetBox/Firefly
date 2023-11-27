#pragma once
#include "Firefly/Rendering/Mesh/SubMesh.h"
#include "Firefly/Asset/Asset.h"
#include "Utils/Math/Sphere.hpp"

namespace TGA
{
	struct FBXModel;
}

namespace Firefly
{
	class Mesh :public Asset
	{
	public:
		Mesh() = default;
		~Mesh() = default;
		static AssetType GetStaticType() { return AssetType::Mesh; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		void Init(const std::vector<SubMesh>& someSubMeshes);
		inline std::vector<SubMesh>& GetSubMeshes() { return mySubMeshes; }

		const Utils::Sphere<float>& GetBoundingSphere() const { return myBoundingSphere; }

	private:
		std::vector<SubMesh> mySubMeshes;
		Utils::Sphere<float> myBoundingSphere;
	};
}
