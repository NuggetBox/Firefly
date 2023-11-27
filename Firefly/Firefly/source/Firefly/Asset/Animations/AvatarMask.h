#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Asset.h"
#include <unordered_set>
#include <unordered_map>

class AvatarMaskEditorWindow;
namespace Firefly
{
	class AnimatedMesh;
	class AvatarMask : public Asset
	{
	public:
		virtual ~AvatarMask() = default ;
		__forceinline const std::unordered_set<uint32_t>& GetBonesToIgnore() const { return myBonesToIgnore; }

		void AddBoneToIgnore(uint32_t aBoneID);
		void RemoveBoneToIgnore(uint32_t aBoneID);
		
		static AssetType GetStaticType() { return AssetType::AvatarMask; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		bool IsLoaded() const override;
		
		void SaveTo(const std::filesystem::path& aPath);

		Ref<AnimatedMesh> GetAnimatedMesh() const { return myAnimatedMesh; }
		void SetAnimatedMesh(Ref<AnimatedMesh> aAnimatedMesh) { myAnimatedMesh = aAnimatedMesh; }

		void SetInfluence(uint32_t aBone, float aInfluence);
		float GetInfluence(uint32_t aBone);
		
	private:
		friend class AvatarMaskImporter;
		friend class ::AvatarMaskEditorWindow;
		std::unordered_set<uint32_t> myBonesToIgnore;
		std::unordered_map<uint32_t, float> myInfluences;
		Ref<AnimatedMesh> myAnimatedMesh;

	};

}