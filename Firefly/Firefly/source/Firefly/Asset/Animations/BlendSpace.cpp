#include "FFpch.h"
#include "BlendSpace.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Core/Log/DebugLogger.h"
#include "nlohmann/json.hpp"

#include "Firefly/Asset/ResourceCache.h"

#include "Utils/Math/LineVolume.hpp"
#include <set>

namespace Firefly
{
	Firefly::BlendSpace::BlendSpace()
	{
		myHorizontalAxis.Name = "Horizontal";
		myHorizontalAxis.Min = 0;
		myHorizontalAxis.Max = 100;

		myVerticalAxis.Name = "Vertical";
		myVerticalAxis.Min = 0;
		myVerticalAxis.Max = 100;

		myBlendspaceType = BlendspaceType::OneDimensional;
	}
	std::optional<Frame> BlendSpace::Sample1D(float aTime, float aPosition)
	{
		if (!myMesh)
		{
			LOGERROR("BlendSpace::Sample1D: Missing AnimatedMesh at Path: {}", myMeshPath);
			return std::nullopt;
		}
		if (myEntries.size() == 0)
		{
			//LOGERROR("BlendSpace::Sample1D: BlendSpace has no entries"); // Spaming the log
			return std::nullopt;
		}
		if (myEntries.size() == 1)
		{
			return myEntries[0].Animation->GetFrame(aTime / myEntries[0].Speed, true);
		}

		// Find the two closest entries
		BlendSpaceEntry* firstEntry = nullptr;
		BlendSpaceEntry* secondEntry = nullptr;
		for (BlendSpaceEntry& entry : myEntries)
		{
			if (entry.HorizontalAxisPosition <= aPosition)
			{
				if (firstEntry == nullptr || firstEntry->HorizontalAxisPosition < entry.HorizontalAxisPosition)
				{
					firstEntry = &entry;
				}
			}
			else
			{
				if (secondEntry == nullptr || secondEntry->HorizontalAxisPosition > entry.HorizontalAxisPosition)
				{
					secondEntry = &entry;
				}
			}
		}

		// If we have two entries, interpolate between them
		if (firstEntry != nullptr && secondEntry != nullptr)
		{
			const float firstEntryWeight = (secondEntry->HorizontalAxisPosition - aPosition) /
				(secondEntry->HorizontalAxisPosition - firstEntry->HorizontalAxisPosition);
			const float secondEntryWeight = 1.0f - firstEntryWeight;

			const float firstEntryDuration = firstEntry->Animation->GetDuration() / firstEntry->Speed;
			const float secondEntryDuration = secondEntry->Animation->GetDuration() / secondEntry->Speed;
			float duration = GetDuration1D(aPosition);

			float firstEntrySyncModifier = 1.0f;
			float secondEntrySyncModifier = 1.0f;

			firstEntrySyncModifier = GetDurationSyncMultiplier(duration, firstEntryDuration);
			secondEntrySyncModifier = GetDurationSyncMultiplier(duration, secondEntryDuration);

			auto firstEntryFrame = firstEntry->Animation->GetFrame(aTime * firstEntry->Speed / firstEntrySyncModifier, true);
			auto secondEntryFrame = secondEntry->Animation->GetFrame(aTime * secondEntry->Speed / secondEntrySyncModifier, true);

			secondEntryFrame.BlendWith(firstEntryFrame, firstEntryWeight);
			return secondEntryFrame;
		}
		else if (firstEntry)
		{
			return firstEntry->Animation->GetFrame(aTime * firstEntry->Speed, true);
		}
		else if (secondEntry)
		{
			return secondEntry->Animation->GetFrame(aTime * secondEntry->Speed, true);
		}
		return std::nullopt;
	}
	float BlendSpace::GetDuration1D(float aPosition) const
	{
		// Find the two closest entries
		const BlendSpaceEntry* firstEntry = nullptr;
		const BlendSpaceEntry* secondEntry = nullptr;
		for (const BlendSpaceEntry& entry : myEntries)
		{
			if (entry.HorizontalAxisPosition <= aPosition)
			{
				if (firstEntry == nullptr || firstEntry->HorizontalAxisPosition < entry.HorizontalAxisPosition)
				{
					firstEntry = &entry;
				}
			}
			else
			{
				if (secondEntry == nullptr || secondEntry->HorizontalAxisPosition > entry.HorizontalAxisPosition)
				{
					secondEntry = &entry;
				}
			}
		}


		if (firstEntry != nullptr && secondEntry != nullptr)
		{
			const float firstEntryWeight = (secondEntry->HorizontalAxisPosition - aPosition) /
				(secondEntry->HorizontalAxisPosition - firstEntry->HorizontalAxisPosition);
			const float secondEntryWeight = 1.0f - firstEntryWeight;

			float firstEntryDuration = firstEntry->Animation->GetDuration() / firstEntry->Speed;
			float secondEntryDuration = secondEntry->Animation->GetDuration() / secondEntry->Speed;

			float finalDuration = Utils::Lerp(firstEntryDuration, secondEntryDuration, secondEntryWeight);

			return finalDuration;
		}
		else if (firstEntry != nullptr)
		{
			return firstEntry->Animation->GetDuration() / firstEntry->Speed;
		}
		else if (secondEntry != nullptr)
		{
			return secondEntry->Animation->GetDuration() / secondEntry->Speed;
		}
		else
		{
			return 0.0f;
		}
	}

	bool BlendSpace::IsLoaded() const
	{
		if (!myMesh)
		{
			return false;
		}
		if (!myMesh->IsLoaded())
		{
			return false;
		}
		if (myIsLoaded)
		{
			for (int i = 0; i < myEntries.size(); ++i)
			{
				if (!myEntries[i].Animation->IsLoaded())
				{
					return false;
				}
			}
		}
		return myIsLoaded;
	}

	void BlendSpace::SaveTo(const std::filesystem::path& aPath)
	{
		nlohmann::json json;
		json["Type"] = myBlendspaceType == BlendspaceType::OneDimensional ? "1D" : "2D";
		json["AnimatedMeshPath"] = myMeshPath;

		json["HorizontalAxis"]["Name"] = myHorizontalAxis.Name;
		json["HorizontalAxis"]["Min"] = myHorizontalAxis.Min;
		json["HorizontalAxis"]["Max"] = myHorizontalAxis.Max;

		json["VerticalAxis"]["Name"] = myVerticalAxis.Name;
		json["VerticalAxis"]["Min"] = myVerticalAxis.Min;
		json["VerticalAxis"]["Max"] = myVerticalAxis.Max;

		auto& entriesJson = json["Entries"];

		for (const BlendSpaceEntry& entry : myEntries)
		{
			nlohmann::json entryJson;
			entryJson["AnimationPath"] = entry.Animation->GetPath();
			entryJson["Speed"] = entry.Speed;
			entryJson["HorizontalAxisPosition"] = entry.HorizontalAxisPosition;
			entryJson["VerticalAxisPosition"] = entry.VerticalAxisPosition;
			entriesJson.push_back(entryJson);
		}

		if (myBlendspaceType == BlendspaceType::TwoDimensional)
		{
			if (myEntries.size() > 2)
			{
				Triangulate2D();
				//save triantgles
				auto& trianglesJson = json["Triangles"];
				for (int i = 0; i < myTriangles.size(); i++)
				{
					auto& triangle = trianglesJson[i];

					triangle[0] = myTriangles[i][0];
					triangle[1] = myTriangles[i][1];
					triangle[2] = myTriangles[i][2];
				}
			}
		}

		std::ofstream file(aPath);
		file << std::setw(4) << json;
		file.close();

		ResourceCache::UnloadAsset(aPath);
	}
	void BlendSpace::Triangulate2D()
	{
		myTriangles.clear();
		//we cannot triangulate a 2D blendspace with less than 3 entries
		if (myEntries.size() < 3)
		{
			LOGERROR("Cannot triangulate a blendspace with less than 3 entries");
			return;
		}

		// we can skip the calculation if we have 3 entries since that is a triangle
		if (myEntries.size() == 3)
		{
			myTriangles.push_back({ 0,1,2 });
			return;
		}

		//this is commented out until i come up with a solution for the editor not to get the wrong index when moving a entry and triangulating, fix if need performance
		// Sort the entries lexicographically (first by horizontal axis position, then by vertical axis position)
		/*std::sort(myEntries.begin(), myEntries.end(),
			[](const BlendSpaceEntry& a, const BlendSpaceEntry& b)
			{
				if (a.HorizontalAxisPosition != b.HorizontalAxisPosition)
				{
					return a.HorizontalAxisPosition < b.HorizontalAxisPosition;
				}
				return a.VerticalAxisPosition < b.VerticalAxisPosition;
			});*/

		struct Triangle
		{
			int Indices[3];
			Utils::Vector2f Positions[3];

			Triangle(int a, int b, int c, const Utils::Vector2f& aPos, const Utils::Vector2f& bPos, const Utils::Vector2f& cPos)
			{
				Indices[0] = a;
				Indices[1] = b;
				Indices[2] = c;
				Positions[0] = aPos;
				Positions[1] = bPos;
				Positions[2] = cPos;
			}

		};

		struct Edge
		{
			int Index1;
			int Index2;
			Utils::Vector2f Position1;
			Utils::Vector2f Position2;
			bool operator== (const Edge& aOther) const
			{
				return ((Index1 == aOther.Index1 && Index2 == aOther.Index2) || (Index1 == aOther.Index2 && Index2 == aOther.Index1)) &&
					((Position1 == aOther.Position1 && Position2 == aOther.Position2) || (Position1 == aOther.Position2 && Position2 == aOther.Position1));
			}
		};

		std::vector<Triangle> triangles;

		//supra triangle encompassing all points
		Utils::Vector2f size = { myHorizontalAxis.Max - myHorizontalAxis.Min,myVerticalAxis.Max - myVerticalAxis.Min };
		Utils::Vector2f center = Utils::Vector2f(myHorizontalAxis.Min, myVerticalAxis.Min) + size / 2.f;
		triangles.push_back(Triangle(-1, -1, -1,
			{ center.x, center.y + size.y * 100.f },
			{ center.x - size.x * 100.f,center.y - size.y * 100.f },
			{ center.x + size.x * 100.f	  , center.y - size.y * 100.f }));


		for (int entryIndex = 0; entryIndex < myEntries.size(); entryIndex++)
		{
			Utils::Vector2f point = { myEntries[entryIndex].HorizontalAxisPosition, myEntries[entryIndex].VerticalAxisPosition };

			//find all triangles whos circumcircle contains the point
			std::vector<int> badTriangles;

			for (int j = 0; j < triangles.size(); j++)
			{
				if (IsPointInTriangleCircumCircle(point, triangles[j].Positions[0], triangles[j].Positions[1], triangles[j].Positions[2]))
				{
					badTriangles.push_back(j);
				}
			}


			//collect all edges that only exist once in the bad triangles
			std::vector<Edge> polygon;
			for (int firstBadTriangleIndex = 0; firstBadTriangleIndex < badTriangles.size(); firstBadTriangleIndex++)
			{
				for (int firstTriangleVertexIndex = 0; firstTriangleVertexIndex < 3; firstTriangleVertexIndex++)
				{
					auto& triangle = triangles[badTriangles[firstBadTriangleIndex]];
					Edge firstTrangleEdge = { triangle.Indices[firstTriangleVertexIndex],
											  triangle.Indices[(firstTriangleVertexIndex + 1) % 3],
											  triangle.Positions[firstTriangleVertexIndex],
											  triangle.Positions[(firstTriangleVertexIndex + 1) % 3] };

					bool isSharedEdge = false;
					for (int secondBadTriangleIndex = 0; secondBadTriangleIndex < badTriangles.size(); secondBadTriangleIndex++)
					{
						if (secondBadTriangleIndex == firstBadTriangleIndex)
						{
							continue;
						}
						for (int secondTriangleVertexIndex = 0; secondTriangleVertexIndex < 3; secondTriangleVertexIndex++)
						{
							auto& secondTriangle = triangles[badTriangles[secondBadTriangleIndex]];
							Edge secondTriangleEdge = { secondTriangle.Indices[secondTriangleVertexIndex],
														secondTriangle.Indices[(secondTriangleVertexIndex + 1) % 3] ,
														secondTriangle.Positions[secondTriangleVertexIndex],
														secondTriangle.Positions[(secondTriangleVertexIndex + 1) % 3] };

							if (firstTrangleEdge == secondTriangleEdge)
							{
								isSharedEdge = true;
								break;
							}
						}
						if (isSharedEdge)
						{
							break;
						}
					}

					if (!isSharedEdge)
					{
						polygon.push_back(firstTrangleEdge);
					}
				}
			}

			//remove the bad triangles
			for (int i = badTriangles.size() - 1; i >= 0; i--)
			{
				triangles.erase(triangles.begin() + badTriangles[i]);
			}

			//add the new triangles
			for (int i = 0; i < polygon.size(); i++)
			{
				triangles.push_back(Triangle(polygon[i].Index1, polygon[i].Index2, entryIndex, polygon[i].Position1, polygon[i].Position2, point));
			}
		}

		//remove all triangles that have a entry index of -1, aka the triangles with connections to the supra triangle
		for (int i = triangles.size() - 1; i >= 0; i--)
		{
			if (triangles[i].Indices[0] == -1 || triangles[i].Indices[1] == -1 || triangles[i].Indices[2] == -1)
			{
				triangles.erase(triangles.begin() + i);
			}
		}

		if (triangles.size() == 0)
		{
			LOGERROR("No triangles generated in blendspace");
			return;
		}

		//loop through all triangles and copy over to myTriangles
		for (int i = 0; i < triangles.size(); i++)
		{
			myTriangles.push_back({ triangles[i].Indices[0], triangles[i].Indices[1], triangles[i].Indices[2] });
		}
		LOGINFO("Triangle Count: {}", myTriangles.size());
	}

	std::optional<Frame> BlendSpace::Sample2D(float aTime, const Utils::Vector2f& aPosition)
	{
		//LOGINFO("x: {}, y: {}", aPosition.x, aPosition.y);
		if (myEntries.size() == 0)
		{
			return std::nullopt;
		}
		else if (myEntries.size() == 1)
		{
			return myEntries[0].Animation->GetFrame(aTime / myEntries[0].Speed, true);
		}
		else if (myEntries.size() == 2)
		{
			const auto& firstEntry = myEntries.front();
			const auto& secondEntry = myEntries.back();
			const Utils::Vector2f firstEntryPosition = { firstEntry.HorizontalAxisPosition, firstEntry.VerticalAxisPosition };
			const Utils::Vector2f secondEntryPosition = { secondEntry.HorizontalAxisPosition, secondEntry.VerticalAxisPosition };

			const float distanceBetween = (firstEntryPosition - secondEntryPosition).Length();
			const float distanceToFirst = (firstEntryPosition - aPosition).Length();

			const float firstEntryWeight = distanceToFirst / distanceBetween;
			const float secondEntryWeight = 1.0f - firstEntryWeight;

			const float firstEntryDuration = firstEntry.Animation->GetDuration() / firstEntry.Speed;
			const float secondEntryDuration = secondEntry.Animation->GetDuration() / secondEntry.Speed;

			float firstEntrySyncModifier = 1.0f;
			float secondEntrySyncModifier = 1.0f;
			//if the first entry has a higher weight, we need to sync the second entry to the first one
			if (firstEntryWeight > secondEntryWeight)
			{
				secondEntrySyncModifier = GetDurationSyncMultiplier(firstEntryDuration, secondEntryDuration);
			}
			else
			{
				firstEntrySyncModifier = GetDurationSyncMultiplier(secondEntryDuration, firstEntryDuration);
			}

			auto firstEntryFrame = firstEntry.Animation->GetFrame(aTime * firstEntry.Speed / firstEntrySyncModifier, true);
			auto secondEntryFrame = secondEntry.Animation->GetFrame(aTime * secondEntry.Speed / secondEntrySyncModifier, true);

			secondEntryFrame.BlendWith(firstEntryFrame, firstEntryWeight);
			return secondEntryFrame;
		}
		else if (myTriangles.empty())
		{
			LOGERROR("BlendSpace::Sample2D: No triangles to sample from!");
			return  myEntries[0].Animation->GetFrame(aTime / myEntries[0].Speed, true);


		}

		//find what triangle the point is in
		auto triangleIndexOptional = GetTriangleThatContainsPoint(aPosition);
		int triangleIndex = 0;
		auto posToCheck = aPosition;
		if (triangleIndexOptional.has_value())
		{
			triangleIndex = triangleIndexOptional.value();
		}
		else
		{
			posToCheck = TranslatePositionIntoNearestTriangle(aPosition);
			triangleIndexOptional = GetTriangleThatContainsPoint(posToCheck);
			if (!triangleIndexOptional.has_value())
			{
				LOGERROR("Blendspace::Sample2D: After mapping the point to the inside of a triangle, the point doesnt seem to be inside a triangle");
			}
			else
			{
				triangleIndex = triangleIndexOptional.value();
			}
		}
		auto& triangle = myTriangles[triangleIndex];
		auto& vertex0Entry = myEntries[triangle[0]];
		auto& vertex1Entry = myEntries[triangle[1]];
		auto& vertex2Entry = myEntries[triangle[2]];
		auto weights = CalculateWeightsInTriangle(posToCheck, vertex0Entry, vertex1Entry, vertex2Entry);

		//find the weights for each vertex
		float& entry0Weight = weights[0];
		float& entry1Weight = weights[1];
		float& entry2Weight = weights[2];


		//select the animation with the highest weight as the main animation
		int mainAnimationIndex = 0;
		float mainAnimationWeight = entry0Weight;
		if (entry1Weight > mainAnimationWeight)
		{
			mainAnimationIndex = 1;
			mainAnimationWeight = entry1Weight;
		}
		if (entry2Weight > mainAnimationWeight)
		{
			mainAnimationIndex = 2;
			mainAnimationWeight = entry2Weight;
		}

		//sync the other animations to the main animation
		float entry0Duration = vertex0Entry.Animation->GetDuration() / vertex0Entry.Speed;
		float entry1Duration = vertex1Entry.Animation->GetDuration() / vertex1Entry.Speed;
		float entry2Duration = vertex2Entry.Animation->GetDuration() / vertex2Entry.Speed;

		float entry0SyncModifier = 1.0f;
		float entry1SyncModifier = 1.0f;
		float entry2SyncModifier = 1.0f;

		if (mainAnimationIndex == 0)
		{
			entry1SyncModifier = GetDurationSyncMultiplier(entry0Duration, entry1Duration);
			entry2SyncModifier = GetDurationSyncMultiplier(entry0Duration, entry2Duration);
		}
		else if (mainAnimationIndex == 1)
		{
			entry0SyncModifier = GetDurationSyncMultiplier(entry1Duration, entry0Duration);
			entry2SyncModifier = GetDurationSyncMultiplier(entry1Duration, entry2Duration);
		}
		else if (mainAnimationIndex == 2)
		{
			entry0SyncModifier = GetDurationSyncMultiplier(entry2Duration, entry0Duration);
			entry1SyncModifier = GetDurationSyncMultiplier(entry2Duration, entry1Duration);
		}
		//

		//get the frame at the current synced time for each animation
		auto entry0Frame = vertex0Entry.Animation->GetFrame(aTime * vertex0Entry.Speed / entry0SyncModifier, true);
		auto entry1Frame = vertex1Entry.Animation->GetFrame(aTime * vertex1Entry.Speed / entry1SyncModifier, true);
		auto entry2Frame = vertex2Entry.Animation->GetFrame(aTime * vertex2Entry.Speed / entry2SyncModifier, true);

		if (mainAnimationIndex == 0)
		{
			entry0Frame.BlendWith(entry1Frame, entry1Weight);
			entry0Frame.BlendWith(entry2Frame, entry2Weight);
			return entry0Frame;
		}
		else if (mainAnimationIndex == 1)
		{
			entry1Frame.BlendWith(entry0Frame, entry0Weight);
			entry1Frame.BlendWith(entry2Frame, entry2Weight);
			return entry1Frame;
		}
		else if (mainAnimationIndex == 2)
		{
			entry2Frame.BlendWith(entry0Frame, entry0Weight);
			entry2Frame.BlendWith(entry1Frame, entry1Weight);
			return entry2Frame;
		}




		//return the blended frame
		return std::nullopt;
	}
	float BlendSpace::GetDuration2D(const Utils::Vector2f& aPosition) const
	{
		if (myEntries.empty())
		{
			return 0.0f;
		}
		else if (myEntries.size() == 1)
		{
			return myEntries.front().Animation->GetDuration() / myEntries.front().Speed;
		}
		else if (myEntries.size() == 2)
		{
			//find the closest entry to the point and return that entry's duration
			Utils::Vector2f firstEntryPosition = { myEntries.front().HorizontalAxisPosition, myEntries.front().VerticalAxisPosition };
			Utils::Vector2f secondEntryPosition = { myEntries.back().HorizontalAxisPosition, myEntries.back().VerticalAxisPosition };

			auto firstEntryDistanceSquared = (firstEntryPosition - aPosition).Length();
			auto secondEntryDistanceSquared = (secondEntryPosition - aPosition).Length();

			if (firstEntryDistanceSquared > secondEntryDistanceSquared)
			{
				return myEntries.front().Animation->GetDuration() / myEntries.front().Speed;
			}
			else
			{
				return myEntries.back().Animation->GetDuration() / myEntries.back().Speed;
			}
		}
		else if (myTriangles.empty())
		{
			LOGERROR("BlendSpace::GetDuration2D: NO TRIANGLES TO GET DURATION FROM FROM");
			return myEntries.front().Animation->GetDuration() / myEntries.front().Speed;
		}
		//find what triangle the point is in
		auto triangleIndexOptional = GetTriangleThatContainsPoint(aPosition);
		int triangleIndex = 0;
		if (triangleIndexOptional.has_value())
		{
			triangleIndex = triangleIndexOptional.value();
		}
		auto& triangle = myTriangles[triangleIndex];
		auto& vertex0Entry = myEntries[triangle[0]];
		auto& vertex1Entry = myEntries[triangle[1]];
		auto& vertex2Entry = myEntries[triangle[2]];
		auto weights = CalculateWeightsInTriangle(aPosition, vertex0Entry, vertex1Entry, vertex2Entry);

		//find the weights for each vertex
		float entry0Weight = weights[0];
		float entry1Weight = weights[1];
		float entry2Weight = weights[2];

		//find the largest wight
		int entryIndex = 0;
		float mainAnimationWeight = entry0Weight;
		if (entry1Weight > mainAnimationWeight)
		{
			entryIndex = 1;
			mainAnimationWeight = entry1Weight;
		}
		if (entry2Weight > mainAnimationWeight)
		{
			entryIndex = 2;
			mainAnimationWeight = entry2Weight;
		}

		//find the duration of the longest animation
		return myEntries[triangle[entryIndex]].Animation->GetDuration();
	}
	Ref<Animation> BlendSpace::GetAnyAnimation() const
	{

		if (myEntries.empty())
		{
			LOGERROR(__FUNCTION__": BlendSpace has no entries, returning null");
			return nullptr;
		}
		return myEntries.front().Animation;
	}
	bool BlendSpace::Empty()
	{
		return myEntries.empty();
	}
	float BlendSpace::GetDuration(const Utils::Vector2f& aPosition) const
	{
		if (myBlendspaceType == BlendspaceType::TwoDimensional)
		{
			return GetDuration2D(aPosition);
		}
		else if (myBlendspaceType == BlendspaceType::OneDimensional)
		{
			return GetDuration1D(aPosition.x);
		}
		return 0.0f;
	}
	bool BlendSpace::IsPointInTriangleCircumCircle(const Utils::Vector2f& aPoint,
		const Utils::Vector2f& aTrianglePoint1,
		const Utils::Vector2f& aTrianglePoint2,
		const Utils::Vector2f& aTrianglePoint3) const
	{
		auto& a = aTrianglePoint1;
		auto& b = aTrianglePoint2;
		auto& c = aTrianglePoint3;
		auto& p = aPoint;

		//calculate the circumcenter
		Utils::Vector2f circumcenter = GetTriangleCircumferenceCenter(a, b, c);

		//calculate the circumradius
		float circumradius = (a - circumcenter).Length();

		//calculate the distance from the point to the circumcenter
		float distance = (p - circumcenter).Length();

		//if the distance is less than the circumradius, the point is inside the triangle
		return distance < circumradius;

	}
	//using jarvs march
	std::vector<int> BlendSpace::CalculateConvexHull(const std::vector<BlendSpaceEntry>& aPoints) const
	{
		//find the leftmost point 
		int leftmostPoint = 0;
		for (int i = 0; i < aPoints.size(); i++)
		{
			if (aPoints[i].HorizontalAxisPosition < aPoints[leftmostPoint].HorizontalAxisPosition)
			{
				leftmostPoint = i;
			}
		}

		//start at the leftmost point and move clockwise
		int currentPoint = leftmostPoint;
		std::vector<int> convexHull;

		do
		{
			convexHull.push_back(currentPoint);
			int nextPoint = (currentPoint + 1) % aPoints.size();
			for (int i = 0; i < aPoints.size(); i++)
			{
				const auto& currentEntry = aPoints[currentPoint];
				const auto& nextEntry = aPoints[nextPoint];
				const auto& entry = aPoints[i];
				const Utils::Vector2f currentVector = { currentEntry.HorizontalAxisPosition, currentEntry.VerticalAxisPosition };
				const Utils::Vector2f nextVector = { nextEntry.HorizontalAxisPosition, nextEntry.VerticalAxisPosition };
				const Utils::Vector2f entryVector = { entry.HorizontalAxisPosition, entry.VerticalAxisPosition };
				//if the cross product is positive then the point is on the left side of the line
				if ((nextVector - currentVector).Cross(entryVector - currentVector) > 0)
				{
					nextPoint = i;
				}
			}
			currentPoint = nextPoint;
		} while (currentPoint != leftmostPoint);

		return convexHull;
	}

	void BlendSpace::LineFromPoints(const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2, float& aOutA, float& aOutB, float& aOutC) const
	{
		aOutA = aPoint2.y - aPoint1.y;
		aOutB = aPoint1.x - aPoint2.x;
		aOutC = aOutA * aPoint1.x + aOutB * aPoint1.y;
	}

	void BlendSpace::PerpendicularBisectorFromLine(const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2, float& aOutA, float& aOutB, float& aOutC) const
	{
		Utils::Vector2f midPoint = (aPoint1 + aPoint2) / 2.f;
		Utils::Vector2f perpendicularVector = { aPoint2.y - aPoint1.y, aPoint1.x - aPoint2.x };
		aOutC = -aOutB * midPoint.x + aOutA * midPoint.y;
		float temp = aOutA;
		aOutA = -aOutB;
		aOutB = temp;
	}

	float Sign(const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2, const Utils::Vector2f& aPoint3)
	{
		return (aPoint1.x - aPoint3.x) * (aPoint2.y - aPoint3.y) - (aPoint2.x - aPoint3.x) * (aPoint1.y - aPoint3.y);
	}
	bool BlendSpace::IsPointInsideTriangle(const Utils::Vector2f& aPoint,
		const Utils::Vector2f& aTrianglePoint1,
		const Utils::Vector2f& aTrianglePoint2,
		const Utils::Vector2f& aTrianglePoint3) const
	{

		float d1, d2, d3;

		d1 = Sign(aPoint, aTrianglePoint1, aTrianglePoint2);
		d2 = Sign(aPoint, aTrianglePoint2, aTrianglePoint3);
		d3 = Sign(aPoint, aTrianglePoint3, aTrianglePoint1);

		const float tolerance = 1.f;
		if ((d1 >= 0 && d2 >= 0 && d3 >= 0) ||
			(d1 >= tolerance && d2 >= 0 && d3 >= 0) ||
			(d1 >= 0 && d2 >= tolerance && d3 >= 0) ||
			(d1 >= 0 && d2 >= 0 && d3 >= tolerance))
		{
			return true; // inside triangle
		}



		return false;
	}

	Utils::Vector2f BlendSpace::IntersectionOfLines(float aPointA1, float aPointB1, float aPointC1,
		float aPointA2, float aPointB2, float aPointC2) const
	{
		float determinant = aPointA1 * aPointB2 - aPointA2 * aPointB1;

		if (determinant == 0)
		{
			return { FLT_MAX, FLT_MAX };
		}
		else
		{
			float x = (aPointB2 * aPointC1 - aPointB1 * aPointC2) / determinant;
			float y = (aPointA1 * aPointC2 - aPointA2 * aPointC1) / determinant;
			return { x, y };
		}
	}
	Utils::Vector2f BlendSpace::GetTriangleCircumferenceCenter(const Utils::Vector2f& aTrianglePoint1, const Utils::Vector2f& aTrianglePoint2, const Utils::Vector2f& aTrianglePoint3) const
	{

		Utils::Vector2f circumcenter;
		const auto& a = aTrianglePoint1;
		const auto& b = aTrianglePoint2;
		const auto& c = aTrianglePoint3;
		const auto d = (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) * 2;
		const auto dDiv = 1.f / d;

		circumcenter.x = dDiv * ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) + (c.x * c.x + c.y * c.y) * (a.y - b.y));
		circumcenter.y = dDiv * ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) + (c.x * c.x + c.y * c.y) * (b.x - a.x));

		return circumcenter;
	}

	float BlendSpace::GetDurationSyncMultiplier(float aToDuration, float aFromDuration) const
	{
		/*auto mod = fmod(aToDuration, aFromDuration);
		int count = aToDuration / aFromDuration;
		auto desiredTime = aToDuration;
		if (count >= 1)
		{
			auto diff = mod / count;
			desiredTime = aFromDuration + diff;
		}*/

		return aToDuration / aFromDuration;
	}

	std::array<float, 3> BlendSpace::CalculateWeightsInTriangle(Utils::Vector2f aPosition, const BlendSpaceEntry& aEntry0, const BlendSpaceEntry& aEntry1, const BlendSpaceEntry& aEntry2) const
	{
		Utils::Vector2f vertex0 = { aEntry0.HorizontalAxisPosition, aEntry0.VerticalAxisPosition };
		Utils::Vector2f vertex1 = { aEntry1.HorizontalAxisPosition, aEntry1.VerticalAxisPosition };
		Utils::Vector2f vertex2 = { aEntry2.HorizontalAxisPosition, aEntry2.VerticalAxisPosition };

		//use barycentric coordinates to calculate weights
		auto barycentricCoords = CalculateBarycentricCoordinates(aPosition, vertex0, vertex1, vertex2);

		//this configuration is intended
		float weight0 = barycentricCoords.y;
		float weight1 = barycentricCoords.z;
		float weight2 = barycentricCoords.x;


		return { weight0, weight1, weight2 };


		//float vertex0Distance = (vertex0 - aPosition).LengthSqr();
		//float vertex1Distance = (vertex1 - aPosition).LengthSqr();
		//float vertex2Distance = (vertex2 - aPosition).LengthSqr();

		////find the weights for each vertex
		//float entry0Weight = 1 / vertex0Distance;
		//float entry1Weight = 1 / vertex1Distance;
		//float entry2Weight = 1 / vertex2Distance;
		//float sum = entry0Weight + entry1Weight + entry2Weight;
		//entry0Weight /= sum;
		//entry1Weight /= sum;
		//entry2Weight /= sum;
		//return { entry0Weight, entry1Weight, entry2Weight };
	}

	std::optional<int> BlendSpace::GetTriangleThatContainsPoint(const Utils::Vector2f& aPosition) const
	{
		int triangleIndex = -1;
		for (int i = 0; i < myTriangles.size(); i++)
		{
			auto& vertex0Entry = myEntries[myTriangles[i][0]];
			auto& vertex1Entry = myEntries[myTriangles[i][1]];
			auto& vertex2Entry = myEntries[myTriangles[i][2]];
			if (IsPointInsideTriangle(aPosition,
				{ vertex0Entry.HorizontalAxisPosition, vertex0Entry.VerticalAxisPosition },
				{ vertex1Entry.HorizontalAxisPosition, vertex1Entry.VerticalAxisPosition },
				{ vertex2Entry.HorizontalAxisPosition, vertex2Entry.VerticalAxisPosition }))
			{
				triangleIndex = i;
				break;
			}
		}
		if (triangleIndex == -1)
		{
			return {};
		}
		return triangleIndex;
	}

	Utils::Vector2f BlendSpace::TranslatePositionIntoNearestTriangle(const Utils::Vector2f& aPosition) const
	{
		if (GetTriangleThatContainsPoint(aPosition).has_value())
		{
			return aPosition;
		}
		std::vector<std::array<int, 2>> lines;
		for (int i = 0; i < myTriangles.size(); i++)
		{
			lines.push_back({ myTriangles[i][0], myTriangles[i][1] });
			lines.push_back({ myTriangles[i][1], myTriangles[i][2] });
			lines.push_back({ myTriangles[i][2], myTriangles[i][0] });
		}
		//remove all pairs that exist more than once
		for (int i = 0; i < lines.size(); i++)
		{
			for (int j = i + 1; j < lines.size(); j++)
			{
				if (lines[i][0] == lines[j][0] && lines[i][1] == lines[j][1] ||
					lines[i][0] == lines[j][1] && lines[i][1] == lines[j][0])
				{
					lines.erase(lines.begin() + j);
					lines.erase(lines.begin() + i);
					i--;
					break;
				}

			}
		}

		int targetPairIndex = -1;
		//for each pair, find the connecting pairs
		for (int i = 0; i < lines.size(); i++)
		{
			const auto& line = lines[i];

			//find what triangle the point belongs to
			int triangleIndex = -1;
			for (int j = 0; j < myTriangles.size(); j++)
			{
				if (std::find(myTriangles[j].begin(), myTriangles[j].end(), line[0]) != myTriangles[j].end() &&
					std::find(myTriangles[j].begin(), myTriangles[j].end(), line[1]) != myTriangles[j].end())
				{
					triangleIndex = j;
					break;
				}
			}
			auto& triangle = myTriangles[triangleIndex];
			//find the other point in the triangle
			int otherPoint = -1;
			for (int j = 0; j < 3; j++)
			{
				if (triangle[j] != line[0] && triangle[j] != line[1])
				{
					otherPoint = triangle[j];
					break;
				}
			}
			int connectingLineA[2] = { line[0], otherPoint };
			int connectingLineB[2] = { otherPoint, line[1] };

			//get the normals
			const Utils::Vector2f lineAPos0 = { myEntries[connectingLineA[0]].HorizontalAxisPosition, myEntries[connectingLineA[0]].VerticalAxisPosition };
			const Utils::Vector2f lineAPos1 = { myEntries[connectingLineA[1]].HorizontalAxisPosition, myEntries[connectingLineA[1]].VerticalAxisPosition };
			const auto vectorA = lineAPos1 - lineAPos0;

			const Utils::Vector2f lineBPos0 = { myEntries[connectingLineB[0]].HorizontalAxisPosition, myEntries[connectingLineB[0]].VerticalAxisPosition };
			const Utils::Vector2f lineBPos1 = { myEntries[connectingLineB[1]].HorizontalAxisPosition, myEntries[connectingLineB[1]].VerticalAxisPosition };
			const auto vectorB = lineBPos1 - lineBPos0;


			const auto normalA = Utils::Vector2f(vectorA.y, -vectorA.x).GetNormalized();
			const auto normalB = Utils::Vector2f(vectorB.y, -vectorB.x).GetNormalized();

			if (normalA.Dot(aPosition - lineAPos0) > 0 && normalB.Dot(aPosition - lineBPos1) > 0)
			{
				targetPairIndex = i;
				break;
			}
		}
		if (targetPairIndex == -1)
		{
			return aPosition;
		}

		auto& targetLine = lines[targetPairIndex];
		Utils::Vector2f linePositions[2] = {
			{myEntries[targetLine[0]].HorizontalAxisPosition, myEntries[targetLine[0]].VerticalAxisPosition},
			{myEntries[targetLine[1]].HorizontalAxisPosition, myEntries[targetLine[1]].VerticalAxisPosition}
		};

		//project aPosition onto the line
		auto lineVec = linePositions[1] - linePositions[0];
		auto posToStart = aPosition - linePositions[0];
		auto param = posToStart.Dot(lineVec) / lineVec.Dot(lineVec);
		auto proj = linePositions[0] + lineVec * param;
		proj += Utils::Vector2f(-lineVec.y, lineVec.x).GetNormalized() * 0.1f;
		return proj;
	}

	Utils::Vector3f BlendSpace::CalculateBarycentricCoordinates(const Utils::Vector2f& aPosition, const Utils::Vector2f& aPoint0, const Utils::Vector2f& aPoint1, const Utils::Vector2f& aPoint2) const
	{
		//https://en.wikipedia.org/wiki/Barycentric_coordinate_system

		//calculate the area of the triangle
		//area = 0.5 * abs( (Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By)) )		
		float area = 0.5f * abs((aPoint0.x * (aPoint1.y - aPoint2.y) + aPoint1.x * (aPoint2.y - aPoint0.y) + aPoint2.x * (aPoint0.y - aPoint1.y)));

		//area_PAB = 0.5 * abs( (Ax * (Py - By) + Px * (By - Ay) + Bx * (Ay - Py)) )
		//area_PBC = 0.5 * abs((Bx * (Py - Cy) + Px * (Cy - By) + Cx * (By - Py)))
		//area_PCA = 0.5 * abs((Cx * (Py - Ay) + Px * (Ay - Cy) + Ax * (Cy - Py)))
		float area_PAB = 0.5f * abs((aPoint0.x * (aPosition.y - aPoint1.y) + aPosition.x * (aPoint1.y - aPoint0.y) + aPoint1.x * (aPoint0.y - aPosition.y)));
		float area_PBC = 0.5f * abs((aPoint1.x * (aPosition.y - aPoint2.y) + aPosition.x * (aPoint2.y - aPoint1.y) + aPoint2.x * (aPoint1.y - aPosition.y)));
		float area_PCA = 0.5f * abs((aPoint2.x * (aPosition.y - aPoint0.y) + aPosition.x * (aPoint0.y - aPoint2.y) + aPoint0.x * (aPoint2.y - aPosition.y)));
		return { area_PAB / area, area_PBC / area, area_PCA / area };
	}

}

