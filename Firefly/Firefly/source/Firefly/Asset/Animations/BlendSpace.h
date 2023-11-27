#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Asset.h"
#include "Firefly/Asset/Animation.h"
#include "Utils/Math/Vector2.hpp"
#include "Utils/Math/Matrix4x4.hpp"
#include "Utils/Math/Triangle.hpp"

#include <filesystem>
class BlendSpaceEditorWindow;
class ContentBrowser;
namespace Firefly
{
	class Animation;
	class AnimatedMesh;

	enum class BlendspaceType
	{
		OneDimensional,
		TwoDimensional
	};
	struct BlendSpaceEntry
	{
		Ref<Firefly::Animation> Animation;
		float HorizontalAxisPosition;
		float VerticalAxisPosition;
		float Speed;


	};

	class BlendSpace : public Asset
	{
	public:
		BlendSpace();
		BlendSpace(const BlendSpace& aOther) = default;
		static AssetType GetStaticType() { return AssetType::BlendSpace; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		BlendspaceType GetDimensionType() const { return myBlendspaceType; }
		std::string GetHorizontalAxisName() const { return myHorizontalAxis.Name; }
		std::string GetVerticalAxisName() const { return myVerticalAxis.Name; }

		std::optional<Frame> Sample1D(float aTime, float aPosition);
		float GetDuration1D(float aPosition) const;

		bool IsLoaded() const override;

		void SaveTo(const std::filesystem::path& aPath);
		/// <summary>
		/// Uses Bowyer Watson's Algorithm of computing Delauney Triangulation
		/// </summary>
		void Triangulate2D();
		std::optional<Frame> Sample2D(float aTime, const Utils::Vector2f& aPosition);
		float GetDuration2D(const Utils::Vector2f& aPosition) const;

		Ref<Animation> GetAnyAnimation() const;
		bool Empty();

		float GetDuration(const Utils::Vector2f& aPosition) const;
	private:
		friend class BlendSpaceImporter;
		friend class ::BlendSpaceEditorWindow;
		friend class ::ContentBrowser;
		/// <summary>
		/// Calculates what multiplier should be applied to the animation to get the correct speed when played together, this will make sure the from duration is a multiple of the to duration
		/// </summary>
		/// <param name="aToDuration"></param>
		/// <param name="aFromDuration"></param>
		/// <returns></returns>
		float GetDurationSyncMultiplier(float aToDuration, float aFromDuration) const;
		std::array<float, 3> CalculateWeightsInTriangle(Utils::Vector2f aPosition, const BlendSpaceEntry& aEntry0, const BlendSpaceEntry& aEntry1, const BlendSpaceEntry& aEntry2) const;
		std::optional<int> GetTriangleThatContainsPoint(const Utils::Vector2f& aPosition) const;
		Utils::Vector2f TranslatePositionIntoNearestTriangle(const Utils::Vector2f& aPosition) const;
		Utils::Vector3f CalculateBarycentricCoordinates(const Utils::Vector2f& aPosition, const Utils::Vector2f& aPoint0, const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2) const;

		bool IsPointInTriangleCircumCircle(const Utils::Vector2f& aPoint,
			const Utils::Vector2f& aTrianglePoint1,
			const Utils::Vector2f& aTrianglePoint2,
			const Utils::Vector2f& aTrianglePoint3) const;


		std::vector<int> CalculateConvexHull(const std::vector<BlendSpaceEntry>& aPoints) const;

		void LineFromPoints(const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2,
			float& aOutA, float& aOutB, float& aOutC) const;
		void PerpendicularBisectorFromLine(const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2,
			float& aOutA, float& aOutB, float& aOutC) const;
		Utils::Vector2f IntersectionOfLines(float aPointA1, float aPointB1, float aPointC1,
			float aPointA2, float aPointB2, float aPointC2) const;
		Utils::Vector2f GetTriangleCircumferenceCenter(const Utils::Vector2f& aTrianglePoint1,
			const Utils::Vector2f& aTrianglePoint2,
			const Utils::Vector2f& aTrianglePoint3) const;

		bool IsPointInsideTriangle(const Utils::Vector2f& aPoint,
			const Utils::Vector2f& aTrianglePoint1,
			const Utils::Vector2f& aTrianglePoint2,
			const Utils::Vector2f& aTrianglePoint3) const;

		Ref<Firefly::AnimatedMesh> myMesh;
		std::string myMeshPath;


		struct Axis
		{
			std::string Name;
			float Min;
			float Max;
		};
		Axis myHorizontalAxis;
		Axis myVerticalAxis;
		BlendspaceType myBlendspaceType;
		std::vector<BlendSpaceEntry> myEntries;

		//contains the indices of the entries that make up the triangles
		std::vector<std::array<int, 3>> myTriangles;
	};
}