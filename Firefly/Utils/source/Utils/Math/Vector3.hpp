#pragma once
#include "Utils/UtilityFunctions.hpp"

#include <cassert>

namespace Utils
{
	template <class T>
	class Vector3
	{
	public:

		T x;
		T y;
		T z;

		//Creates a null-vector
		Vector3<T>();

		//Creates a vector where all components are the same
		Vector3<T>(const T& aValue);

		//Creates a vector (aX, aY, aZ)
		Vector3<T>(const T& aX, const T& aY, const T& aZ);

		//Copy constructor (compiler generated)
		Vector3<T>(const Vector3<T>& aVector) = default;

		Vector3<T>& operator*=(const Vector3<T>& aRhs);

		//Assignment operator (compiler generated)
		Vector3<T>& operator=(const Vector3<T>& aVector3) = default;

		T& operator[](int anIndex);
		const T& operator[](int anIndex) const;

		bool IsAlmostEqual(const Vector3<T>& aVector, T aTolerance = 0.0001f);

		//Destructor (compiler generated)
		~Vector3<T>() = default;

		//Returns the squared length of the vector
		T LengthSqr() const;

		//Returns the length of the vector
		T Length() const;

		static Vector3<T> Lerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor);
		static Vector3<T> Slerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor);
		static Vector3<T> NormalizedLerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor);

		//Returns a normalized copy of this
		Vector3<T> GetNormalized() const;

		//Normalizes the vector
		void Normalize();

		static void Orthonormalize(Vector3<T>& aNormal, Utils::Vector3<T>& aTangent);

		//Returns the dot product of this and aVector
		T Dot(const Vector3<T>& aVector) const;

		//Returns the cross product of this and aVector
		Vector3<T> Cross(const Vector3<T>& aVector) const;

		static Vector3<T> Zero() { return Vector3<T>(0, 0, 0); }
		static Vector3<T> One() { return Vector3<T>(1, 1, 1); }
		static Vector3<T> Up() { return Vector3<T>(0, 1, 0); }
		static Vector3<T> Down() { return Vector3<T>(0, -1, 0); }
		static Vector3<T> Right() { return Vector3<T>(1, 0, 0); }
		static Vector3<T> Left() { return Vector3<T>(-1, 0, 0); }
		static Vector3<T> Forward() { return Vector3<T>(0, 0, 1); }
		static Vector3<T> Backward() { return Vector3<T>(0, 0, -1); }
	};

	typedef Vector3<float> Vector3f;
	typedef Vector3<float> Vec3; // Niklas Added this: don't need to write So much to get the most common version of a vector.

	template <class T>
	Vector3<T>::Vector3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	template <class T>
	Vector3<T>::Vector3(const T& aValue)
	{
		x = aValue;
		y = aValue;
		z = aValue;
	}

	template <class T>
	Vector3<T>::Vector3(const T& aX, const T& aY, const T& aZ)
	{
		x = aX;
		y = aY;
		z = aZ;
	}

	template<class T>
	Vector3<T>& Vector3<T>::operator*=(const Vector3<T>& aRhs)
	{
		*this = *this * aRhs;
		return *this;
	}

	template <class T>
	T& Vector3<T>::operator[](int anIndex)
	{
		assert(anIndex < 3 && "Tried to access a Vector3 value outside of index range, 0 gives x, 1 gives y, 2 gives z");

		switch (anIndex)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}
		return x;
	}

	template <class T>
	const T& Vector3<T>::operator[](int anIndex) const
	{
		assert(anIndex < 3 && "Tried to access a Vector3 value outside of index range, 0 gives x, 1 gives y, 2 gives z");

		switch (anIndex)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}
		return x;
	}

	template<class T>
	bool Vector3<T>::IsAlmostEqual(const Vector3<T>& aVector, T aTolerance)
	{
		return
			Utils::IsAlmostEqual(x, aVector.x, aTolerance) && 
			Utils::IsAlmostEqual(y, aVector.y, aTolerance) && 
			Utils::IsAlmostEqual(z, aVector.z, aTolerance);
	}

	template <class T>
	T Vector3<T>::LengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	template <class T>
	T Vector3<T>::Length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	template<class T>
	Vector3<T> Vector3<T>::Lerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor)
	{
		return Vector3<T>(
			Utils::Lerp(aFirstVector.x, aSecondVector.x, aFactor),
			Utils::Lerp(aFirstVector.y, aSecondVector.y, aFactor),
			Utils::Lerp(aFirstVector.z, aSecondVector.z, aFactor));
	}

	template<class T>
	inline Vector3<T> Vector3<T>::Slerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor)
	{
		// Dot product - the cosine of the angle between 2 vectors.
		auto vec = aFirstVector.GetNormalized();
		auto secontVec = aSecondVector.GetNormalized();
		float dot = vec.Dot(vec);
		// Clamp it to be in the range of Acos()
		// This may be unnecessary, but floating point
		// precision can be a fickle mistress.
		dot = Clamp(dot, -1.0f, 1.0f);
		// Acos(dot) returns the angle between start and end,
		// And multiplying that by percent returns the angle between
		// start and the final result.
		float theta = std::acos(dot) * aFactor;
		Vector3<T> RelativeVec = aSecondVector - aFirstVector * dot;
		RelativeVec.Normalize();
		// Orthonormal basis
		// The final result.
		return ((aFirstVector * std::cos(theta)) + (RelativeVec * std::sin(theta)));
	}

	template<class T>
	Vector3<T> Vector3<T>::NormalizedLerp(const Vector3<T>& aFirstVector, const Vector3<T>& aSecondVector, float aFactor)
	{
		Vector3<T> lerped = Lerp(aFirstVector, aSecondVector, aFactor);

		if (lerped.LengthSqr() > 0)
		{
			lerped.Normalize();
		}

		return lerped;
	}

	template <class T>
	Vector3<T> Vector3<T>::GetNormalized() const
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return *this;
		}

		T factor = 1 / Length();
		return Vector3<T>(x * factor, y * factor, z * factor);
	}

	template <class T>
	void Vector3<T>::Normalize()
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return;
		}

		T factor = 1 / Length();

		x = x * factor;
		y = y * factor;
		z = z * factor;
	}

	template<class T>
	void Vector3<T>::Orthonormalize(Vector3<T>& aNormal, Utils::Vector3<T>& aTangent)
	{
		aNormal.Normalize();
		aTangent = aTangent - aTangent.Dot(aNormal) * aNormal;
		aTangent.Normalize();
	}

	template <class T>
	T Vector3<T>::Dot(const Vector3<T>& aVector) const
	{
		return x * aVector.x + y * aVector.y + z * aVector.z;
	}

	template <class T>
	Vector3<T> Vector3<T>::Cross(const Vector3<T>& aVector) const
	{
		return Vector3<T>(y * aVector.z - z * aVector.y, z * aVector.x - x * aVector.z, x * aVector.y - y * aVector.x);
	}

	//Returns the vector sum of aVector0 and aVector1
	template <class T>
	Vector3<T> operator+(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z);
	}

	//Returns the vector difference of aVector0 and aVector1
	template <class T>
	Vector3<T> operator-(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(aVector0.x - aVector1.x, aVector0.y - aVector1.y, aVector0.z - aVector1.z);
	}

	template <class T>
	Vector3<T> operator-(const Vector3<T>& aVector)
	{
		return aVector * static_cast<T>(-1.0);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector, const T& aScalar)
	{
		return Vector3<T>(aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar);
	}

	//Returns the vector aVector multiplied by the scalar aScalar
	template <class T>
	Vector3<T> operator*(const T& aScalar, const Vector3<T>& aVector)
	{
		return Vector3<T>(aScalar * aVector.x, aScalar * aVector.y, aScalar * aVector.z);
	}

	//Vector * vector multiplication
	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(aVector0.x * aVector1.x, aVector0.y * aVector1.y, aVector0.z * aVector1.z);
	}

	//Vector / vector division
	template <class T>
	Vector3<T> operator/(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return Vector3<T>(aVector0.x / aVector1.x, aVector0.y / aVector1.y, aVector0.z / aVector1.z);
	}

	//Returns the vector aVector divided by the scalar aScalar (equivalent to aVector multiplied by 1 / aScalar)
	template <class T>
	Vector3<T> operator/(const Vector3<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		return Vector3<T>(aVector.x / aScalar, aVector.y / aScalar, aVector.z / aScalar);
	}

	//Equivalent to setting aVector0 to (aVector0 + aVector1)
	template <class T>
	void operator+=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

	//Equivalent to setting aVector0 to (aVector0 - aVector1)
	template <class T>
	void operator-=(Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

	//Equivalent to setting aVector to (aVector * aScalar)
	template <class T>
	void operator*=(Vector3<T>& aVector, const T& aScalar)
	{
		aVector = aVector * aScalar;
	}

	//Equivalent to setting aVector to (aVector / aScalar)
	template <class T>
	void operator/=(Vector3<T>& aVector, const T& aScalar)
	{
		assert(aScalar != 0 && "Division by zero");
		aVector = aVector / aScalar;
	}

	//Compare operator, compares each individual value
	template <class T>
	bool operator==(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return aVector0.x == aVector1.x && aVector0.y == aVector1.y && aVector0.z == aVector1.z;
	}

	//Inverse compare operator, compares each individual value
	template <class T>
	bool operator!=(const Vector3<T>& aVector0, const Vector3<T>& aVector1)
	{
		return aVector0.x != aVector1.x || aVector0.y != aVector1.y || aVector0.z != aVector1.z;
	}
}
