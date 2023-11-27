#pragma once
#include "Vector3.hpp"
#include "Line.hpp"
#include "LineVolume.hpp"
#include "Vector2.hpp"

namespace Utils
{
	class BoundingPlane
	{
	public:
		BoundingPlane() = default;

		//Contructs a BoundingPlane with the normal pointing in clockwise winding order of the 3 points
		BoundingPlane(const Vector3f& aFirstPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint);

		//Contructs a BoundingPlane with the normal pointing in clockwise winding order of the 3 points
		void InitWithThreePoints(const Vector3f& aFirstPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint);

		//Returns if a point is inside of the plane surface within a given tolerance distance
		bool IsInside(const Vector3f& aPoint, float aTolerance = 0.001f) const;

		const Vector3f& GetFirstPoint() const;
		const Vector3f& GetSecondPoint() const;
		const Vector3f& GetThirdPoint() const;

		const Vector3f GetFourthPoint() const;

		const Vector3f GetNormal() const;

	private:
		Vector3f myFirstPoint;
		Vector3f mySecondPoint;
		Vector3f myThirdPoint;
	};

	inline BoundingPlane::BoundingPlane(const Vector3f& aFirstPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint)
	{
		myFirstPoint = aFirstPoint;
		mySecondPoint = aSecondPoint;
		myThirdPoint = aThirdPoint;
	}

	inline void BoundingPlane::InitWithThreePoints(const Vector3f& aFirstPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint)
	{
		myFirstPoint = aFirstPoint;
		mySecondPoint = aSecondPoint;
		myThirdPoint = aThirdPoint;
	}

	inline bool BoundingPlane::IsInside(const Vector3f& aPoint, float aTolerance) const
	{
		const Vector3f planeToPoint = aPoint - myFirstPoint;
		const float positionalDot = planeToPoint.Dot(GetNormal());

		// Check if point literally originates from inside the plane
		if (positionalDot > -aTolerance && positionalDot < aTolerance)
		{
			//Now check if inside plane projected on 2d plane
			Vector2f A = { myFirstPoint.x, myFirstPoint.z };
			Vector2f B = { mySecondPoint.x, mySecondPoint.z };
			Vector2f C = { myThirdPoint.x, myThirdPoint.z };

			Vector3f fourth = GetFourthPoint();
			Vector2f D = { fourth.x, fourth.z };

			LineVolume<float> plane2d;
			Line AB(A, B);
			Line BD(B, D);
			Line DC(D, C);
			Line CA(C, A);
			plane2d.AddLine(AB);
			plane2d.AddLine(BD);
			plane2d.AddLine(DC);
			plane2d.AddLine(CA);

			Vector2f point2d(aPoint.x, aPoint.z);

			if (plane2d.IsInside(point2d))
			{
				return true;
			}
		}

		return false;
	}

	inline const Vector3f& BoundingPlane::GetFirstPoint() const
	{
		return myFirstPoint;
	}

	inline const Vector3f& BoundingPlane::GetSecondPoint() const
	{
		return mySecondPoint;
	}

	inline const Vector3f& BoundingPlane::GetThirdPoint() const
	{
		return myThirdPoint;
	}

	inline const Vector3f BoundingPlane::GetFourthPoint() const
	{
		//return myFirstPoint + myThirdPoint - mySecondPoint;
		//myFirstPoint + (mySecondPoint - myFirstPoint) + (myThirdPoint - myFirstPoint);
		return mySecondPoint + myThirdPoint - myFirstPoint;
	}

	inline const Vector3f BoundingPlane::GetNormal() const
	{
		return (mySecondPoint - myFirstPoint).Cross(myThirdPoint - myFirstPoint).GetNormalized();
	}
}
