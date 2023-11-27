#pragma once

#include "Vector3.hpp"
#include "AABB3D.hpp"
#include "Sphere.hpp"

namespace Utils
{
	template <class T>
	class Plane
	{
	public:
		// Default constructor.
		Plane();

		// Constructor taking three points where the normal is (aPoint1 - aPoint0) x (aPoint2 -aPoint0).
		Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);

		// Constructor taking a point and a normal.
		Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal);

		// Init the plane with three points, the same as the constructor above.
		void InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);

		// Init the plane with a point and a normal, the same as the constructor above.
		void InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal);

		// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
		bool IsInside(const Vector3<T>& aPosition) const;

		bool IsInside(const AABB3D<T>& aAABB) const;

		bool IsInside(const Sphere<T>& aSphere) const;

		// Returns the origin of the plane.
		const Vector3<T>& GetOrigin() const;

		// Returns the normal of the plane.
		const Vector3<T>& GetNormal() const;

	private:
		Vector3<T> myOrigin;
		Vector3<T> myNormal;
	};

	template <class T>
	Plane<T>::Plane()
	{
		myOrigin = Vector3<T>(0, 0, 0);
		myNormal = Vector3<T>(0, 0, 0);
	}

	template <class T>
	Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myOrigin = aPoint0;
		myNormal = (aPoint1 - aPoint0).Cross(aPoint2 - aPoint0);
		myNormal.Normalize();
	}

	template <class T>
	Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal)
	{
		myOrigin = aPoint0;
		myNormal = aNormal;
		myNormal.Normalize();
	}

	template <class T>
	void Plane<T>::InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myOrigin = aPoint0;
		myNormal = (aPoint1 - aPoint0).Cross(aPoint2 - aPoint0);
		myNormal.Normalize();
	}

	template <class T>
	void Plane<T>::InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal)
	{
		myOrigin = aPoint;
		myNormal = aNormal;
		myNormal.Normalize();
	}

	template <class T>
	bool Plane<T>::IsInside(const Vector3<T>& aPosition) const
	{
		return (myNormal.Dot(aPosition - myOrigin) <= 0);
	}

	template<class T>
	bool Plane<T>::IsInside(const AABB3D<T>& aAABB) const
	{
		Vector3<T> closestPoint = aAABB.GetFurthestCornerInDir(-myNormal);
		return IsInside(closestPoint);
	}

	template<class T>
	bool Plane<T>::IsInside(const Sphere<T>& aSphere) const
	{
		Vector3<T> closestPoint = aSphere.GetOrigin() - myNormal * aSphere.GetRadius();
		return IsInside(closestPoint);
	}

	template <class T>
	const Vector3<T>& Plane<T>::GetOrigin() const
	{
		return myOrigin;
	}

	template <class T>
	const Vector3<T>& Plane<T>::GetNormal() const
	{
		return myNormal;
	}
}
