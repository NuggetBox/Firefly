#pragma once
#include <vector>

#include "Plane.hpp"

namespace Utils
{
	template <class T>
	class PlaneVolume
	{
	public:
		// Default constructor: empty PlaneVolume.
		PlaneVolume();
		// Constructor taking a list of Plane that makes up the PlaneVolume.
		PlaneVolume(const std::vector<Plane<T>>& aPlaneList);
		// Add a Plane to the PlaneVolume.
		void AddPlane(const Plane<T>& aPlane);
		// Returns whether a point is inside the PlaneVolume: it is inside when the point is on the plane or on the side the normal is pointing away from for all the planes in the PlaneVolume.
		bool IsInside(const Vector3<T>& aPosition);

		const std::vector<Plane<T>>& GetPlanes();

		void Clear();

	private:
		std::vector<Plane<T>> myPlanes;
	};

	template <class T>
	PlaneVolume<T>::PlaneVolume()
	{
		myPlanes.reserve(6);
	}

	template <class T>
	PlaneVolume<T>::PlaneVolume(const std::vector<Plane<T>>& aPlaneList)
	{
		myPlanes = aPlaneList;
	}

	template <class T>
	void PlaneVolume<T>::AddPlane(const Plane<T>& aPlane)
	{
		myPlanes.push_back(aPlane);
	}

	template <class T>
	bool PlaneVolume<T>::IsInside(const Vector3<T>& aPosition)
	{
		for (const Plane<T>& plane : myPlanes)
		{
			if (!plane.IsInside(aPosition))
			{
				return false;
			}
		}

		return true;
	}

	template<class T>
	const std::vector<Plane<T>>& PlaneVolume<T>::GetPlanes()
	{
		return myPlanes;
	}

	template<class T>
	void PlaneVolume<T>::Clear()
	{
		myPlanes.clear();
	}
}
