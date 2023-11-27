#pragma once
#include "Firefly/Asset/Asset.h"

#include <Utils/Math/Vector3.hpp>
#include <Utils/Math/Matrix4x4.hpp>

#include <vector>
#include "Firefly/Core/Core.h"
#include "Utils/Math/Transform.h"
#include <unordered_map>

class AnimationEditor;

enum class TrackType : uint32_t
{
	None = 0,
	PositionX = (1 << 0),
	PositionY = (1 << 1),
	PositionZ = (1 << 2),
	RotationX = (1 << 3),
	RotationY = (1 << 4),
	RotationZ = (1 << 5),
	ScaleX = (1 << 6),
	ScaleY = (1 << 7),
	ScaleZ = (1 << 8),

	POSITION = PositionX | PositionY | PositionZ,
	ROTATION = RotationX | RotationY | RotationZ,
	SCALE = ScaleX | ScaleY | ScaleZ,
	ALL = (1 << 9) - 1,

	COUNT = 9
};

inline std::string TrackTypeToString(TrackType aTrackType)
{
	switch (aTrackType)
	{
		case TrackType::PositionX:
			return "Position X";
		case TrackType::PositionY:
			return "Position Y";
		case TrackType::PositionZ:
			return "Position Z";
		case TrackType::RotationX:
			return "Rotation X";
		case TrackType::RotationY:
			return "Rotation Y";
		case TrackType::RotationZ:
			return "Rotation Z";
		case TrackType::ScaleX:
			return "Scale X";
		case TrackType::ScaleY:
			return "Scale Y";
		case TrackType::ScaleZ:
			return "Scale Z";
		default:
			return "Unknown";
	}
}

enum class CurveType : uint32_t
{
	Linear = 0,
	Constant = 1,
	CubicFlat = 2,
	CubicBroken = 3,
};
struct KeyFrame
{
	uint32_t ID;
	float Value;
	float Frame; // float since it can be between frames
	CurveType CurveType = CurveType::CubicFlat;
	Utils::Vector2f TangentIn = Utils::Vector2f(1,0);
	Utils::Vector2f TangentOut = Utils::Vector2f(0,1);
};

class Track
{
public:
	//Frame, keframe
	std::vector<KeyFrame> KeyFrames;
	TrackType Type;
	float GetValue(float aTime) const;

private:
	float GetValueLinear(const KeyFrame& aFirstKey, const KeyFrame& aSecond, float aFraction) const ;
	float GetValueConstant(const KeyFrame& aFirstKey, const KeyFrame& aSecond, float aFraction) const ;
	float GetValueCubic(const KeyFrame& aFirstKey, const KeyFrame& aSecond, float aFraction) const ;

};


namespace Firefly
{
	class AnimatedMesh;
	class AvatarMask;
	class FBXImporter;
	struct IKTransform
	{
		int BoneIndex;
		Utils::Matrix4f Transform;
	};
	struct Skeleton;

	class Frame
	{
	public:
		std::vector<Utils::BasicTransform> LocalTransforms;

		void BlendWith(const Frame& aFrameToBlendWith, float aBlendAlpha);
		void BlendWith(const Frame& aFrameToBlendWith, float aBlendAlpha, Ref<AvatarMask> aMask);
		void Add(const Frame& aFrameToAdd, float aAddAlpha);
		void CalculateTransforms(Skeleton& aSkeleton, std::vector<Utils::Matrix4x4<float>>& aOutTransforms);
	private:
		void CalculateTransformsRecursive(Frame& aFrame, Skeleton& aSkeleton, uint32_t aIndex,
			const Utils::Matrix4x4<float>& aParentTransform, std::vector<Utils::Matrix4x4<float>>& aTransforms) const;
	};

	class Animation : public Asset
	{
	public:
		Animation() = default;
		~Animation() = default;

		static AssetType GetStaticType() { return AssetType::Animation; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		Frame GetFrame(float aTime, bool aLoopFlag, bool aWithKeyFrames = true) const;
		float GetDuration() const;

		Ref<AnimatedMesh> GetAnimatedMesh() const { return myAnimatedMesh; }
		std::filesystem::path GetAnimatedMeshPath() const { return myAnimatedMeshPath; }

		void SetAnimatedMeshPath(const std::filesystem::path& aPath);

		bool IsLoaded() const override;

		std::vector<Frame> Frames;
		unsigned int FrameCount = 0;
		float FramesPerSecond = 0;
		std::string Name;

	private:
		friend class ::AnimationEditor;
		friend class Firefly::FBXImporter;


		std::filesystem::path myAnimatedMeshPath;
		Ref<AnimatedMesh> myAnimatedMesh;

		std::unordered_map<int, std::unordered_map<TrackType, Track>> myAdditiveTracks;


	};
}



inline TrackType operator&(TrackType a, TrackType b)
{
	return static_cast<TrackType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline TrackType operator|(TrackType a, TrackType b)
{
	return static_cast<TrackType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline TrackType operator~(TrackType a)
{
	return static_cast<TrackType>(~static_cast<uint32_t>(a));
}