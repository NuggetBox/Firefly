#pragma once
#include "AABB3D.hpp"
#include "Plane.hpp"
#include "BoundingPlane.hpp"
#include "Polygon.hpp"
#include "Sphere.hpp"
#include "Ray.hpp"
#include "Utils/UtilityFunctions.hpp"

namespace Utils
{
	// If the ray is parallel to the plane, aOutIntersectionPoint remains unchanged. If
	// the ray is in the plane, true is returned, if not, false is returned. If the ray
	// isn't parallel to the plane, the intersection point is stored in
	// aOutIntersectionPoint and true returned.
	template <class T>
	bool IntersectionPlaneRay(const Plane<T>& aPlane, const Ray<T>& aRay, Vector3<T>& aOutIntersectionPoint)
	{
		const T tolerance = 0.000001;

		const Vector3<T> planeToRay = aRay.GetOrigin() - aPlane.GetOrigin();
		const T positionalDot = planeToRay.Dot(aPlane.GetNormal());

		// Check if ray origin literally originates from inside the plane
		if (positionalDot > -tolerance && positionalDot < tolerance)
		{
			aOutIntersectionPoint = aRay.GetOrigin();
			return true;
		}

		const T directionDot = aPlane.GetNormal().Dot(aRay.GetDirection());

		// if dot product of direction close to 0, plane and ray diretion basically parallel, no intersection
		if (directionDot > -tolerance && directionDot < tolerance)
		{
			return false;
		}

		const bool rayAbovePlane = !aPlane.IsInside(aRay.GetOrigin());

		// ray originates from above plane & ray points down || ray originates from under plane & ray points up
		if (rayAbovePlane && directionDot < 0 || !rayAbovePlane && directionDot > 0)
		{
			aOutIntersectionPoint = aRay.GetOrigin() + aRay.GetDirection() * (-positionalDot / directionDot);
			return true;
		}

		return false;
	}

	inline bool IntersectionPolygonRay(const Polygon& aPolygon, const Ray<float>& aRay, Vector3f& aOutIntersectionPoint)
	{
		Plane<float> plane(aPolygon.GetPoint(0), aPolygon.GetPoint(aPolygon.GetNumPoints() - 1), aPolygon.GetPoint(1));

		if (IntersectionPlaneRay(plane, aRay, aOutIntersectionPoint))
		{
			if (aPolygon.IsInside(aOutIntersectionPoint))
			{
				return true;
			}
		}

		aOutIntersectionPoint = Vector3f::Zero();
		return false;
	}

	inline bool IntersectionBoundingPlaneRay(const BoundingPlane& aBoundingPlane, const Ray<float>& aRay, Vector3f& aOutIntersectionPoint)
	{
		Plane<float> plane(aBoundingPlane.GetFirstPoint(), aBoundingPlane.GetSecondPoint(), aBoundingPlane.GetThirdPoint());

		if (IntersectionPlaneRay(plane, aRay, aOutIntersectionPoint))
		{
			if (aBoundingPlane.IsInside(aOutIntersectionPoint))
			{
				return true;
			}
		}

		aOutIntersectionPoint = Vector3f::Zero();
		return false;
	}

	template <class T>
	bool IntersectionPlaneSphere(const Plane<T>& aPlane, const Sphere<T>& aSphere)
	{
		const Vector3<T> toPlane = aSphere.GetOrigin() - aPlane.GetNormal() * aSphere.GetRadius();
		return aPlane.IsInside(toPlane);
	}

	// If the ray intersects the AABB, true is returned, if not, false is returned.
	// A ray in one of the AABB's sides is counted as intersecting it.
	template <class T>
	bool IntersectionAABBRay(const AABB3D<T>& aAABB, const Ray<T>& aRay)
	{
		Vector3<T> min = aAABB.GetMin();
		Vector3<T> max = aAABB.GetMax();
		Vector3<T> dir = aRay.GetDirection();
		Vector3<T> origin = aRay.GetOrigin();

		T xMinInt = (min.x - origin.x) / dir.x;
		T xMaxInt = (max.x - origin.x) / dir.x;

		if (xMinInt > xMaxInt)
		{
			Utils::Swap(xMinInt, xMaxInt);
		}

		T yMinInt = (min.y - origin.y) / dir.y;
		T yMaxInt = (max.y - origin.y) / dir.y;

		if (yMinInt > yMaxInt)
		{
			Utils::Swap(yMinInt, yMaxInt);
		}

		if (xMinInt > yMaxInt || yMinInt > xMaxInt)
		{
			return false;
		}

		if (yMinInt > xMinInt)
		{
			xMinInt = yMinInt;
		}

		if (yMaxInt < xMaxInt)
		{
			xMaxInt = yMaxInt;
		}

		T zMinInt = (min.z - origin.z) / dir.z;
		T zMaxInt = (max.z - origin.z) / dir.z;

		if (zMinInt > zMaxInt)
		{
			Utils::Swap(zMinInt, zMaxInt);
		}

		if (xMinInt > zMaxInt || zMinInt > xMaxInt)
		{
			return false;
		}

		if (zMinInt > xMinInt)
		{
			xMinInt = zMinInt;
		}

		if (zMaxInt < xMaxInt)
		{
			xMaxInt = zMaxInt;
		}

		return true;
	}

	// If the ray intersects the sphere, true is returned, if not, false is returned.
	// A rat intersecting the surface of the sphere is considered as intersecting it.
	template <class T>
	bool IntersectionSphereRay(const Sphere<T>& aSphere, const Ray<T>& aRay)
	{
		Vector3<T> rayToSphere = aSphere.GetOrigin() - aRay.GetOrigin();

		// Ray origin inside sphere
		if (rayToSphere.LengthSqr() < aSphere.GetRadius() * aSphere.GetRadius())
		{
			return true;
		}

		T t = rayToSphere.Dot(aRay.GetDirection());

		// Ray origin is in front of sphere origin
		if (t < 0)
		{
			return false;
		}

		// Ray origin is behind sphere origin

		Vector3<T> sphereProjRay = aRay.GetOrigin() + aRay.GetDirection() * t;

		T projDistanceSquared = (sphereProjRay - aSphere.GetOrigin()).LengthSqr();

		// sphere proj is inside sphere
		if (projDistanceSquared <= aSphere.GetRadius() * aSphere.GetRadius())
		{
			return true;
		}

		return false;
	}

	inline void ClosesetPointOnLine(const std::pair<Utils::Vector3f, Utils::Vector3f>& aLine, const Utils::Vector3f& aPoint, Utils::Vector3f& aOutPoint)
	{
		Utils::Vector2f a = { aLine.first.x, aLine.first.z };
		Utils::Vector2f b = { aLine.second.x, aLine.second.z };
		Utils::Vector2f p = { aPoint.x, aPoint.z };
		Utils::Vector2f ab = b - a;
		Utils::Vector2f ap = p - a;

		float proj = ap.Dot(ab);
		float abLenSq = ab.LengthSqr();
		float d = proj / abLenSq;

		if (d <= 0)
		{
			aOutPoint = aLine.first;
		}
		else if (d >= 1)
		{
			aOutPoint = aLine.second;
		}
		else
		{
			aOutPoint = aLine.first + ((aLine.second - aLine.first) * d);
		}
	}

	// Taken from https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect comment from Gavin
	inline bool IntersectionLineLine(const Utils::Vector3f A, const Utils::Vector3f B, const Utils::Vector3f C, const Utils::Vector3f D, Utils::Vector3f& aOutPos)
	{
		float p0_x = A.x; float p0_y = A.z;
		float p1_x = B.x; float p1_y = B.z;

		float p2_x = C.x; float p2_y = C.z;
		float p3_x = D.x; float p3_y = D.z;

		float s1_x, s1_y, s2_x, s2_y;
		s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
		s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

		float s, t;
		s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			aOutPos.x = p0_x + (t * s1_x);
			aOutPos.z = p0_y + (t * s1_y);
			aOutPos.y = A.y;
			return true;
		}
		return false;


	}
	
	/*inline bool IntersectionLineLine(const Utils::Vector2f& aLine1Start, const Utils::Vector2f& aLine1End, const Utils::Vector2f& aLine2Start, const Utils::Vector2f& aLine2End, Utils::Vector2f& aOutPos)
	{
		float s1_x, s1_y, s2_x, s2_y;
		s1_x = aLine1End.x - aLine1Start.x;     s1_y = aLine1End.y - aLine1Start.y;
		s2_x = aLine2End.x - aLine2Start.x;     s2_y = aLine2End.y - aLine2Start.y;

		float s, t;
		s = (-s1_y * (aLine1Start.x - aLine2Start.x) + s1_x * (aLine1Start.y - aLine2Start.y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = (s2_x * (aLine1Start.y - aLine2Start.y) - s2_y * (aLine1Start.x - aLine2Start.x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			aOutPos.x = aLine1Start.x + (t * s1_x);
			aOutPos.y = aLine1Start.y + (t * s1_y);
			return true;
		}
		return false;
	}*/
}