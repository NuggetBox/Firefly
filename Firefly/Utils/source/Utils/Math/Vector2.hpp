#pragma once
#include <cassert>

#include "Utils/UtilityFunctions.hpp"

namespace Utils
{
	template <class T>
	class Vector2
	{
	public:

		T x;
		T y;

		//Creates a null-vector
		Vector2<T>();

		//Creates a vector where all components are the same
		Vector2<T>(const T& aValue);

		//Creates a vector (aX, aY)
		Vector2<T>(const T& aX, const T& aY);

		//Copy constructor (compiler generated)
		Vector2<T>(const Vector2<T>& aVector) = default;

		//Assignment operator (compiler generated)
		Vector2<T>& operator=(const Vector2<T>& aVector2) = default;

		bool IsAlmostEqual(const Vector2<T>& aVector, T aTolerance = 0.0001f);

		//Destructor (compiler generated)
		~Vector2<T>() = default;

		//Returns the squared length of the vector
		T LengthSqr() const;

		//Returns the length of the vector
		T Length() const;
		
		T Cross(const Vector2<T>& aVector) const;

		static T Cross(const Vector2<T>& aVector1, const Vector2<T>& aVector2);

		static Vector2<T> Lerp(const Vector2<T>& aFirstVector, const Vector2<T>& aSecondVector, float aFactor);

		//Returns a normalized copy of this
		Vector2<T> GetNormalized() const;

		//Normalizes the vector
		void Normalize();

		//Returns the dot product of this and aVector
		T Dot(const Vector2<T>& aVector) const;

		T& operator[](const int aIndex);
	};

	typedef Vector2<float> Vector2f;

	template <class T>
	Vector2<T>::Vector2()
	{
		x = 0;
		y = 0;
	}

	template <class T>
	Vector2<T>::Vector2(const T& aValue)
	{
		x = aValue;
		y = aValue;
	}

	template <class T>
	Vector2<T>::Vector2(const T& aX, const T& aY)
	{
		x = aX;
		y = aY;
	}

	template<class T>
	bool Vector2<T>::IsAlmostEqual(const Vector2<T>& aVector, T aTolerance)
	{
		return
			Utils::IsAlmostEqual(x, aVector.x, aTolerance) && 
			Utils::IsAlmostEqual(y, aVector.y, aTolerance);
	}

	template <class T>
	T Vector2<T>::LengthSqr() const
	{
		return x * x + y * y;
	}

	template <class T>
	T Vector2<T>::Length() const
	{
		return std::sqrt(x * x + y * y);
		//return std::sqrt(LengthSqr());
	}

	template<class T>
	inline T Vector2<T>::Cross(const Vector2<T>& aVector) const
	{
		return x * aVector.y - y * aVector.x;
	}
	
	template<class T>
	inline T Vector2<T>::Cross(const Vector2<T>& aVector1, const Vector2<T>& aVector2)
	{
		return aVector1.x * aVector2.y - aVector1.y * aVector2.x;
	}

	template <class T>
	Vector2<T> Vector2<T>::Lerp(const Vector2<T>& aFirstVector, const Vector2<T>& aSecondVector, float aFactor)
	{
		return Vector2<T>(
			Utils::Lerp(aFirstVector.x, aSecondVector.x, aFactor),
			Utils::Lerp(aFirstVector.y, aSecondVector.y, aFactor));
	}

	template <class T>
	Vector2<T> Vector2<T>::GetNormalized() const
	{
		//assert(x != 0 || y != 0 && "The Vector2 was a zero-vector and a normalized version can't be returned");

		T factor = 1 / Length();
		return Vector2<T>(x * factor, y * factor);
	}

	template <class T>
	void Vector2<T>::Normalize()
	{
		assert(x != 0 || y != 0 && "The Vector2 was a zero-vector and can't be normalized");

		T factor = 1 / Length();

		x = x * factor;
		y = y * factor;
	}

	template <class T>
	T Vector2<T>::Dot(const Vector2<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y;
	}
	
	template <class T>
	T& Vector2<T>::operator[](const int aIndex)
	{
		assert(aIndex >= 0 && aIndex < 2 && "Index out of bounds");
		return *(&x + aIndex);
	}

	//Returns the vector sum of aVector0 and aVector1
	template <class T> Vector2<T> operator+(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y);
	}

	//Returns the vector difference of aVector0 and aVector1
	template <class T> Vector2<T> operator-(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x - aVector1.x, aVector0.y - aVector1.y);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T> Vector2<T> operator*(const Vector2<T>& aVector, const T& aScalar)
	{
		return Vector2<T>(aVector.x * aScalar, aVector.y * aScalar);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T> Vector2<T> operator*(const T& aScalar, const Vector2<T>& aVector)
	{
		return Vector2<T>(aScalar * aVector.x, aScalar * aVector.y);
	}

	//Returns the vector aVector divided by the scalar aScalar (equivalent to aVector multiplied by 1 / aScalar)
	template <class T> Vector2<T> operator/(const Vector2<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		return Vector2<T>(aVector.x / aScalar, aVector.y / aScalar);
	}

	//Equivalent to setting aVector0 to (aVector0 + aVector1)
	template <class T> void operator+=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	//Equivalent to setting aVector0 to (aVector0 - aVector1)
	template <class T> void operator-=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	//Equivalent to setting aVector to (aVector * aScalar)
	template <class T> void operator*=(Vector2<T>& aVector, const T& aScalar)
	{
		aVector = aVector * aScalar;
	}

	//Equivalent to setting aVector to (aVector / aScalar)
	template <class T> void operator/=(Vector2<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		aVector = aVector / aScalar;
	}

	//Compare operator, compares each individual value
	template <class T> bool operator==(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return aVector0.x == aVector1.x && aVector0.y == aVector1.y;
	}

	//Inverse compare operator, compares each individual value
	template <class T> bool operator!=(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return aVector0.x != aVector1.x || aVector0.y != aVector1.y;
	}
}
