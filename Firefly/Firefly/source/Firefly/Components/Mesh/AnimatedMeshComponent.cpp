#include "FFpch.h"
#include "AnimatedMeshComponent.h"

#include "Firefly/Rendering/Renderer.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"

#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/ResourceCache.h"

#include "Firefly/Application/Application.h"

namespace Firefly
{
	REGISTER_COMPONENT(AnimatedMeshComponent);

	AnimatedMeshComponent::AnimatedMeshComponent()
		:Component("AnimatedMeshComponent"), myRenderInfo(Utils::Matrix4f(), true)
	{
		myRenderInfo.CastShadows = true;
		myRenderInfo.Outline = false;
		myMeshPath = "";
		myOffset = { 0,0,0 };

		myMaterialPaths.resize(5);

		EditorVariable("Skeleton", ParameterType::File, &myMeshPath, ".skeleton");
		EditorListVariable("Materials", ParameterType::File, &myMaterialPaths, ".mat");
		EditorListDefaultOpen();
		EditorVariable("Offset", ParameterType::Vec3, &myOffset);
		EditorVariable("Cast Shadows", ParameterType::Bool, &myCastShadows);
		EditorVariable("Should Render", ParameterType::Bool, &myShouldRender);
		EditorListVariable("Only Render Certain Submeshes", Firefly::ParameterType::Int, &myOnlyRenderOneSubMeshIndices);

		myPositionOverride = false;
		myPositionOverrideValue = { 0,0,0 };

		myInitialRadius = 1000.0f;
	}
	void AnimatedMeshComponent::Initialize()
	{
		myRenderInfo.EntityID = myEntity->GetID();
		myRenderInfo.CreationTime = myEntity->GetCreationTime();
		LoadMesh();
		LoadMaterial();
	}
	void AnimatedMeshComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& e)
			{
				if (!myShouldRender)
				{
					return false;
				}

				if (myMesh)
				{
					if (myMesh->IsLoaded())
					{

						//if (myAnimation)
						//{
						//	std::vector<IKTransform> iks ;
						//	//iks = DoIKStuff();

						//	matrices = myAnimation->Sample(myCurrentAnimationTime, myRenderInfo.Mesh->GetSkeleton(),
						//		myBlendAnimation ? myBlendAnimation : nullptr, myBlendAlpha,
						//		myBlendAnimationTime,myLooping, iks);
						//}
						//DrawSkeleton(matrices);
						myRenderInfo.Transform = myEntity->GetTransform().GetMatrix();
						if (myPositionOverride)
						{
							myRenderInfo.Transform(4, 1) = myPositionOverrideValue.x;
							myRenderInfo.Transform(4, 2) = myPositionOverrideValue.y;
							myRenderInfo.Transform(4, 3) = myPositionOverrideValue.z;
						}
						if (!myOffset.IsAlmostEqual(Utils::Vector3f(0, 0, 0)))
						{
							//add offset

							myRenderInfo.Transform(4, 1) = myRenderInfo.Transform(4, 1) + myOffset.x;
							myRenderInfo.Transform(4, 2) = myRenderInfo.Transform(4, 2) + myOffset.y;
							myRenderInfo.Transform(4, 3) = myRenderInfo.Transform(4, 3) + myOffset.z;
						}

						myRenderInfo.BoundingSphere = myMesh->GetBoundingSphere();
						{
							myRenderInfo.BoundingSphere.SetOrigin(myEntity->GetTransform().GetPosition());
							float scale = myEntity->GetTransform().GetScale().x;
							if (myEntity->GetTransform().GetScale().y > scale) scale = myEntity->GetTransform().GetScale().y;
							if (myEntity->GetTransform().GetScale().z > scale) scale = myEntity->GetTransform().GetScale().z;

							myRenderInfo.BoundingSphere.SetRadius(myInitialRadius * scale);
						}

						myRenderInfo.IsAnimation = true;
						myRenderInfo.CastShadows = myCastShadows;
						myRenderInfo.AutomaticCleanup = false;
						int index = 0;
						for (auto& submesh : myMesh->GetSubMeshes())
						{
							if (!myOnlyRenderOneSubMeshIndices.empty())
							{
								auto it = std::find(myOnlyRenderOneSubMeshIndices.begin(), myOnlyRenderOneSubMeshIndices.end(), index);
								if (it == myOnlyRenderOneSubMeshIndices.end())
								{
									index++;
									continue;
								}
							}

							myRenderInfo.Mesh = &submesh;
							if (index < myMaterials.size())
							{
								myRenderInfo.Material = myMaterials[index];
							}

							Renderer::Submit(myRenderInfo);
							index++;
						}
					}

				}
				
				return false;
			});

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				if (e.GetParamName() == "Skeleton")
				{
					LoadMesh();
				}
				else if (e.GetParamName() == "Materials")
				{
					LoadMaterial();
				}
				return false;
			});
	}
	void AnimatedMeshComponent::SetMesh(std::string aMeshPath)
	{
		myMeshPath = aMeshPath;
		LoadMesh();
	}

	void AnimatedMeshComponent::SetShouldOutline(bool aShouldOutline)
	{
		myRenderInfo.Outline = aShouldOutline;
	}

	/*void AnimatedMeshComponent::SetMaterial(const std::string& aPath)
	{
		myMaterialPath = aPath;
		LoadMaterial();
	}*/

	void AnimatedMeshComponent::SetCurrentMatrices(std::vector<Utils::Matrix4f>& someMatrices)
	{
		for (int i = 0; i < someMatrices.size(); i++)
		{
			myRenderInfo.BoneTransforms[i] = someMatrices[i];
		}
	}

	void AnimatedMeshComponent::LoadAssets()
	{
		LoadMesh();
		LoadMaterial();
	}

	void AnimatedMeshComponent::SetPositionOverride(bool aShouldOverride, const Utils::Vector3f& aPosition)
	{
		myPositionOverride = aShouldOverride;
		myPositionOverrideValue = aPosition;
	}

	void AnimatedMeshComponent::LoadMesh()
	{
		if (myMeshPath != "")
		{
			myMesh = ResourceCache::GetAsset<AnimatedMesh>(myMeshPath);

			if (myMesh)
			{
				for (int i = 0; i < myMesh->GetSkeleton().Bones.size(); i++)
				{
					myRenderInfo.BoneTransforms[i].SetIdentity();
				}
			}
		}
		else
		{
			myRenderInfo.Mesh = nullptr;
		}
	}
	void AnimatedMeshComponent::LoadMaterial()
	{
		if (myMaterialPaths.empty())
		{
			return;
		}
		else
		{
			myMaterials.resize(myMaterialPaths.size());
			for (int i = 0; i < myMaterialPaths.size(); i++)
			{
				if (myMaterialPaths[i].empty())
				{
					continue;
				}
				myMaterials[i] = ResourceCache::GetAsset<MaterialAsset>(myMaterialPaths[i]);
			}
		}
	}

	//Utils::Matrix4f AnimatedMeshComponent::GetGlobalTransform(int aIndex, const std::vector<Utils::Matrix4f>& aTransforms)
	//{
	//	Utils::Matrix4f result = aTransforms[aIndex];
	//	for (int parent = myRenderInfo.Mesh->GetSkeleton().Bones[aIndex].Parent; parent >= 0;
	//		parent = myRenderInfo.Mesh->GetSkeleton().Bones[parent].Parent)
	//	{
	//		result = aTransforms[parent] * result;
	//	}
	//	result = result;
	//	return result;
	//}
	//Utils::Matrix4f AnimatedMeshComponent::GetChainGlobalTransform(const std::vector<Utils::Matrix4f>& aChainMatrices, int aIndex)
	//{
	//	auto world = aChainMatrices[aIndex];
	//	for (int k = aIndex - 1; k >= 0; k--)
	//	{
	//		world = aChainMatrices[k] * world;
	//	}
	//	return world;
	//}
	//void AnimatedMeshComponent::IKChainToWorld(std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float>& someLengths, std::vector<Utils::Matrix4f>& aIKChain)
	//{
	//	//IK chain to world positions and lengths
	//	for (int i = 0; i < aIKChain.size(); i++)
	//	{

	//		//Get Global Transform
	//		auto world = GetChainGlobalTransform(aIKChain, i);
	//		//collect the position from the world matrix
	//		Utils::Vector3f worldPos = { world(1,4), world(2,4), world(3,4) };
	//		someWorldPositions [i] = worldPos;

	//		if (i >= 1)
	//		{
	//			Utils::Vector3f prev = someWorldPositions[i - 1];
	//			someLengths[i] = (prev - worldPos).Length();
	//		}
	//	}
	//}
	//void AnimatedMeshComponent::WorldToIkChain(std::vector<Utils::Vector3f>& someWorldPositions, std::vector<Utils::Matrix4f>& aIKChain)
	//{
	//	for (int i = 0; i < aIKChain.size() - 1; i++)
	//	{
	//		auto world = GetChainGlobalTransform(aIKChain, i);
	//		auto next = GetChainGlobalTransform(aIKChain, i + 1);

	//		Utils::Vector3f worldPos;
	//		Utils::Quaternion worldRot;
	//		Utils::Vector3f worldScale;
	//		Utils::Matrix4f::Decompose(world.GetTranspose(),
	//			worldPos, worldRot, worldScale);

	//		Utils::Vector3f nextPos;
	//		Utils::Quaternion nextRot;
	//		Utils::Vector3f nextScale;
	//		Utils::Matrix4f::Decompose(next.GetTranspose(),
	//			nextPos, nextRot, nextScale);


	//		auto toNext = nextPos - worldPos;
	//		toNext = worldRot.GetInverse() * toNext;

	//		auto toDesired = someWorldPositions[i + 1] - worldPos;
	//		toDesired = worldRot.GetInverse() * toDesired;

	//		Utils::Quaternion delta = Utils::Quaternion::FromTo(toNext, toDesired);

	//		Utils::Matrix4f rotMat = Utils::Matrix4f::CreateRotationMatrix(delta).GetTranspose();

	//		aIKChain[i] = aIKChain[i] * rotMat;

	//	}
	//}
	//void AnimatedMeshComponent::IterateBackwards(Utils::Vector3f aTarget, std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float> aChainLengths)
	//{
	//	someWorldPositions.back() = aTarget;
	//	for (int j = someWorldPositions.size() - 2; j >= 0; --j)
	//	{
	//		auto diff = someWorldPositions[j] - someWorldPositions[j + 1];
	//		auto dir = diff.GetNormalized();

	//		someWorldPositions[j] = someWorldPositions[j + 1] + dir * aChainLengths[j];
	//	}
	//}
	//void AnimatedMeshComponent::IterateForwards(Utils::Vector3f aBasePos, std::vector<Utils::Vector3f>& someWorldPositions, std::vector<float> aChainLengths)
	//{
	//	someWorldPositions[0] = aBasePos;
	//	for (int j = 1; j < someWorldPositions.size(); ++j)
	//	{
	//		auto diff = someWorldPositions[j] - someWorldPositions[j - 1];
	//		auto dir = diff.GetNormalized();

	//		someWorldPositions[j] = someWorldPositions[j - 1] + dir * aChainLengths[j];
	//	}
	//}
	//void AnimatedMeshComponent::BoneToWorldAndDecompose(const Utils::Matrix4f& aMat, const Utils::Matrix4f& aBindPose, Utils::Vector3f& aOutPos, Utils::Vector3f& aOutRot, Utils::Vector3f& aOutScale)
	//{
	//	auto worldMat = aMat * aBindPose * myEntity->GetTransform().GetMatrix().GetTranspose();
	//	worldMat = worldMat.GetTranspose();
	//	Utils::Matrix4f::Decompose(worldMat, aOutPos, aOutRot, aOutScale);
	//}
	//void AnimatedMeshComponent::BoneFromWorldToLocalAndCompose(const Utils::Vector3f& aPos, const Utils::Vector3f& aRot, const Utils::Vector3f& aScale, const Utils::Matrix4f& aBindPoseInverse, Utils::Matrix4f& aOutMat)
	//{
	//	auto mat = Utils::Matrix4f::CreateFromPosRotScale(aPos, aRot, aScale).GetTranspose();
	//	aOutMat = mat * Utils::Matrix4f::GetInverse(myEntity->GetTransform().GetMatrix().GetTranspose()) * aBindPoseInverse;
	//}
	//void AnimatedMeshComponent::OffsetAllChildrenRecursive(std::vector<Utils::Matrix4f>& someMatrices, Skeleton::Bone& aBone, Utils::Vector3f aOffset)
	//{
	//	for (int k = 0; k < aBone.Children.size(); k++)
	//	{
	//		auto child = aBone.Children[k];
	//		Utils::Vector3f pos;
	//		Utils::Vector3f rot;
	//		Utils::Vector3f scale;
	//		BoneToWorldAndDecompose(someMatrices[child],
	//			Utils::Matrix4f::GetInverse(myRenderInfo.Mesh->GetSkeleton().Bones[child].BindPoseInverse),
	//			pos, rot, scale);

	//		pos += aOffset;


	//		BoneFromWorldToLocalAndCompose(pos, rot, scale,
	//			myRenderInfo.Mesh->GetSkeleton().Bones[child].BindPoseInverse,
	//			someMatrices[child]);

	//		OffsetAllChildrenRecursive(someMatrices, myRenderInfo.Mesh->GetSkeleton().Bones[child], aOffset);
	//	}
	//}
	//void AnimatedMeshComponent::DrawSkeleton(const std::vector<Utils::Matrix4f>& someMatrices)
	//{
	//	std::vector<Utils::Vector3f> worldPositions;
	//	auto& rootBone = myRenderInfo.Mesh->GetSkeleton().Bones[0];
	//	auto& bindPoseInverse = rootBone.BindPoseInverse;
	//	auto worldMat = myEntity->GetTransform().GetMatrix().GetTranspose() * someMatrices[0] * Utils::Matrix4f::GetInverse(bindPoseInverse);
	//	worldMat = worldMat.GetTranspose();
	//	Utils::Vector3f pos;
	//	Utils::Vector3f rot;
	//	Utils::Vector3f scale;
	//	Utils::Matrix4f::Decompose(worldMat, pos, rot, scale);
	//	for (int i = 0; i < rootBone.Children.size(); i++)
	//	{
	//		DrawSkeletonRecursive(someMatrices, pos, rootBone.Children[i], true);
	//	}

	//}
	//void AnimatedMeshComponent::DrawSkeletonRecursive(const std::vector<Utils::Matrix4f>& someMatrices, Utils::Vector3f aParentPos, int aChildBoneIndex, bool aDrawWhiteFlag)
	//{
	//	auto& bone = myRenderInfo.Mesh->GetSkeleton().Bones[aChildBoneIndex];
	//	auto& bindPoseInverse = bone.BindPoseInverse;
	//	auto worldMat = someMatrices[aChildBoneIndex] * Utils::Matrix4f::GetInverse(bindPoseInverse) * myEntity->GetTransform().GetMatrix().GetTranspose();
	//	worldMat = worldMat.GetTranspose();
	//	Utils::Vector3f pos;
	//	Utils::Vector3f rot;
	//	Utils::Vector3f scale;
	//	Utils::Matrix4f::Decompose(worldMat, pos, rot, scale);

	//	Renderer::SubmitDebugLine(aParentPos, pos, { (float)aDrawWhiteFlag,(float)aDrawWhiteFlag,(float)aDrawWhiteFlag,1 });

	//	for (int i = 0; i < bone.Children.size(); i++)
	//	{
	//		DrawSkeletonRecursive(someMatrices, pos, bone.Children[i], !aDrawWhiteFlag);
	//	}
	//}
	//std::vector<IKTransform> AnimatedMeshComponent::DoIKStuff()
	//{
	//	std::vector<IKTransform> iks;
	//	auto targetEnt = GetEntityWithName("FootTarget");
	//	if (targetEnt)
	//	{
	//		auto locTransforms = myAnimation->GetFrame(myCurrentAnimationTime, myRenderInfo.Mesh->GetSkeleton(),
	//			myBlendAnimation ? myBlendAnimation : nullptr, myBlendAlpha,
	//			myBlendAnimationTime, myLooping).LocalTransforms;
	//		std::vector<Utils::Matrix4f> locMats;
	//		locMats.resize(locTransforms.size());
	//		for (int i = 0; i < locMats.size(); i++)
	//		{
	//			locMats[i] = locTransforms[i].Matrix;
	//		}

	//		auto entityMat = myEntity->GetTransform().GetMatrix().GetTranspose();
	//		std::vector<int> chainIndices{ 23,24,25 };
	//		std::vector < Utils::Matrix4f> chainMatrices = {
	//			 entityMat * GetGlobalTransform(23, locMats),
	//			locMats[24],
	//			locMats[25]
	//		};


	//		std::vector<Utils::Vector3f> worldPositions;
	//		worldPositions.resize(chainIndices.size());
	//		std::vector<float> lengths;
	//		lengths.resize(chainIndices.size());
	//		for (auto& len : lengths)
	//		{
	//			len = 0;
	//		}

	//		IKChainToWorld(worldPositions, lengths, chainMatrices);

	//		auto targetPos = targetEnt->GetTransform().GetPosition();
	//		auto basePos = worldPositions[0];

	//		for (int iteration = 0; iteration < 10; iteration++)
	//		{
	//			auto effectorPos = worldPositions.back();
	//			if ((effectorPos - targetPos).LengthSqr() < 0.01f)
	//			{
	//				WorldToIkChain(worldPositions, chainMatrices);
	//				break;
	//			}

	//			IterateBackwards(targetPos, worldPositions, lengths);
	//			IterateForwards(basePos, worldPositions, lengths);

	//			//WorldToIkChain(worldPositions, chainMatrices);

	//			////Hinge constraint
	//			//{

	//			//	const int knee = 1;
	//			//	const Utils::Vector3f axis = { 1,0,0 };
	//			//	Utils::Matrix4f joint = GetChainGlobalTransform(chainMatrices, knee);
	//			//	Utils::Matrix4f parent = GetChainGlobalTransform(chainMatrices, knee - 1);

	//			//	Utils::Vector3f throwaway;
	//			//	Utils::Vector3f jointPos;
	//			//	Utils::Quaternion jointRot;
	//			//	Utils::Quaternion parentRot;
	//			//	Utils::Matrix4f::Decompose(joint.GetTranspose(), jointPos, jointRot, throwaway);
	//			//	Utils::Matrix4f::Decompose(parent.GetTranspose(), throwaway, parentRot, throwaway);

	//			//	
	//			//	Utils::Vector3f currentHinge = jointRot * axis;
	//			//	Utils::Vector3f desiredHinge = parentRot * axis;   

	//			//	//Firefly::Renderer::SubmitDebugLine(jointPos, jointPos + currentHinge * 100.f, {0,1,0,0});
	//			//	//Firefly::Renderer::SubmitDebugLine(jointPos, jointPos + desiredHinge * 100.f, {0,0,1,0});
	//			//

	//			//	chainMatrices[knee] = chainMatrices[knee] * Utils::Matrix4f::CreateRotationMatrix(Utils::Quaternion::FromTo(currentHinge, desiredHinge)).GetTranspose();
	//			//}


	//			//IKChainToWorld(worldPositions,lengths, chainMatrices);

	//		}
	//		WorldToIkChain(worldPositions, chainMatrices);

	//		auto rootWorld = entityMat * GetGlobalTransform(myRenderInfo.Mesh->GetSkeleton().Bones[chainIndices[0]].Parent, locMats);
	//		chainMatrices[0] = Utils::Matrix4f::GetInverse(rootWorld) * chainMatrices[0];
	//		for (int i = 0; i < chainIndices.size(); i++)
	//		{
	//			iks.push_back({ chainIndices[i], chainMatrices[i] });
	//		}
	//		for (int i = 1; i < worldPositions.size(); i++)
	//		{
	//			Renderer::SubmitDebugLine(worldPositions[i - 1], worldPositions[i], { 0,1,0,1 });
	//		}

	//		Renderer::SubmitDebugSphere(worldPositions.back(), 5);
	//	}
	//	return iks;
	//}
}
