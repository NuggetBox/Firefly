#include "FFpch.h"
#include "FBXImporter.h"
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Animation.h"
#include "FBXImporter/Importer.h"

namespace Firefly
{
	FBXImporter::FBXImporter()
	{
		TGA::FBX::Importer::InitImporter();
	}
	FBXImporter::~FBXImporter()
	{
		TGA::FBX::Importer::UninitImporter();
	}
	std::shared_ptr<Mesh> FBXImporter::ImportMesh(const std::filesystem::path& aPath)
	{

		TGA::FBX::Model tgaModel;
		{
			myImporterMutex.lock();

			try
			{
				if (!TGA::FBX::Importer::LoadModel(aPath.wstring(), tgaModel, false, true))
				{
					LOGERROR("Failed to load model {}", aPath.string());
				}
			}
			catch (std::runtime_error error)
			{
				LOGERROR("Could not import Static Mesh using fbx file at path: \"{}\". Error is: \"{}\"", aPath.string(), error.what());
				myImporterMutex.unlock();
				return std::shared_ptr<Mesh>();
			}
			myImporterMutex.unlock();
		}

		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

		std::vector<SubMesh> subMeshes;
		ExtractSubMeshes(tgaModel, subMeshes);

		mesh->Init(subMeshes);
		return mesh;
	}
	std::shared_ptr<AnimatedMesh> FBXImporter::ImportAnimatedMesh(const std::filesystem::path& aPath)
	{
		TGA::FBX::Model tgaModel;
		{
			myImporterMutex.lock();
			try
			{
				if (!TGA::FBX::Importer::LoadModel(aPath.wstring(), tgaModel))
				{
					//Boom!
					LOGERROR("Failed to load animated model {}", aPath.string());
				}
			}
			catch (std::runtime_error error)
			{
				LOGERROR("Could not import Skeleton using fbx file at path: \"{}\". Error is: \"{}\"", aPath.string(), error.what());
				myImporterMutex.unlock();
				return std::shared_ptr<AnimatedMesh>();
			}
			myImporterMutex.unlock();
		}

		std::vector<SubMesh> subMeshes;
		ExtractSubMeshes(tgaModel, subMeshes);

		std::shared_ptr<AnimatedMesh> mesh = std::make_shared<AnimatedMesh>();

		Skeleton skeleton;
		skeleton.Name = tgaModel.Skeleton.Name;

		for (auto& tgaBone : tgaModel.Skeleton.Bones)
		{
			Skeleton::Bone bone;

			Utils::Matrix4x4<float> mat;
			mat = tgaBone.BindPoseInverse.Data;
			bone.BindPoseInverse = mat;

			bone.Name = tgaBone.Name;
			bone.Children = tgaBone.Children;
			bone.Parent = tgaBone.ParentIdx;
			skeleton.Bones.push_back(bone);
		}

		skeleton.BoneNameToIndex = tgaModel.Skeleton.BoneNameToIndex;

		mesh->Init(subMeshes, skeleton);

		return mesh;
	}
	std::shared_ptr<Animation> FBXImporter::ImportAnimation(const std::filesystem::path& aPath, const std::filesystem::path& aSkeletonPath)
	{
		TGA::FBX::Animation tgaAnimation;
		{
			myImporterMutex.lock();
			try
			{
				if (!TGA::FBX::Importer::LoadAnimation(aPath.wstring(), tgaAnimation))
				{
					//Boom!
				}
			}
			catch (std::runtime_error error)
			{
				LOGERROR("Could not import animation using fbx file at path: \"{}\". Error is: \"{}\"", aPath.string(), error.what());
				myImporterMutex.unlock();
				return std::shared_ptr<Animation>();
			}
			myImporterMutex.unlock();
		}
		std::shared_ptr<Animation> animation = std::make_shared<Animation>();

		animation->Name = tgaAnimation.Name;
		animation->FrameCount = tgaAnimation.Length;
		animation->FramesPerSecond = tgaAnimation.FramesPerSecond;

		for (auto& tgaFrame : tgaAnimation.Frames)
		{
			Frame frame;
			for (auto& tgaLocTrans : tgaFrame.LocalTransforms)
			{
				Utils::Matrix4x4<float> mat;
				mat = tgaLocTrans.Data;
				Utils::Vector3f pos;
				Utils::Quaternion rot;
				Utils::Vector3f scale;
				//here the mat we get is row major
				Utils::Matrix4f::Decompose(mat, pos, rot, scale);

				auto newMat = Utils::Matrix4f::CreateFromPosRotScale(pos, rot, scale);

				//check if the mat is correct

				for (int x = 1; x <= 4; x++)
				{
					for (int y = 1; y <= 4; y++)
					{
						//chek if equal with 0.0001f
						if (std::abs(mat(x, y) - newMat(x, y)) > 0.0001f)
						{
							LOGERROR("Matrices are not equal");
						}
					}
				}

				frame.LocalTransforms.emplace_back(pos, rot, scale);
			}

			animation->Frames.push_back(frame);
		}

		animation->SetAnimatedMeshPath(aSkeletonPath);
		return animation;
	}

	bool FBXImporter::ImportMeshBinary(Ref<Mesh> aMesh)
	{
		std::ifstream file(aMesh->GetPath(), std::ios::binary);

		uint32_t subMeshCount = 0;

		file.read((char*)&subMeshCount, sizeof(uint32_t));

		std::vector<SubMesh> subMeshes;
		for (int i = 0; i < subMeshCount; i++)
		{
			SubMeshHeader header;
			file.read((char*)&header, sizeof(SubMeshHeader));

			std::vector<Vertex> vertices;
			vertices.resize(header.VertexBufferCount);
			file.read((char*)vertices.data(), header.VertexBufferCount * sizeof(Vertex));

			std::vector<uint32_t> indices;
			indices.resize(header.IndexBufferCount);
			file.read((char*)indices.data(), header.IndexBufferCount * sizeof(uint32_t));

			subMeshes.emplace_back(vertices, indices, aMesh->GetPath().filename().string(), i);
		}
		file.close();

		aMesh->Init(subMeshes);
		return true;
	}

	bool FBXImporter::ImportAnimatedMeshBinary(Ref<AnimatedMesh> aAnimatedMesh)
	{
		std::ifstream file(aAnimatedMesh->GetPath(), std::ios::binary);

		uint32_t subMeshCount = 0;

		file.read((char*)&subMeshCount, sizeof(uint32_t));

		std::vector<SubMesh> subMeshes;
		for (int i = 0; i < subMeshCount; i++)
		{
			SubMeshHeader header;
			file.read((char*)&header, sizeof(SubMeshHeader));

			std::vector<Vertex> vertices;
			vertices.resize(header.VertexBufferCount);
			file.read((char*)vertices.data(), header.VertexBufferCount * sizeof(Vertex));

			std::vector<uint32_t> indices;
			indices.resize(header.IndexBufferCount);
			file.read((char*)indices.data(), header.IndexBufferCount * sizeof(uint32_t));

			subMeshes.emplace_back(vertices, indices);
		}

		Skeleton skeleton;
		char* name = nullptr;
		unsigned int nameLength = 0;
		file.read(reinterpret_cast<char*>(&nameLength), sizeof(unsigned int));
		name = new char[nameLength];
		file.read(reinterpret_cast<char*>(name), nameLength * sizeof(char));
		skeleton.Name = name;
		delete[] name;
		name = nullptr;

		unsigned int boneCount = 0;
		file.read(reinterpret_cast<char*>(&boneCount), sizeof(unsigned int));

		for (int i = 0; i < boneCount; i++)
		{
			Firefly::Skeleton::Bone bone;
			file.read(reinterpret_cast<char*>(&bone.BindPoseInverse), sizeof(Utils::Matrix4f));
			file.read(reinterpret_cast<char*>(&bone.Parent), sizeof(int));

			unsigned int childCount = 0;
			file.read(reinterpret_cast<char*>(&childCount), sizeof(unsigned int));
			bone.Children.resize(childCount);
			file.read(reinterpret_cast<char*>(bone.Children.data()), childCount * sizeof(unsigned int));

			unsigned int boneNameLength = 0;
			file.read(reinterpret_cast<char*>(&boneNameLength), sizeof(unsigned int));
			name = new char[boneNameLength];
			file.read(reinterpret_cast<char*>(name), boneNameLength * sizeof(char));
			bone.Name = name;
			delete[] name;
			name = nullptr;

			skeleton.Bones.push_back(bone);
		}

		unsigned int boneNameToIndexCount = 0;
		file.read(reinterpret_cast<char*>(&boneNameToIndexCount), sizeof(unsigned int));
		for (int i = 0; i < boneNameToIndexCount; i++)
		{
			unsigned int nameLength = 0;
			file.read(reinterpret_cast<char*>(&nameLength), sizeof(unsigned int));
			name = new char[nameLength];
			file.read(reinterpret_cast<char*>(name), nameLength * sizeof(char));

			unsigned int idx = 0;
			file.read(reinterpret_cast<char*>(&idx), sizeof(unsigned int));

			skeleton.BoneNameToIndex[name] = idx;
			delete[] name;
		}
		file.close();

		aAnimatedMesh->Init(subMeshes, skeleton);
		return true;
	}

	bool FBXImporter::ImportAnimationBinary(Ref<Animation> aAnimation)
	{

		std::ifstream file(aAnimation->GetPath(), std::ios::binary);

		unsigned int nameLength = 0;
		file.read(reinterpret_cast<char*> (&nameLength), sizeof(unsigned int));
		char* name = new char[nameLength];
		file.read(reinterpret_cast<char*>(name), nameLength * sizeof(char));
		aAnimation->Name = name;
		delete[] name;

		file.read(reinterpret_cast<char*>(&aAnimation->FrameCount), sizeof(unsigned int));
		file.read(reinterpret_cast<char*> (&aAnimation->FramesPerSecond), sizeof(float));
		unsigned int  locTransCount = 0;
		file.read(reinterpret_cast<char*>(&locTransCount), sizeof(unsigned int));


		aAnimation->Frames.resize(aAnimation->FrameCount);

		for (int i = 0; i < aAnimation->FrameCount; i++)
		{
			aAnimation->Frames[i].LocalTransforms.resize(locTransCount);
			for (auto& transform : aAnimation->Frames[i].LocalTransforms)
			{
				file.read(reinterpret_cast<char*>(transform.data()), (sizeof(Utils::Vector3f) * 2 + sizeof(Utils::Quaternion)));
			}
		}

		/*
			std::string animatedMeshPath = aAnimation->GetAnimatedMeshPath().string();
			unsigned int animatedMeshPathSize = animatedMeshPath.size() + 1;
			file.write(reinterpret_cast<const char*>(&animatedMeshPathSize), sizeof(unsigned int));
			file.write(reinterpret_cast<const char*>(animatedMeshPath.data()), animatedMeshPathSize * sizeof(char));
		*/

		unsigned int animatedMeshPathSize = 0;
		file.read(reinterpret_cast<char*>(&animatedMeshPathSize), sizeof(unsigned int));
		name = new char[animatedMeshPathSize];
		file.read(reinterpret_cast<char*>(name), animatedMeshPathSize * sizeof(char));
		aAnimation->SetAnimatedMeshPath(name);

		unsigned int additiveTrackBoneCount = 0;
		file.read(reinterpret_cast<char*>(&additiveTrackBoneCount), sizeof(unsigned int));

		if (!file.eof())
		{
			for (int i = 0; i < additiveTrackBoneCount; i++)
			{
				unsigned int boneIndex = 0;
				file.read(reinterpret_cast<char*>(&boneIndex), sizeof(unsigned int));

				unsigned int additiveTrackSize = 0;
				file.read(reinterpret_cast<char*>(&additiveTrackSize), sizeof(unsigned int));

				for (int j = 0; j < additiveTrackSize; j++)
				{
					Track track;
					file.read(reinterpret_cast<char*>(&track.Type), sizeof(TrackType));

					unsigned int keyframeCount = 0;
					file.read(reinterpret_cast<char*>(&keyframeCount), sizeof(unsigned int));

					for (int k = 0; k < keyframeCount; k++)
					{
						KeyFrame keyframe;
						file.read(reinterpret_cast<char*>(&keyframe), sizeof(KeyFrame));
						track.KeyFrames.push_back(keyframe);
					}
					aAnimation->myAdditiveTracks[boneIndex][track.Type] = track;
				}
			}
		}

		file.close();

		return true;
	}


	void FBXImporter::ExportMeshToBinary(Ref<Mesh> aMesh, const std::filesystem::path& aToPath)
	{
		std::ofstream file(aToPath, std::ios::binary);

		uint32_t subMeshCount = aMesh->GetSubMeshes().size();

		file.write((char*)&subMeshCount, sizeof(uint32_t));

		for (auto& subMesh : aMesh->GetSubMeshes())
		{
			auto& vertices = subMesh.GetVertices();
			auto& indices = subMesh.GetIndices();
			SubMeshHeader header;
			header.VertexBufferCount = vertices.size();
			header.IndexBufferCount = indices.size();

			file.write((char*)&header, sizeof(SubMeshHeader));

			file.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(Vertex));

			file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));
		}

		file.close();


	}

	void FBXImporter::ExportAnimatedMeshToBinary(Ref<AnimatedMesh> aMesh, const std::filesystem::path& aToPath)
	{
		std::ofstream file(aToPath, std::ios::binary);

		uint32_t subMeshCount = aMesh->GetSubMeshes().size();

		file.write((char*)&subMeshCount, sizeof(uint32_t));

		for (auto& subMesh : aMesh->GetSubMeshes())
		{
			auto& vertices = subMesh.GetVertices();
			auto& indices = subMesh.GetIndices();
			SubMeshHeader header;
			header.VertexBufferCount = vertices.size();
			header.IndexBufferCount = indices.size();

			file.write((char*)&header, sizeof(SubMeshHeader));
			file.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(Vertex));

			file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));
		}

		auto& skeleton = aMesh->GetSkeleton();

		unsigned int skeleNameLength = skeleton.Name.size() + 1;
		file.write(reinterpret_cast<const char*>(&skeleNameLength), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(skeleton.Name.data()), skeleNameLength * sizeof(char));

		unsigned int boneCount = skeleton.Bones.size();
		file.write(reinterpret_cast<const char*>(&boneCount), sizeof(unsigned int));
		for (auto& bone : skeleton.Bones)
		{
			file.write(reinterpret_cast<const char*>(&bone.BindPoseInverse), sizeof(Utils::Matrix4f));
			file.write(reinterpret_cast<const char*>(&bone.Parent), sizeof(int));

			unsigned int childCount = bone.Children.size();
			file.write(reinterpret_cast<const char*>(&childCount), sizeof(unsigned int));
			file.write(reinterpret_cast<const char*>(bone.Children.data()), bone.Children.size() * sizeof(unsigned int));

			unsigned int boneNameLength = bone.Name.size() + 1;
			file.write(reinterpret_cast<const char*>(&boneNameLength), sizeof(unsigned int));
			file.write(reinterpret_cast<const char*>(bone.Name.data()), boneNameLength * sizeof(char));
		}

		unsigned int boneNameToIndexLength = skeleton.BoneNameToIndex.size();
		file.write(reinterpret_cast<const char*>(&boneNameToIndexLength), sizeof(unsigned int));
		for (auto pair : skeleton.BoneNameToIndex)
		{
			unsigned int nameLength = pair.first.size() + 1;
			file.write(reinterpret_cast<const char*>(&nameLength), sizeof(unsigned int));
			file.write(reinterpret_cast<const char*>(pair.first.data()), nameLength * sizeof(char));

			unsigned int idx = pair.second;
			file.write(reinterpret_cast<const char*>(&idx), sizeof(unsigned int));
		}


		file.close();
	}

	void FBXImporter::ExportAnimationToBinary(Ref<Animation> aAnimation, const std::filesystem::path& aToPath)
	{
		std::ofstream file(aToPath, std::ios::binary);
		if (!file.is_open())
		{
			LOGERROR("FILE NOT OPEN");
		}

		if (file.bad())
		{
			LOGERROR("FILE BAD");
		}

		auto& name = aAnimation->Name;

		unsigned int nameLength = name.size() + 1;
		file.write(reinterpret_cast<const char*>(&nameLength), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(name.data()), nameLength * sizeof(char));

		file.write(reinterpret_cast<const char*>(&aAnimation->FrameCount), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&aAnimation->FramesPerSecond), sizeof(float));

		unsigned int  locTransCount = aAnimation->Frames[0].LocalTransforms.size();
		file.write(reinterpret_cast<const char*>(&locTransCount), sizeof(unsigned int));

		for (auto& frame : aAnimation->Frames)
		{
			for (auto& transform : frame.LocalTransforms)
			{
				file.write(reinterpret_cast<const char*>(transform.data()), (sizeof(Utils::Vector3f) * 2 + sizeof(Utils::Quaternion)));
			}
		}

		std::string animatedMeshPath = aAnimation->GetAnimatedMeshPath().string();
		unsigned int animatedMeshPathSize = animatedMeshPath.size() + 1;
		file.write(reinterpret_cast<const char*>(&animatedMeshPathSize), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(animatedMeshPath.data()), animatedMeshPathSize * sizeof(char));

		// additive tracks
		unsigned int additiveTrackBoneCount = aAnimation->myAdditiveTracks.size();
		file.write(reinterpret_cast<const char*>(&additiveTrackBoneCount), sizeof(unsigned int));

		for (auto bone : aAnimation->myAdditiveTracks)
		{
			//Bone index
			file.write(reinterpret_cast<const char*>(&bone.first), sizeof(unsigned int));

			//Additive track
			unsigned int additiveTrackSize = bone.second.size();
			file.write(reinterpret_cast<const char*>(&additiveTrackSize), sizeof(unsigned int));

			for (auto& trackPair : bone.second)
			{
				//track type
				file.write(reinterpret_cast<const char*>(&trackPair.first), sizeof(TrackType));

				//track data
				auto& track = trackPair.second;

				//Keyframe count
				unsigned int keyframeCount = track.KeyFrames.size();
				file.write(reinterpret_cast<const char*>(&keyframeCount), sizeof(unsigned int));

				for (auto& keyframe : track.KeyFrames)
				{
					file.write(reinterpret_cast<const char*>(&keyframe), sizeof(KeyFrame));
				}
			}
		}


		file.close();
	}


#pragma endregion
	void FBXImporter::ExtractSubMeshes(TGA::FBX::Model& aFbxModel, std::vector<SubMesh>& aMeshes)
	{
		aMeshes.reserve(aFbxModel.Meshes.size());

		for (auto& subMesh : aFbxModel.Meshes)
		{
			std::vector<Vertex> vertices;
			vertices.resize(subMesh.Vertices.size());

			memcpy(vertices.data(), subMesh.Vertices.data(), subMesh.Vertices.size() * sizeof(TGA::FBX::Vertex));

			//for (auto& vertex : subMesh.Vertices)
			//{
			//	//vertex.VertexColors[0][0] = (rand() / static_cast<float>(RAND_MAX) + 1) / 2;
			//	//vertex.VertexColors[0][1] = (rand() / static_cast<float>(RAND_MAX) + 1) / 2;
			//	//vertex.VertexColors[0][2] = (rand() / static_cast<float>(RAND_MAX) + 1) / 2;
			//	//vertex.VertexColors[0][3] = 1.0f;

			//	vertices.emplace_back(vertex.Position, vertex.VertexColors, vertex.UVs, vertex.Normal,
			//		vertex.Tangent, vertex.Binormal, vertex.BoneIDs, vertex.BoneWeights);
			//}

			aMeshes.emplace_back(vertices, subMesh.Indices);
		}
	}
}
