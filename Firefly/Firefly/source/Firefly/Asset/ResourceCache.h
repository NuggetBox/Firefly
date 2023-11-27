#pragma once
#include "Firefly/Core/Core.h"

#include "Firefly/Asset/Importers/FBXImporter.h"
#include "Firefly/Asset/Importers/TextureImporter.h"
#include "Firefly/Asset/Importers/MaterialImporter.h"
#include "Firefly/Asset/Importers/ParticleEmitterImporter.h"
#include "Firefly/Asset/Importers/PrefabImporter.h"
#include "Firefly/Asset/Importers/FontImporter.h"
#include "Firefly/Asset/Importers/AnimatorImporter.h"
#include "Firefly/Asset/Importers/BlendSpaceImporter.h"
#include "Firefly/Asset/Importers/VoiceLineDataImporter.h"
#include "Firefly/Asset/Asset.h"

#include "Firefly/Core/Log/DebugLogger.h"

#include <filesystem>
#include <unordered_map>
#include <queue>
#include <future>

#include "Importers/VisualScriptImporter.h"

namespace Firefly
{
	class Asset;
	class Mesh;
	class AvatarMask;
	class AnimatedMesh;
	class ResourceCache
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void SaveAsset(Ref<Asset> aAsset);
		static void UnloadAsset(const std::filesystem::path& aPath);
		static void ReloadAsset(const std::filesystem::path& aPath);
		static bool IsAssetInCache(const std::filesystem::path& aPath);
		template<class T>
		static Ref<T> GetAsset(const std::filesystem::path& aPath, bool aNowFlag = false);
		template<class T>
		static uint64_t GetAssetID(const std::filesystem::path& aPath);
		static std::filesystem::path GetAssetPath(uint64_t aId);
		/// <summary>
		/// ONLY WORKS FOR PREFABS AT THE MOMENT
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="aId"></param>
		/// <returns></returns>
		template<class T>
		static Ref<T> GetAsset(uint64_t aId, bool aNowFlag = false);
		/// <summary>
		/// ONLY WORKS FOR PREFABS AT THE MOMENT
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="aId"></param>
		/// <returns></returns>
		static void IndexAllAssets();

		static FBXImporter* GetFBXImporter() { return myFBXImporter.get(); }

		template<class T>
		static std::vector<Ref<T>> GetAllAssetsOfType();

		static void CompileBinaryFBX(const std::filesystem::path& aPath);
		static void CompileFBXToAnimation(const std::filesystem::path& aFromPath, const std::filesystem::path& aToPath, const std::filesystem::path& aSkeletonPath);
		static void CompileFBXToMesh(const std::filesystem::path& aFromPath, const std::filesystem::path& aToPath);
		static void CompileFBXToSkeleton(const std::filesystem::path& aFromPath, const std::filesystem::path& aToPath);

	private:
		static Ref<Mesh> CreateUnitCube();
		static Ref<Mesh> CreateLDCube();
		static Ref<Mesh> CreateUnitPyramid();
		static Ref<Mesh> CreateUnitPyramidBottomPivot();
		static Ref<Mesh> CreateUnitQuad();
		static Ref<Mesh> CreateTriangle();
		static Ref<Mesh> CreateCylinder();
		static Ref<MaterialAsset> CreateDefaultMaterial();
		static Ref<Font> CreateDefaultFont();

		static bool LoadAvatarMask(Ref<AvatarMask> aAvatarMask);

		static void ImportAssetsThread();

		static void ImportAsset(Ref<Asset> aAsset);

		static std::unordered_map<std::filesystem::path, Ref<Asset>> myAssets;
		inline static std::unordered_map<uint64_t, std::filesystem::path> myAssetsById;

		static Scope<FBXImporter> myFBXImporter;
		static Scope<TextureImporter> myTextureImporter;
		static Scope<MaterialImporter> myMaterialImporter;
		inline static Scope<FontImporter> myFontImporter;
		inline static Scope<ParticleEmitterImporter> myParticleEmitterImporter;
		inline static Scope<PrefabImporter> myPrefabImporter = CreateScope<PrefabImporter>();
		inline static Scope<AnimatorImporter> myAnimatorImporter = CreateScope<AnimatorImporter>();
		inline static Scope<BlendSpaceImporter> myBlendSpaceImporter = CreateScope<BlendSpaceImporter>();
		inline static Scope<VisualScriptImporter> myVisualScriptImporter = CreateScope<VisualScriptImporter>();
		inline static Scope<VoiceLineDataImporter> myVoiceLineDataImporter = CreateScope<VoiceLineDataImporter>();

		inline static std::mutex myAssetMapMutex;
		inline static std::mutex myImportingMutex;
		inline static std::queue<Ref<Asset>> myAssetsToImport;
		inline static std::vector<std::future<void>> myAssetImporterThreads;

		static inline bool myRunning = false;
	};

	template<class T>
	inline std::shared_ptr<T> ResourceCache::GetAsset(const std::filesystem::path& aPath, bool aNowFlag)
	{
		auto it = myAssets.find(aPath);
		if (it != myAssets.end() && it->second->GetAssetType() == T::GetStaticType())
		{
			while (aNowFlag && !it->second->IsLoaded())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			return std::reinterpret_pointer_cast<T>(it->second);
		}

		if (aPath.empty())
		{
			LOGERROR("Failed to get asset of type {}, no path given", Asset::AssetTypeToString(T::GetStaticType()));
			return Ref<T>();
		}

		if (!std::filesystem::exists(aPath))
		{
			LOGERROR("Failed to get asset of type {}, no file exists at path {} ", Asset::AssetTypeToString(T::GetStaticType()), aPath.string().c_str());
			return Ref<T>();
		}
		if (!std::filesystem::is_regular_file(aPath))
		{
			LOGERROR("Failed to get asset with type {}, file at path {} is not a file", Asset::AssetTypeToString(T::GetStaticType()), aPath.string().c_str());
			return Ref<T>();
		}


		auto asset = CreateRef<T>();
		asset->SetPath(aPath);

		myImportingMutex.lock();
		myAssetsToImport.push(asset);
		myImportingMutex.unlock();

		SaveAsset(asset);

		if (aNowFlag)
		{
			while (!asset->IsLoaded())
			{
				//wait here until the asset is loaded
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		std::scoped_lock mapLock(myAssetMapMutex);
		return std::reinterpret_pointer_cast<T>(myAssets[aPath]);
	}

	template<class T>
	inline Ref<T> ResourceCache::GetAsset(uint64_t aId, bool aNowFlag)
	{
		if (myAssetsById.contains(aId))
		{
			return GetAsset<T>(myAssetsById[aId], aNowFlag);
		}
		else
		{
			return Ref<T>();
		}
	}


	template<class T>
	inline std::vector<Ref<T>> ResourceCache::GetAllAssetsOfType()
	{
		std::vector<Ref<T>> assets;
		std::scoped_lock mapLock(myAssetMapMutex);
		for (auto& asset : myAssets)
		{
			if (asset.second->GetAssetType() == T::GetStaticType())
			{
				assets.push_back(std::reinterpret_pointer_cast<T>(asset.second));
			}
		}
		return assets;
	}
}
