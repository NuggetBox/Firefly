#pragma once
#include <cassert>

#include "Vector3.hpp"
#include "Utils/UtilityFunctions.hpp"

namespace Utils
{
	template <class T>
	class Vector4
	{
	public:

		T x;
		T y;
		T z;
		T w;

		//Creates a null-vector
		Vector4<T>();

		//Creates a vector where all components are the same
		Vector4<T>(const T& aValue);

		//Creates a vector (aX, aY, aZ, aW)
		Vector4<T>(const T& aX, const T& aY, const T& aZ, const T& aW);

		//Copy constructor (compiler generated)
		Vector4<T>(const Vector4<T>& aVector) = default;

		Vector4<T>(const Vector3<T>& aVector3);

		Vector4<T>(const std::array<T, 4>& anArray);

		Vector4<T>& operator*=(const Vector4<T>& aRhs);

		//Assignment operator (compiler generated)
		Vector4<T>& operator=(const Vector4<T>& aVector4) = default;

		T& operator[](const unsigned anIndex);
		const T& operator[](const unsigned anIndex) const;

		bool IsAlmostEqual(const Vector4<T>& aVector, T aTolerance = 0.00001f);

		//Destructor (compiler generated)
		~Vector4<T>() = default;

		//Returns the squared length of the vector
		T LengthSqr() const;

		//Returns the length of the vector
		T Length() const;

		static Vector4<T> Lerp(const Vector4<T>& aFirstVector, const Vector4<T>& aSecondVector, float aFactor);
		static Vector4<T> NormalizedLerp(const Vector4<T>& aFirstVector, const Vector4<T>& aSecondVector, float aFactor);

		//Returns a normalized copy of this
		Vector4<T> GetNormalized() const;

		//Normalizes the vector
		void Normalize();

		//Returns the dot product of this and aVector
		T Dot(const Vector4<T>& aVector) const;
	};

	typedef Vector4<float> Vector4f;
	typedef Vector4<float> Vec4;

	template <class T>
	Vector4<T>::Vector4()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	template <class T>
	Vector4<T>::Vector4(const T& aValue)
	{
		x = aValue;
		y = aValue;
		z = aValue;
		w = aValue;
	}

	template <class T>
	Vector4<T>::Vector4(const T& aX, const T& aY, const T& aZ, const T& aW)
	{
		x = aX;
		y = aY;
		z = aZ;
		w = aW;
	}

	template<class T>
	Vector4<T>::Vector4(const Vector3<T>& aVector3)
	{
		x = aVector3.x;
		y = aVector3.y;
		z = aVector3.z;
		w = 1;
	}

	template<class T>
	Vector4<T>::Vector4(const std::array<T, 4>& anArray)
	{
		x = anArray[0];
		y = anArray[1];
		z = anArray[2];
		w = anArray[3];
	}

	template<class T>
	Vector4<T>& Vector4<T>::operator*=(const Vector4<T>& aRhs)
	{
		*this = *this * aRhs;
		return *this;
	}

	template <class T>
	T& Vector4<T>::operator[](const unsigned anIndex)
	{
		assert(anIndex >= 0 && anIndex < 4 && "Tried to access a Vector4 value outside of index range, 0 gives x, 1 gives y, 2 gives z, 3 gives w");

		switch (anIndex)
		{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
		}

		return x;
	}

	template <class T>
	const T& Vector4<T>::operator[](const unsigned anIndex) const
	{
		assert(anIndex >= 0 && anIndex < 4 && "Tried to access a Vector4 value outside of index range, 0 gives x, 1 gives y, 2 gives z, 3 gives w");

		switch (anIndex)
		{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
		}

		return x;
	}

	template<class T>
	bool Vector4<T>::IsAlmostEqual(const Vector4<T>& aVector, T aTolerance)
	{
		return
			Utils::IsAlmostEqual(x, aVector.x, aTolerance) && 
			Utils::IsAlmostEqual(y, aVector.y, aTolerance) && 
			Utils::IsAlmostEqual(z, aVector.z, aTolerance) && 
			Utils::IsAlmostEqual(w, aVector.w, aTolerance);
	}

	template <class T>
	T Vector4<T>::LengthSqr() const
	{
		return x * x + y * y + z * z + w * w;
	}

	template <class T>
	T Vector4<T>::Length() const
	{
		return std::sqrt(LengthSqr());
	}

	template<class T>
	Vector4<T> Vector4<T>::Lerp(const Vector4<T>& aFirstVector, const Vector4<T>& aSecondVector, float aFactor)
	{
		return Vector4<T>(
			Utils::Lerp(aFirstVector.x, aSecondVector.x, aFactor),
			Utils::Lerp(aFirstVector.y, aSecondVector.y, aFactor),
			Utils::Lerp(aFirstVector.z, aSecondVector.z, aFactor),
			Utils::Lerp(aFirstVector.w, aSecondVector.w, aFactor));
	}

	template<class T>
	Vector4<T> Vector4<T>::NormalizedLerp(const Vector4<T>& aFirstVector, const Vector4<T>& aSecondVector, float aFactor)
	{
		Vector4<T> lerped = Lerp(aFirstVector, aSecondVector, aFactor);

		if (lerped.LengthSqr() > 0)
		{
			lerped.Normalize();
		}

		return lerped;
	}

	template <class T>
	Vector4<T> Vector4<T>::GetNormalized() const
	{
		//assert(x != 0 || y != 0 || z != 0 && "The Vector4 was a zero-vector and a normalized version can't be returned");

		if (x == static_cast<T>(0.0) && y == static_cast<T>(0.0) && z == static_cast<T>(0.0) && w == static_cast<T>(0.0))
		{
			return { static_cast<T>(0.0) };
		}

		T factor = 1 / Length();
		return Vector4<T>(x * factor, y * factor, z * factor, w * factor);
	}

	template <class T>
	void Vector4<T>::Normalize()
	{
		//assert(x != 0 || y != 0 || z != 0 || w != 0 && "The Vector4 was a zero-vector and can't be normalized");

		if (x == static_cast<T>(0.0) && y == static_cast<T>(0.0) && z == static_cast<T>(0.0) && w == static_cast<T>(0.0))
		{
			return;
		}

		T factor = 1 / Length();

		x = x * factor;
		y = y * factor;
		z = z * factor;
		w = w * factor;
	}

	template <class T>
	T Vector4<T>::Dot(const Vector4<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y + z * aVector.z + w * aVector.w;
	}

	//Returns the vector sum of aVector0 and aVector1
	template <class T> Vector4<T> operator+(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z, aVector0.w + aVector1.w);
	}

	//Returns the vector difference of aVector0 and aVector1
	template <class T> Vector4<T> operator-(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x - aVector1.x, aVector0.y - aVector1.y, aVector0.z - aVector1.z, aVector0.w - aVector1.w);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T> Vector4<T> operator*(const Vector4<T>& aVector, const T& aScalar)
	{
		return Vector4<T>(aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar, aVector.w * aScalar);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T> Vector4<T> operator*(const T& aScalar, const Vector4<T>& aVector)
	{
		return Vector4<T>(aScalar * aVector.x, aScalar * aVector.y, aScalar * aVector.z, aScalar * aVector.w);
	}
	
	//Vector times vector
	template <class T> Vector4<T> operator*(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x * aVector1.x, aVector0.y * aVector1.y, aVector0.z * aVector1.z, aVector0.w * aVector1.w);
	}


	//Returns the vector aVector divided by the scalar aScalar (equivalent to aVector multiplied by 1 / aScalar)
	template <class T> Vector4<T> operator/(const Vector4<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		return Vector4<T>(aVector.x / aScalar, aVector.y / aScalar, aVector.z / aScalar, aVector.w / aScalar);
	}

	//Equivalent to setting aVector0 to (aVector0 + aVector1)
	template <class T> void operator+=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	//Equivalent to setting aVector0 to (aVector0 - aVector1)
	template <class T> void operator-=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	//Equivalent to setting aVector to (aVector * aScalar)
	template <class T> void operator*=(Vector4<T>& aVector, const T& aScalar)
	{
		aVector = aVector * aScalar;
	}

	//Equivalent to setting aVector to (aVector / aScalar)
	template <class T> void operator/=(Vector4<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		aVector = aVector / aScalar;
	}

	//Compare operator, compares each individual value
	template <class T> bool operator==(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return aVector0.x == aVector1.x && aVector0.y == aVector1.y && aVector0.z == aVector1.z && aVector0.w == aVector1.w;
	}

	//Inverse compare operator, compares each individual value
	template <class T> bool operator!=(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return aVector0.x != aVector1.x || aVector0.y != aVector1.y || aVector0.z != aVector1.z || aVector0.w != aVector1.w;
	}
}

typedef Utils::Vector4<float> Vector4f;