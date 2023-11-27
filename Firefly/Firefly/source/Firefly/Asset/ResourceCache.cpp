#include "FFpch.h"
#include "ResourceCache.h"
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Animations/AvatarMask.h"
#include "nlohmann/json.hpp"
#include "Animation.h"

namespace Firefly
{
	std::unordered_map<std::filesystem::path, std::shared_ptr<Asset>> ResourceCache::myAssets;
	std::unique_ptr<FBXImporter> ResourceCache::myFBXImporter = CreateScope<FBXImporter>();
	std::unique_ptr<TextureImporter> ResourceCache::myTextureImporter = nullptr;
	std::unique_ptr<MaterialImporter> ResourceCache::myMaterialImporter = nullptr;

	void ResourceCache::Initialize()
	{
		myRunning = true;

		myAssets.reserve(1200);

		SaveAsset(CreateUnitCube());
		SaveAsset(CreateLDCube());
		SaveAsset(CreateUnitPyramid());
		SaveAsset(CreateUnitPyramidBottomPivot());
		SaveAsset(CreateUnitQuad());
		SaveAsset(CreateTriangle());
		SaveAsset(CreateCylinder());
		SaveAsset(CreateDefaultMaterial());
		SaveAsset(CreateDefaultFont());
		IndexAllAssets();

		for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++)
		{
			myAssetImporterThreads.push_back(std::async(std::launch::async, &ResourceCache::ImportAssetsThread));
		}
	}

	void ResourceCache::Shutdown()
	{
		myRunning = false;
		for (auto& thread : myAssetImporterThreads)
		{
			thread.wait();
		}
	}

	void ResourceCache::SaveAsset(Ref<Asset> aAsset)
	{
		std::scoped_lock mapLock(myAssetMapMutex);
		myAssets[aAsset->GetPath().string()] = aAsset;
	}

	void ResourceCache::UnloadAsset(const std::filesystem::path& aPath)
	{
		if (!IsAssetInCache(aPath))
		{
			LOGWARNING("Tried to unload an asset that wasn't loaded {}", aPath.string());
			return;
		}

		std::scoped_lock mapLock(myAssetMapMutex);
		myAssets[aPath]->SetNotLoaded();
		myAssets.erase(aPath);
	}

	void ResourceCache::ReloadAsset(const std::filesystem::path& aPath)
	{
		std::scoped_lock mapLock(myAssetMapMutex, myImportingMutex);
		myAssets[aPath]->SetNotLoaded();
		myAssetsToImport.push(myAssets[aPath]);
	}

	bool ResourceCache::IsAssetInCache(const std::filesystem::path& aPath)
	{
		std::scoped_lock mapLock(myAssetMapMutex);
		return myAssets.contains(aPath);
	}

	std::filesystem::path ResourceCache::GetAssetPath(uint64_t aId)
	{
		if (myAssetsById.contains(aId))
		{
			return myAssetsById[aId];
		}
		return std::filesystem::path();
	}



	void ResourceCache::IndexAllAssets()
	{
		//loop through all assets in assets folder
		for (auto& p : std::filesystem::recursive_directory_iterator("Assets"))
		{
			auto path = std::filesystem::relative(p.path(), std::filesystem::current_path());
			if (p.path().extension() == ".prefab")
			{
				auto id = myPrefabImporter->ImportID(p.path());
				myAssetsById.insert({ id, path });
			}
		}

		//if asset is not in cache, load it
		//if asset is in cache, check if it has been modified
		//if asset has been modified, reload it

	}

	void ResourceCache::CompileBinaryFBX(const std::filesystem::path& aPath)
	{

	}

	void ResourceCache::CompileFBXToAnimation(const std::filesystem::path& aPath, const std::filesystem::path& aToPath, const std::filesystem::path& aSkeletonPath)
	{
		auto anim = myFBXImporter->ImportAnimation(aPath, aSkeletonPath);

		while (!anim->GetAnimatedMesh()->IsLoaded())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (anim->GetFrame(0, true).LocalTransforms.size() != anim->GetAnimatedMesh()->GetSkeleton().Bones.size())
		{
			LOGERROR("Could not Compile Animation \"{}\" to a binary file. It has a different amount of bones than the skeleton \"{}\"", aPath.string(), aSkeletonPath.string());
			return;
		}

		if (anim)
		{
			myFBXImporter->ExportAnimationToBinary(anim, aToPath);
		}
		else
		{
			LOGERROR("Could not Import animation at path \"{}\"", aPath.string());
		}
	}

	void ResourceCache::CompileFBXToMesh(const std::filesystem::path& aPath, const std::filesystem::path& aToPath)
	{
		auto mesh = myFBXImporter->ImportMesh(aPath);

		myFBXImporter->ExportMeshToBinary(mesh, aToPath);
	}

	void ResourceCache::CompileFBXToSkeleton(const std::filesystem::path& aPath, const std::filesystem::path& aToPath)
	{
		std::shared_ptr<AnimatedMesh> animatedMesh = myFBXImporter->ImportAnimatedMesh(aPath);

		myFBXImporter->ExportAnimatedMeshToBinary(animatedMesh, aToPath);
	}

	void ResourceCache::ImportAsset(Ref<Asset> aAsset)
	{
		bool success = false;
		try
		{
			switch (aAsset->GetAssetType())
			{
				case AssetType::Texture:
					success = myTextureImporter->ImportTexture(std::reinterpret_pointer_cast<Texture2D>(aAsset));
					break;
				case AssetType::Material:
					success = myMaterialImporter->ImportMaterial(std::reinterpret_pointer_cast<MaterialAsset>(aAsset));
					break;
				case AssetType::EmitterTemplate:
					success = myParticleEmitterImporter->ImportEmitterTemplate(std::reinterpret_pointer_cast<ParticleEmitterTemplate>(aAsset));
					break;
				case AssetType::Prefab:
					success = myPrefabImporter->ImportPrefab(std::reinterpret_pointer_cast<Prefab>(aAsset));
					break;
				case AssetType::Font:
					success = myFontImporter->ImportFont(std::reinterpret_pointer_cast<Font>(aAsset));
					break;
				case AssetType::Animator:
					success = myAnimatorImporter->ImportAnimator(std::reinterpret_pointer_cast<Animator>(aAsset));
					break;
				case AssetType::BlendSpace:
					success = myBlendSpaceImporter->ImportBlendSpace(std::reinterpret_pointer_cast<BlendSpace>(aAsset));
					break;
				case AssetType::AnimatedMesh:
					success = myFBXImporter->ImportAnimatedMeshBinary(std::reinterpret_pointer_cast<AnimatedMesh>(aAsset));
					break;
				case AssetType::Mesh:
					success = myFBXImporter->ImportMeshBinary(std::reinterpret_pointer_cast<Mesh>(aAsset));
					break;
				case AssetType::Animation:
					success = myFBXImporter->ImportAnimationBinary(std::reinterpret_pointer_cast<Animation>(aAsset));
					break;
				case AssetType::AvatarMask:
					success = LoadAvatarMask(std::reinterpret_pointer_cast<AvatarMask>(aAsset));
					break;
				case AssetType::VisualScript:
					success = myVisualScriptImporter->ImportVisualScript(std::reinterpret_pointer_cast<VisualScriptAsset>(aAsset));
					break;
				case AssetType::VoiceLineData: 
					success = myVoiceLineDataImporter->ImportVoiceLineData(std::reinterpret_pointer_cast<VoiceLineData>(aAsset));
					break;
			}
		}
		catch (...)
		{
			success = false;
			LOGERROR("CRASH WHEN IMPORTING ASSET:\"{}\"", aAsset->GetPath().string());
		}

		if (success)
		{
			aAsset->SetLoaded();
		}
		else
		{
			LOGERROR("Failed to import asset: \"{}\"", aAsset->GetPath().string());
		}
	}

	void ResourceCache::ImportAssetsThread()
	{
		while (myRunning)
		{

			Ref<Firefly::Asset> asset = nullptr;
			myImportingMutex.lock();
			if (myAssetsToImport.size() > 0)
			{
				asset = myAssetsToImport.front();
				myAssetsToImport.pop();
				myImportingMutex.unlock();
				ImportAsset(asset);
			}
			else
			{
				myImportingMutex.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}


	std::shared_ptr<Mesh> ResourceCache::CreateUnitCube()
	{
		std::vector<Vertex> cubeVertices =
		{
			//Front face
			{ -50.0f, 50.0f, -50.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, 50.0f, -50.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, -50.0f, -50.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ -50.0f, -50.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			//Right face
			{ 50.0f, 50.0f, -50.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, 50.0f, 50.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, -50.0f, 50.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, -50.0f, -50.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			//Back face
			{ 50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ -50.0f, 50.0f, 50.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ -50.0f, -50.0f, 50.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 50.0f, -50.0f, 50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			//Left face
			{ -50.0f, 50.0f, 50.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ -50.0f, 50.0f, -50.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ -50.0f, -50.0f, -50.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ -50.0f, -50.0f, 50.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			//Top face
			{ -50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 50.0f, 50.0f, 50.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 50.0f, 50.0f, -50.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ -50.0f, 50.0f, -50.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			//Bottom face
			{ -50.0f, -50.0f, -50.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 50.0f, -50.0f, -50.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 50.0f, -50.0f, 50.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ -50.0f, -50.0f, 50.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
		};

		std::vector<UINT> cubeIndices =
		{
			//Front face
			0, 1, 3,
			3, 1, 2,
			//Right face
			4, 5, 7,
			7, 5, 6,
			//Back face
			8, 9, 11,
			11, 9, 10,
			//Left face
			12, 13, 15,
			15, 13, 14,
			//Top face
			16, 17, 19,
			19, 17, 18,
			//Bottom face
			20, 21, 23,
			23, 21, 22
		};

		//Generate sub mesh
		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(cubeVertices, cubeIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("Cube");
		mesh->SetLoaded();
		return mesh;
	}

	Ref<Mesh> ResourceCache::CreateLDCube()
	{
		std::vector<Vertex> ldVertices =
		{
			//Front face
			{ 0.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			//Right face
			{ 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 100.0f, 100.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 100.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f },
			//Back face
			{ 100.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 100.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, 100.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			//Left face
			{ 0.0f, 100.0f, 100.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 100.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			//Top face
			{ 0.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 100.0f, 100.0f, 100.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 100.0f, 100.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			//Bottom face
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 100.0f, 0.0f, 100.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
		};

		std::vector<UINT> ldIndices =
		{
			//Front face
			0, 1, 3,
			3, 1, 2,
			//Right face
			4, 5, 7,
			7, 5, 6,
			//Back face
			8, 9, 11,
			11, 9, 10,
			//Left face
			12, 13, 15,
			15, 13, 14,
			//Top face
			16, 17, 19,
			19, 17, 18,
			//Bottom face
			20, 21, 23,
			23, 21, 22
		};

		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(ldVertices, ldIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("LDCube");
		mesh->SetLoaded();
		return mesh;
	}

	std::shared_ptr<Mesh> ResourceCache::CreateUnitPyramid()
	{
		// X and Y component of a normalized 2d vector with angle 30degrees to X-axis
		constexpr float xN = 0.894427180f;
		constexpr float yN = 0.447213590f;

		std::vector<Vertex> pyramidVertices =
		{
			//Front face
			{0.0f, 50, 0.0f, 0.5f, 0.5f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			{-50.0f,  -50.0f, -50.0f, 0.0f, 1.0f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			{50.0f,  -50.0f, -50.0f, 1.0f, 1.0f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			//Right face
			{0.0f, 50.0f, 0.0f, 0.5f, 0.5f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			{50.0f,  -50.0f, -50.0f, 0.0f, 1.0f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			{50.0f,  -50.0f, 50.0f, 1.0f, 1.0f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			//Back face
			{0.0f, 50.0f, 0.0f, 0.5f, 0.5f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			{50.0f,  -50.0f, 50.0f, 0.0f, 1.0f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			{-50.0f,  -50.0f, 50.0f, 1.0f, 1.0f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			//Left face
			{0.0f, 50.0f, 0.0f, 0.5f, 0.5f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			{-50.0f,  -50.0f, 50.0f, 0.0f, 1.0f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			{-50.0f,  -50.0f, -50.0f, 1.0f, 1.0f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			//Bottom face
			{-50.0f,  -50.0f, -50.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{50.0f,  -50.0f, -50.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{50.0f,  -50.0f, 50.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{-50.0f,  -50.0f, 50.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}
		};

		std::vector<unsigned int> pyramidIndices =
		{
			0, 2, 1,
			3, 5, 4,
			6, 8, 7,
			9, 11, 10,
			12, 13, 14,
			12, 14, 15
		};

		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(pyramidVertices, pyramidIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("Pyramid");
		mesh->SetLoaded();
		return mesh;
	}

	std::shared_ptr<Mesh> ResourceCache::CreateUnitPyramidBottomPivot()
	{
		// X and Y component of a normalized 2d vector with angle 30degrees to X-axis
		constexpr float xN = 0.894427180f;
		constexpr float yN = 0.447213590f;

		std::vector<Vertex> pyramidVertices =
		{
			//Front face
			{0.0f, 100.f, 0.0f, 0.5f, 0.5f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			{-50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			{50.0f, 0.0f, -50.0f, 1.0f, 1.0f, 0.0f, yN, -xN, 1.0f, 0.0f, 0.0f, 0.0f, -xN, -yN},
			//Right face
			{0.0f, 100.f, 0.0f, 0.5f, 0.5f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			{50.0f, 0.0f, -50.0f, 0.0f, 1.0f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			{50.0f, 0.0f, 50.0f, 1.0f, 1.0f, xN, yN, 0.0f, 0.0f, 0.0f, 1.0f, yN, -xN, 0.0f},
			//Back face
			{0.0f, 100.f, 0.0f, 0.5f, 0.5f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			{50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			{-50.0f, 0.0f, 50.0f, 1.0f, 1.0f, 0.0f, yN, xN, -1.0f, 0.0f, 0.0f, 0.0f, -xN, yN},
			//Left face
			{0.0f, 100.f, 0.0f, 0.5f, 0.5f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			{-50.0f, 0.0f, 50.0f, 0.0f, 1.0f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			{-50.0f, 0.0f, -50.0f, 1.0f, 1.0f, -xN, yN, 0.0f, 0.0f, 0.0f, -1.0f, -yN, -xN, 0.0f},
			//Bottom face
			{-50.0f, 0.0f, -50.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{50.0f, 0.0f, -50.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{50.0f, 0.0f, 50.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{-50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}
		};

		std::vector<unsigned int> pyramidIndices =
		{
			0, 2, 1,
			3, 5, 4,
			6, 8, 7,
			9, 11, 10,
			12, 13, 14,
			12, 14, 15
		};

		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(pyramidVertices, pyramidIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("PyramidBottomPivot");
		mesh->SetLoaded();
		return mesh;
	}

	std::shared_ptr<Mesh> ResourceCache::CreateUnitQuad()
	{
		std::vector<Vertex> quadVertices =
		{
			//Top face
			{-50.0f, 0.0f, 50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{50.0f, 0.0f, 50.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{-50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{50.0f, 0.0f, -50.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f }
		};

		std::vector<unsigned int> quadIndices =
		{
			0, 1, 2,
			1, 3, 2
		};

		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(quadVertices, quadIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("Plane");
		mesh->SetLoaded();
		return mesh;
	}

	Ref<Mesh> ResourceCache::CreateTriangle()
	{
		//X and Y component of normalized 45 degree vector
		constexpr float N = 0.7071067f;

		std::vector<Vertex> triangleVertices =
		{
			//Front face
			{ 0.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f },
			//Right face
			{ 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, N, 0.0f, N, -N, 0.0f, N, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 100.0f, 100.0f, 1.0f, 0.0f, N, 0.0f, N, -N, 0.0f, N, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, 100.0f, 1.0f, 1.0f, N, 0.0f, N, -N, 0.0f, N, 0.0f, -1.0f, 0.0f },
			{ 100.0f, 0.0f, 0.0f, 0.0f, 1.0f, N, 0.0f, N, -N, 0.0f, N, 0.0f, -1.0f, 0.0f },
			//Left face
			{ 0.0f, 100.0f, 100.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 100.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
			//Top face
			{ 0.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 100.0f, 100.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			//Bottom face
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
		};

		std::vector<UINT> triangleIndices =
		{
			//Front face
			0, 1, 2,
			0, 2, 3,
			//Right face
			4, 5, 6,
			4, 6, 7,
			//Left face
			8, 9, 10,
			8, 10, 11,
			//Top face
			12, 13, 14,
			//Bottom face
			15, 16, 17
		};

		//Generate sub mesh
		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(triangleVertices, triangleIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("Triangle");
		mesh->SetLoaded();
		return mesh;
	}

	Ref<Mesh> ResourceCache::CreateCylinder()
	{
		constexpr float height = 100.0f;
		constexpr float radius = 50.0f;
		constexpr int resolution = 25;

		constexpr float anglePerVertex = PI2 / resolution;
		constexpr int vertexCount = 6 * resolution + 2;

		std::vector<Vertex> cylinderVertices;
		cylinderVertices.reserve(vertexCount);

		//Middle Vertex of Top Face, INDEX = 0
		cylinderVertices.emplace_back(0.0f, height, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);

		//Add top outer vertices, count = res, INDEX = 1 -> resolution
		//First vertex is at -z towards camera
		//Added counterclockwise seen from above
		for (int i = 0; i < resolution; ++i)
		{
			float angle = anglePerVertex * i;
			float relativeAngle = angle - PI / 2.0f;
			float x = cos(relativeAngle);
			float z = sin(relativeAngle);
			float u = (x + 1.0f) / 2.0f;
			float v = (z * -1.0f + 1.0f) / 2.0f;

			cylinderVertices.emplace_back(x * radius, height, z * radius, u, v, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		}

		//Middle Vertex of Bottom Face, INDEX = resolution + 1
		cylinderVertices.emplace_back(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		//Add bottom outer vertices, count = res, INDEX = resolution + 2 -> 2 x resolution + 1
		//First vertex is at -z towards camera
		//Added counterclockwise seen from above
		for (int i = 0; i < resolution; ++i)
		{
			float angle = anglePerVertex * i;
			float relativeAngle = angle - PI / 2.0f;
			float x = cos(relativeAngle);
			float z = sin(relativeAngle);
			float u = (x + 1.0f) / 2.0f;
			float v = (z + 1.0f) / 2.0f;

			cylinderVertices.emplace_back(x * radius, 0.0f, z * radius, u, v, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		//Add middle part vertices, count = 4 x res, INDEX = 2 x resolution + 2: i < resolution, (i * 4) + 0 -> 3
		//Create one box for every loop
		for (int i = 0; i < resolution; ++i)
		{
			float angle1 = anglePerVertex * i;
			float relativeAngle1 = angle1 - PI / 2.0f;
			float x1 = cos(relativeAngle1);
			float z1 = sin(relativeAngle1);

			float angle2 = anglePerVertex * (i + 1);
			float relativeAngle2 = angle2 - PI / 2.0f;
			float x2 = cos(relativeAngle2);
			float z2 = sin(relativeAngle2);

			float u1 = (static_cast<float>(i) / static_cast<float>(resolution));
			float u2 = (static_cast<float>(i + 1) / static_cast<float>(resolution));

			float normalAngle = (relativeAngle1 + relativeAngle2) / 2.0f;
			Utils::Vector2f normal(cos(normalAngle), sin(normalAngle));
			normal.Normalize();
			Utils::Vector2f tangent(cos(normalAngle + PI / 2.0f), sin(normalAngle + PI / 2.0f));
			tangent.Normalize();

			cylinderVertices.emplace_back(x1 * radius, height, z1 * radius, u1, 0.0f, normal.x, 0.0f, normal.y, tangent.x, 0.0f, tangent.y, 0.0f, -1.0f, 0.0f);
			cylinderVertices.emplace_back(x2 * radius, height, z2 * radius, u2, 0.0f, normal.x, 0.0f, normal.y, tangent.x, 0.0f, tangent.y, 0.0f, -1.0f, 0.0f);
			cylinderVertices.emplace_back(x2 * radius, 0.0f, z2 * radius, u2, 1.0f, normal.x, 0.0f, normal.y, tangent.x, 0.0f, tangent.y, 0.0f, -1.0f, 0.0f);
			cylinderVertices.emplace_back(x1 * radius, 0.0f, z1 * radius, u1, 1.0f, normal.x, 0.0f, normal.y, tangent.x, 0.0f, tangent.y, 0.0f, -1.0f, 0.0f);
		}

		std::vector<UINT> cylinderIndices;
		cylinderIndices.reserve(vertexCount);

		//Top vertices
		for (int i = 0; i < resolution - 1; ++i)
		{
			cylinderIndices.push_back(i + 1);
			cylinderIndices.push_back(0);
			cylinderIndices.push_back(i + 2);
		}

		//Add the last triangle that connects to the first outer vertex
		cylinderIndices.push_back(resolution);
		cylinderIndices.push_back(0);
		cylinderIndices.push_back(1);

		//Bottom vertices, reverse winding order because upside down
		for (int i = resolution + 1; i < 2 * resolution; ++i)
		{
			cylinderIndices.push_back(i + 1);
			cylinderIndices.push_back(i + 2);
			cylinderIndices.push_back(resolution + 1);
		}

		//Add the last triangle that connects to the first outer vertex
		cylinderIndices.push_back(2 * resolution + 1);
		cylinderIndices.push_back(resolution + 2);
		cylinderIndices.push_back(resolution + 1);

		//Middle vertices
		for (int i = 0; i < resolution; ++i)
		{
			int index = 2 * resolution + 2 + i * 4;

			cylinderIndices.push_back(index);
			cylinderIndices.push_back(index + 1);
			cylinderIndices.push_back(index + 2);

			cylinderIndices.push_back(index);
			cylinderIndices.push_back(index + 2);
			cylinderIndices.push_back(index + 3);
		}

		std::vector<SubMesh> subMeshes;
		subMeshes.emplace_back(cylinderVertices, cylinderIndices);

		auto mesh = std::make_shared<Mesh>();
		mesh->Init(subMeshes);

		mesh->SetPath("Cylinder");
		mesh->SetLoaded();
		return mesh;
	}

	Ref<MaterialAsset> ResourceCache::CreateDefaultMaterial()
	{
		Ref<MaterialAsset> defaultMat = myMaterialImporter->ImportMaterial("FireflyEngine/Defaults/Default.mat");
		defaultMat->SetPath("Default");
		defaultMat->SetLoaded();
		return defaultMat;
	}
	Ref<Font> ResourceCache::CreateDefaultFont()
	{
		Ref<Font> fontMat = CreateRef<Font>(L"FireflyEngine/Fonts/Roboto.ttf");
		fontMat->SetPath("DefaultFont");
		fontMat->SetLoaded();
		return fontMat;
	}
	bool ResourceCache::LoadAvatarMask(Ref<AvatarMask> aAvatarMask)
	{

		std::ifstream file(aAvatarMask->GetPath());
		if (!file.is_open())
		{
			LOGERROR("File could not be opened! Path: {}", aAvatarMask->GetPath().string());
			return false;
		}
		nlohmann::json json;
		file >> json;
		file.close();

		aAvatarMask->SetAnimatedMesh(GetAsset<AnimatedMesh>(json["AnimatedMesh"].get<std::string>()));
		for (auto& bone : json["BonesToIgnore"])
		{
			aAvatarMask->AddBoneToIgnore(bone);
		}

		if (json.contains("Influences") && !json["Influences"].is_null())
		{

			for (auto& influence : json["Influences"])
			{
				aAvatarMask->SetInfluence(influence["TargetBone"], influence["Value"]);
			}
		}


		return true;
	}
}