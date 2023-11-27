#pragma once
#pragma once

#include <Utils/Math/Sphere.hpp>

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"

#include "Firefly/Rendering/RenderCommands.h"


namespace Firefly
{	
	class AnimatedMesh;
	class Animation;
	class MaterialAsset;
	class AnimatedMeshComponent : public Component
	{
	public:
		AnimatedMeshComponent();
		~AnimatedMeshComponent() = default;

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		static std::string GetFactoryName() { return "AnimatedMeshComponent"; }
		static Ref<Component> Create() { return CreateRef<AnimatedMeshComponent>(); }

		void SetMesh(std::string aMeshPath);
		Ref<AnimatedMesh> GetMesh() const { return myMesh; }
		std::string GetMeshPath() { return myMeshPath; }

		std::vector<Ref<MaterialAsset>> GetMaterials() const { return myMaterials; }
		void SetMaterials(std::vector<Ref<MaterialAsset>>& aMaterials) { myMaterials = aMaterials; }


		std::vector<std::string> GetMaterialPaths() const { return myMaterialPaths; }
		
		void SetShouldOutline(bool aShouldOutline = true);

		//void SetMaterial(const std::string& aPath);

		void SetCurrentMatrices(std::vector<Utils::Matrix4f>& someMatrices);

		//128 bonetransforms
		const Utils::Matrix4f* GetCurrentBoneTransforms() const { return myRenderInfo.BoneTransforms; }

		Utils::Matrix4f* GetCurrentBoneTransformsMutable() { return myRenderInfo.BoneTransforms; }

		void LoadAssets();

		void SetOffset(const Utils::Vector3f& aOffset) { myOffset = aOffset; }
		Utils::Vector3f GetOffset() const { return myOffset; }

		void SetPositionOverride(bool aShouldOverride, const Utils::Vector3f& aPosition = Utils::Vector3f(0.0f, 0.0f, 0.0f));

		std::vector<int> GetOnlyRenderOneSubMeshIndices() const { return myOnlyRenderOneSubMeshIndices; }
		

	private:
		void LoadMesh();
		void LoadMaterial();
		
		Utils::Matrix4f GetGlobalTransform(int aIndex, const std::vector<Utils::Matrix4f>& aTransforms);
		
		Utils::Matrix4f GetChainGlobalTransform(const std::vector<Utils::Matrix4f>& aTransforms, int aIndex);


		/*void IKChainToWorld(std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float>& someLengths, std::vector<Utils::Matrix4f>& aIKChain);
		void WorldToIkChain(std::vector<Utils::Vector3f>& someWorldPositions, std::vector<Utils::Matrix4f>& aIKChain);

		void IterateBackwards(Utils::Vector3f aTarget, std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float> aChainLengths);
		void IterateForwards(Utils::Vector3f aBasePos, std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float> aChainLengths);


		void BoneToWorldAndDecompose(const Utils::Matrix4f& aMat, const Utils::Matrix4f& aBindPose, Utils::Vector3f& aOutPos, Utils::Vector3f& aOutRot, Utils::Vector3f& aOutScale);
		void BoneFromWorldToLocalAndCompose(const Utils::Vector3f& aPos, const Utils::Vector3f& aRot, const Utils::Vector3f& aScale, const Utils::Matrix4f& aBindPoseInverse, Utils::Matrix4f& aOutMat);

		void OffsetAllChildrenRecursive(std::vector<Utils::Matrix4f>& someMatrices, Skeleton::Bone& aBone, Utils::Vector3f aOffset);
		void DrawSkeleton(const std::vector<Utils::Matrix4f>& someMatrices);
		void DrawSkeletonRecursive(const std::vector<Utils::Matrix4f>& someMatrices, Utils::Vector3f aParentPos, int aChildBoneIndex, bool aDrawWhiteFlag);
		std::vector<IKTransform> DoIKStuff();*/
		std::string myMeshPath;
		std::vector<std::string> myMaterialPaths;
		std::vector<Ref<MaterialAsset>> myMaterials;
		std::string myLastMeshPath;
		Ref<AnimatedMesh> myMesh;
		MeshSubmitInfo myRenderInfo;

		Utils::Vector3f myOffset;

		Utils::Sphere<float> myBoundingSphere;
		float myInitialRadius;

		bool myCastShadows = true;
		bool myPositionOverride;
		Utils::Vector3f myPositionOverrideValue;
		bool myShouldRender = true;

		std::vector<int> myOnlyRenderOneSubMeshIndices;
	};
}