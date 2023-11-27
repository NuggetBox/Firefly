#pragma once
#include "Vector3.hpp"

namespace Utils
{
	template <class T>
	class Ray
	{
	public:
		// Default constructor: there is no ray, the origin and direction are the
		// zero vector.
		Ray();
		// Copy constructor.
		Ray(const Ray<T>& aRay);
		// Constructor that takes two points that define the ray, the direction is
		// aPoint - aOrigin and the origin is aOrigin.
		Ray(const Vector3<T>& aOrigin, const Vector3<T>& aPoint);
		// Init the ray with two points, the same as the constructor above.
		void InitWith2Points(const Vector3<T>& aOrigin, const Vector3<T>& aPoint);
		// Init the ray with an origin and a direction.
		void InitWithOriginAndDirection(const Vector3<T>& aOrigin, const Vector3<T>& aDirection);

		const Vector3<T>& GetOrigin() const;
		const Vector3<T>& GetDirection() const;
		const Vector3<T>& GetEnd() const { return myEnd; }

	private:
		Vector3<T> myOrigin;
		Vector3<T> myEnd;
		Vector3<T> myDirection;

		//Normalizes the direction of the ray
		void Normalize();
	};

	template <class T>
	const Vector3<T>& Ray<T>::GetOrigin() const
	{
		return myOrigin;
	}

	template <class T>
	const Vector3<T>& Ray<T>::GetDirection() const
	{
		return myDirection;
	}

	template <class T>
	Ray<T>::Ray()
	{
	}

	template <class T>
	Ray<T>::Ray(const Ray<T>& aRay)
	{
		myOrigin = aRay.GetOrigin();
		myDirection = aRay.GetDirection();
		Normalize();
	}

	template <class T>
	Ray<T>::Ray(const Vector3<T>& aOrigin, const Vector3<T>& aPoint)
	{
		myOrigin = aOrigin;
		myDirection = aPoint - aOrigin;
		Normalize();
	}

	template <class T>
	void Ray<T>::InitWith2Points(const Vector3<T>& aOrigin, const Vector3<T>& aPoint)
	{
		myOrigin = aOrigin;
		myDirection = aPoint - aOrigin;
		myEnd = aPoint;
		Normalize();
	}

	template <class T>
	void Ray<T>::InitWithOriginAndDirection(const Vector3<T>& aOrigin, const Vector3<T>& aDirection)
	{
		myOrigin = aOrigin;
		myDirection = aDirection;
		Normalize();
	}

	template <class T>
	void Ray<T>::Normalize()
	{
		myDirection.Normalize();
	}
}
