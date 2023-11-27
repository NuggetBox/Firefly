#include "FFpch.h"
#include "Animation.h"

#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Animations/AvatarMask.h"

#include "Utils/UtilityFunctions.hpp"

#include <iostream>


namespace Firefly
{
	Frame Animation::GetFrame(float aTime, bool aLoopFlag, bool aWithKeyFrames) const
	{
		float secondsPerFrame = 1 / FramesPerSecond;
		float fraction = aTime / GetDuration();
		float frameTime = (fraction * GetDuration()) / secondsPerFrame;
		int currentFrameIndex = static_cast<int>(std::floor(frameTime)) % (FrameCount - 1);
		uint32_t nextFrameIndex = (currentFrameIndex + 1) % (FrameCount - 1);
		if (!aLoopFlag && fraction >= 1.f)
		{
			currentFrameIndex = (FrameCount - 1);
			nextFrameIndex = (FrameCount - 1);
		}
		if (!aLoopFlag && nextFrameIndex < currentFrameIndex)
		{
			nextFrameIndex = currentFrameIndex;
		}
		float delta = frameTime - std::floor(frameTime);

		Frame currentFrame = Frames[currentFrameIndex];
		Frame nextFrame = Frames[nextFrameIndex];

		currentFrame.BlendWith(nextFrame, delta);

		if (aWithKeyFrames)
		{
			for (int boneIndex = 0; boneIndex < currentFrame.LocalTransforms.size(); boneIndex++)
			{
				for (uint32_t trackType = 0; trackType < static_cast<uint32_t>(TrackType::COUNT); trackType++)
				{
					auto trackTypeEnum = static_cast<TrackType>(std::pow(2, trackType));
					if (myAdditiveTracks.contains(boneIndex))
					{
						if (myAdditiveTracks.at(boneIndex).contains(trackTypeEnum))
						{
							auto& track = myAdditiveTracks.at(boneIndex).at(trackTypeEnum);
							auto& locBoneTransform = currentFrame.LocalTransforms[boneIndex];
							switch (trackTypeEnum)
							{
								case TrackType::PositionX:
									locBoneTransform.SetXPosition(locBoneTransform.GetXPosition() + track.GetValue(aTime));
									break;
								case TrackType::PositionY:
									locBoneTransform.SetYPosition(locBoneTransform.GetYPosition() + track.GetValue(aTime));
									break;
								case TrackType::PositionZ:
									locBoneTransform.SetZPosition(locBoneTransform.GetZPosition() + track.GetValue(aTime));
									break;
								case TrackType::RotationX:
									locBoneTransform.SetRotation(locBoneTransform.GetRotation() + Utils::Vector3f(track.GetValue(aTime), 0, 0));
									break;
								case TrackType::RotationY:
									locBoneTransform.SetRotation(locBoneTransform.GetRotation() + Utils::Vector3f(0, track.GetValue(aTime), 0));
									break;
								case TrackType::RotationZ:
									locBoneTransform.SetRotation(locBoneTransform.GetRotation() + Utils::Vector3f(0, 0, track.GetValue(aTime)));
									break;
								case TrackType::ScaleX:
									locBoneTransform.SetXScale(locBoneTransform.GetScale().x * track.GetValue(aTime));
									break;
								case TrackType::ScaleY:
									locBoneTransform.SetYScale(locBoneTransform.GetScale().y * track.GetValue(aTime));
									break;
								case TrackType::ScaleZ:
									locBoneTransform.SetZScale(locBoneTransform.GetScale().z * track.GetValue(aTime));
									break;
							}
						}
					}
				}
			}
		}


		return currentFrame;
	}

	float Animation::GetDuration() const
	{
		float secondsPerFrame = 1 / FramesPerSecond;
		return (FrameCount - 1) * secondsPerFrame;
	}

	void Animation::SetAnimatedMeshPath(const std::filesystem::path& aPath)
	{
		myAnimatedMeshPath = aPath;
		myAnimatedMesh = Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(aPath);

	}

	bool Animation::IsLoaded() const
	{
		if (!myAnimatedMesh)
		{
			return false;
		}
		return myIsLoaded && myAnimatedMesh->IsLoaded();
	}

	void Frame::BlendWith(const Frame& aFrameToBlendWith, float aBlendAlpha)
	{
		if (LocalTransforms.size() != aFrameToBlendWith.LocalTransforms.size())
		{
			LOGERROR("Frame counts did not match! failed to blend.");
			return;
		}

		for (int i = 0; i < LocalTransforms.size(); ++i)
		{
			LocalTransforms[i] = Utils::BasicTransform::Lerp(LocalTransforms[i], aFrameToBlendWith.LocalTransforms[i], aBlendAlpha);
		}
	}

	void Frame::BlendWith(const Frame& aFrameToBlendWith, float aBlendAlpha, Ref<AvatarMask> aMask)
	{
		if (LocalTransforms.size() != aFrameToBlendWith.LocalTransforms.size())
		{
			LOGERROR("Frame counts did not match! failed to blend.");
			return;
		}
		for (int i = 0; i < LocalTransforms.size(); ++i)
		{
			if (!aMask->GetBonesToIgnore().contains(i))
			{
				LocalTransforms[i] = Utils::BasicTransform::Lerp(LocalTransforms[i], aFrameToBlendWith.LocalTransforms[i], aBlendAlpha * aMask->GetInfluence(i));
			}
		}
	}

	void Frame::Add(const Frame& aFrameToAdd, float aAddAlpha)
	{
		if (LocalTransforms.size() != aFrameToAdd.LocalTransforms.size())
		{
			LOGERROR("Frame counts did not match! failed to blend.");
			return;
		}

		for (int i = 0; i < LocalTransforms.size(); ++i)
		{
			LocalTransforms[i].SetPosition(aFrameToAdd.LocalTransforms[i].GetPosition() * aAddAlpha + LocalTransforms[i].GetPosition() );
			LocalTransforms[i].SetRotation(Utils::Quaternion::SLerp(LocalTransforms[i].GetQuaternion(), LocalTransforms[i].GetQuaternion() * aFrameToAdd.LocalTransforms[i].GetQuaternion(), aAddAlpha));
			LocalTransforms[i].SetScale( LocalTransforms[i].GetScale() + aFrameToAdd.LocalTransforms[i].GetScale() * aAddAlpha);
		}
	}

	void Frame::CalculateTransforms(Skeleton& aSkeleton, std::vector<Utils::Matrix4x4<float>>& aOutTransforms)
	{
		if (aSkeleton.Bones.size() != LocalTransforms.size())
		{
			LOGERROR("Skeleton and animation have a bone count missmatch!");
			return;
		}
		aOutTransforms.resize(aSkeleton.Bones.size());
		CalculateTransformsRecursive(*this, aSkeleton, 0, Utils::Matrix4x4<float>(), aOutTransforms);
	}

	void Frame::CalculateTransformsRecursive(Frame& aFrame, Skeleton& aSkeleton, uint32_t aIndex, const Utils::Matrix4x4<float>& aParentTransform, std::vector<Utils::Matrix4x4<float>>& aTransforms) const
	{
		const Skeleton::Bone& currentBone = aSkeleton.Bones[aIndex];
		Utils::Matrix4f currentBoneLocalTransform =
			Utils::Matrix4f::CreateFromPosRotScale(aFrame.LocalTransforms[aIndex].GetPosition(),
				aFrame.LocalTransforms[aIndex].GetQuaternion(), aFrame.LocalTransforms[aIndex].GetScale()).GetTranspose();
		Utils::Matrix4f currentBoneGlobalTransform = aParentTransform * currentBoneLocalTransform;


		Utils::Matrix4f finalBoneTransform;
		finalBoneTransform *= currentBoneGlobalTransform;
		finalBoneTransform *= currentBone.BindPoseInverse;
		aTransforms[aIndex] = finalBoneTransform;

		//Dont calculate children with BindPoseInverse
		for (uint32_t i = 0; i < currentBone.Children.size(); ++i)
		{
			CalculateTransformsRecursive(aFrame, aSkeleton, currentBone.Children[i], currentBoneGlobalTransform, aTransforms);
		}
	}
}

float Track::GetValue(float aTime) const
{
	if (KeyFrames.size() == 0)
	{
		if (static_cast<uint32_t>(Type & TrackType::SCALE) != 0)
		{
			return 1.f;
		}
		else
		{
			return 0.0f;
		}
	}

	if (KeyFrames.size() == 1)
	{
		return KeyFrames[0].Value;
	}

	//find the closest keyframe before the time
	int closestKeyframeIndex = -1;
	for (int i = 0; i < KeyFrames.size(); i++)
	{
		if (KeyFrames[i].Frame < aTime)
		{
			closestKeyframeIndex = i;
		}
	}

	//if is after the last keyframe
	if (closestKeyframeIndex == KeyFrames.size() - 1)
	{
		return KeyFrames[closestKeyframeIndex].Value;
	}
	//if is before the first keyframe
	if (closestKeyframeIndex == -1)
	{
		return KeyFrames[0].Value;
	}

	const auto& keyframe1 = KeyFrames[closestKeyframeIndex];
	const auto& keyframe2 = KeyFrames[closestKeyframeIndex + 1];

	const float fraction = (aTime - keyframe1.Frame) / (keyframe2.Frame - keyframe1.Frame);
	switch (keyframe1.CurveType)
	{
		case CurveType::Constant:
			return GetValueConstant(keyframe1, keyframe2, fraction);
		case CurveType::Linear:
			return GetValueLinear(keyframe1, keyframe2, fraction);
		case CurveType::CubicBroken:
			return GetValueCubic(keyframe1, keyframe2, fraction);
		default:
			return 0.0f;
	}

	return Utils::Lerp(keyframe1.Value, keyframe2.Value, fraction);
}


float Track::GetValueLinear(const KeyFrame& aFirstKey, const KeyFrame& aSecond, float aFraction) const
{
	return Utils::Lerp(aFirstKey.Value, aSecond.Value, aFraction);
}

float Track::GetValueConstant(const KeyFrame& aFirstKey, const KeyFrame& aSecond, float aFraction) const
{
	return aFirstKey.Value;
}

float Track::GetValueCubic(const KeyFrame& aFirstKey, const KeyFrame& aSecondKey, float aFraction) const
{
	const auto aPoint1 = aFirstKey.TangentOut + Utils::Vector2f(aFirstKey.Frame, aFirstKey.Value);
	const auto aPoint2 = aSecondKey.TangentIn + Utils::Vector2f(aSecondKey.Frame, aSecondKey.Value);

	const float targetXPos = aFirstKey.Frame + (aSecondKey.Frame - aFirstKey.Frame) * aFraction;


	Utils::Vector2f beforePoint;
	Utils::Vector2f afterPoint;
	for (int i = 0; i <= 10; i++)
	{
		float t = i / 10.0f;
		Utils::Vector2f result;
		result += (1 - t) * (1 - t) * (1 - t) * Utils::Vector2f(aFirstKey.Frame, aFirstKey.Value);
		result += 3 * (1 - t) * (1 - t) * t * aPoint1;
		result += 3 * (1 - t) * t * t * aPoint2;
		result += t * t * t * Utils::Vector2f(aSecondKey.Frame, aSecondKey.Value);
		if (result.x < targetXPos)
		{
			beforePoint = result;
		}
		else
		{
			afterPoint = result;
			break;
		}
	}

	auto length = afterPoint.x - beforePoint.x;
	if (length == 0)
	{
		return beforePoint.y;
	}

	auto diff = targetXPos - beforePoint.x;
	float delta = diff / length;

	return Utils::Lerp(beforePoint.y, afterPoint.y, delta);
}
