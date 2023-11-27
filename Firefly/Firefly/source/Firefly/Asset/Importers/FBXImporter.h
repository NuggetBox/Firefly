#pragma once
#include "Firefly/Core/Core.h"
#include <filesystem>
#include <mutex>

namespace TGA
{
	namespace FBX
	{
		struct Model;
	}
}

namespace Firefly

{
	class Mesh;
	class AnimatedMesh;
	class SubMesh;
	class Animation;
	class FBXImporter
	{

		struct SubMeshHeader
		{
			uint32_t VertexBufferCount;
			uint32_t VertexStrideSize;
			uint32_t IndexBufferCount;
		};
	public:
		FBXImporter();
		~FBXImporter();

		Ref<Mesh> ImportMesh(const std::filesystem::path& aPath);
		Ref<AnimatedMesh> ImportAnimatedMesh(const std::filesystem::path& aPath);
		Ref<Animation> ImportAnimation(const std::filesystem::path& aPath,const std::filesystem::path& aSkeletonPath);

		bool ImportMeshBinary(Ref<Mesh> aMesh);
		bool ImportAnimatedMeshBinary(Ref<AnimatedMesh> aAnimatedMesh);
		bool ImportAnimationBinary(Ref<Animation> aAnimation);
		
		void ExportMeshToBinary(Ref<Mesh> aMesh, const std::filesystem::path& aToPath);
		void ExportAnimatedMeshToBinary(Ref<AnimatedMesh> aMesh, const std::filesystem::path& aToPath);
		void ExportAnimationToBinary(Ref<Animation> aAnimation, const std::filesystem::path& aToPath);


	private:
		void ExtractSubMeshes(TGA::FBX::Model& aFbxModel, std::vector<SubMesh>& aMeshes);

		std::mutex myImporterMutex;



	};
}