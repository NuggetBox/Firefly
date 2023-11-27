#pragma once
#include "Vector3.hpp"

namespace Utils
{
	template <class T>
	class AABB3D
	{
	public:
		AABB3D();
		AABB3D(const AABB3D<T>& aAABB3D);
		// Constructor taking the positions of the minimum and maximum corners.
		AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax);
		// Init the AABB with the positions of the minimum and maximum corners, same as the constructor above.
		void InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax);
		// Returns whether a point is inside the AABB: it is inside when the point is on any
		// of the AABB's sides or inside of the AABB.
		bool IsInside(const Vector3<T>& aPosition) const;
		T GetDistanceFromAABB(const Vector3<T>& aPostion);
		Vector3<T> GetMin() const;
		Vector3<T> GetMax() const;

		Vector3<T> GetNearBottomLeft() const;
		Vector3<T> GetNearBottomRight() const;
		Vector3<T> GetFarBottomLeft() const;
		Vector3<T> GetFarBottomRight() const;
		Vector3<T> GetNearTopLeft() const;
		Vector3<T> GetNearTopRight() const;
		Vector3<T> GetFarTopLeft() const;
		Vector3<T> GetFarTopRight() const;

		Vector3<T> GetFurthestCornerInDir(const Vector3<T>& aDir) const;

	private:
		Vector3<T> myMin;
		Vector3<T> myMax;
	};

	template <class T>
	AABB3D<T>::AABB3D()
	{
	}

	template <class T>
	AABB3D<T>::AABB3D(const AABB3D<T>& aAABB3D)
	{
		myMin = aAABB3D.GetMin();
		myMax = aAABB3D.GetMax();
	}

	template <class T>
	AABB3D<T>::AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		myMin = aMin;
		myMax = aMax;
	}

	template <class T>
	void AABB3D<T>::InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		myMin = aMin;
		myMax = aMax;
	}

	template <class T>
	bool AABB3D<T>::IsInside(const Vector3<T>& aPosition) const
	{
		if (aPosition.x < myMin.x)
		{
			return false;
		}
		if (aPosition.x > myMax.x)
		{
			return false;
		}
		if (aPosition.z < myMin.z)
		{
			return false;
		}
		if (aPosition.z > myMax.z)
		{
			return false;
		}
		if (aPosition.y < myMin.y)
		{
			return false;
		}
		if (aPosition.y > myMax.y)
		{
			return false;
		}

		return true;
	}

	template<class T>
	inline T AABB3D<T>::GetDistanceFromAABB(const Vector3<T>& aPostion)
	{
		// Find the closest point on the box to the given point
		Vector3<T> closestPoint;
		closestPoint.x = std::max(myMin.x, std::min(aPostion.x, myMax.x));
		closestPoint.y = std::max(myMin.y, std::min(aPostion.y, myMax.y));
		closestPoint.z = std::max(myMin.z, std::min(aPostion.z, myMax.z));

		// Calculate the distance between the closest point and the given point
		return (aPostion - closestPoint).Length();
	}

	template <class T>
	Vector3<T> AABB3D<T>::GetMin() const
	{
		return myMin;
	}

	template <class T>
	Vector3<T> AABB3D<T>::GetMax() const
	{
		return myMax;
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetNearBottomLeft() const
	{
		return GetMin();
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetNearBottomRight() const
	{
		return Vector3<T>(myMax.x, myMin.y, myMin.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetFarBottomLeft() const
	{
		return Vector3<T>(myMin.x, myMin.y, myMax.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetFarBottomRight() const
	{
		return Vector3<T>(myMax.x, myMin.y, myMax.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetNearTopLeft() const
	{
		return Vector3<T>(myMin.x, myMax.y, myMin.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetNearTopRight() const
	{
		return Vector3<T>(myMax.x, myMax.y, myMin.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetFarTopLeft() const
	{
		return Vector3<T>(myMin.x, myMax.y, myMax.z);
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetFarTopRight() const
	{
		return GetMax();
	}

	template<class T>
	Vector3<T> AABB3D<T>::GetFurthestCornerInDir(const Vector3<T>& aDir) const
	{
		return Vector3<T>
		(
			aDir.x > 0 ? myMax.x : myMin.x,
			aDir.y > 0 ? myMax.y : myMin.y,
			aDir.z > 0 ? myMax.z : myMin.z
		);
	}
}
