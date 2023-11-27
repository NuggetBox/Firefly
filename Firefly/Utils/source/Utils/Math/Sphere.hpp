#pragma once
#include "Vector3.hpp"

namespace Utils
{
	template <class T>
	class Sphere
	{
	public:

		// Default constructor: there is no sphere, the radius is zero and the position is
		// the zero vector.
		Sphere();
		// Copy constructor.
		Sphere(const Sphere<T>& aSphere);
		// Constructor that takes the center position and radius of the sphere.
		Sphere(const Vector3<T>& aCenter, T aRadius);
		// Init the sphere with a center and a radius, the same as the constructor above.
		void InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius);
		// Returns whether a point is inside the sphere: it is inside when the point is on the
		// sphere surface or inside of the sphere.
		bool IsInside(const Vector3<T>& aPosition) const;

		void SetOrigin(const Vector3<T>& anOrigin);
		void SetRadius(float aRadius);

		Vector3<T> GetOrigin() const;
		T GetRadius() const;

	private:
		Vector3<T> myOrigin;
		T myRadius;
	};

	template <class T>
	Sphere<T>::Sphere()
	{
		myRadius = 0;
	}

	template <class T>
	Sphere<T>::Sphere(const Sphere<T>& aSphere)
	{
		myOrigin = aSphere.GetOrigin();
		myRadius = aSphere.GetRadius();
	}

	template <class T>
	Sphere<T>::Sphere(const Vector3<T>& aCenter, T aRadius)
	{
		myOrigin = aCenter;
		myRadius = aRadius;
	}

	template <class T>
	void Sphere<T>::InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius)
	{
		myOrigin = aCenter;
		myRadius = aRadius;
	}

	template <class T>
	bool Sphere<T>::IsInside(const Utils::Vector3<T>& aPosition) const
	{
		return (aPosition - myOrigin).LengthSqr() <= (myRadius * myRadius);
	}

	template<class T>
	void Sphere<T>::SetOrigin(const Vector3<T>& anOrigin)
	{
		myOrigin = anOrigin;
	}

	template<class T>
	void Sphere<T>::SetRadius(const float aRadius)
	{
		myRadius = aRadius;
	}

	template <class T>
	Vector3<T> Sphere<T>::GetOrigin() const
	{
		return myOrigin;
	}

	template <class T>
	T Sphere<T>::GetRadius() const
	{
		return myRadius;
	}
}
