#pragma once
#include <filesystem>
namespace Firefly
{
	enum class AssetType
	{
		Mesh,
		AnimatedMesh,
		Animation,
		Animator,
		Texture,
		Material,
		EmitterTemplate,
		Prefab,
		Font,
		BlendSpace,
		AvatarMask,
		VisualScript,
		VoiceLineData
	};

	class Asset
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;
		virtual AssetType GetAssetType() const = 0;

		//TODO: AssetType: Add to string convertion when adding asset types
		static std::string AssetTypeToString(AssetType anAssetType);

		inline const std::filesystem::path& GetPath() const { return myPath; }
		inline void SetPath(const std::filesystem::path& aPath) { myPath = aPath; }

		virtual bool IsLoaded() const { return myIsLoaded; }

		//for use in ResourceCache::ImportAsset()
		void SetLoaded() { myIsLoaded = true; }
		void SetNotLoaded() { myIsLoaded = false; }


	protected:
		std::filesystem::path myPath;
		bool myIsLoaded = false;
	};

	inline std::string Asset::AssetTypeToString(AssetType anAssetType)
	{
		switch (anAssetType)
		{
		case AssetType::Mesh: return "Mesh";
		case AssetType::AnimatedMesh: return "Animated Mesh";
		case AssetType::Animation: return "Animation";
		case AssetType::Texture: return "Texture";
		case AssetType::Material: return "Material";
		case AssetType::EmitterTemplate: return "Particle Emitter";
		case AssetType::Prefab: return "Prefab";
		case AssetType::Font: return "Font";
		case AssetType::Animator: return "Animator";
		case AssetType::BlendSpace: return "Blend Space";
		case AssetType::VisualScript: return "Visual Script";

		default: return "Unknown Asset Type";
		}
	}
}
