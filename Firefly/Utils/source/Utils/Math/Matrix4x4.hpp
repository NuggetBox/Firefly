#pragma once
#include <vector>

#include "MathDefines.h"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Quaternion.h"

namespace Utils
{
	template <class T>
	class Matrix4x4
	{
	public:
		Matrix4x4<T>(bool aIdentity = true);

		T& operator[](uint8_t aElementIndex); //0, 1, 2 ... 14, 15
		const T& operator[](uint8_t aElementIndex) const; //0, 1, 2 ... 14, 15

		T& operator()(uint8_t aRow, uint8_t aColumn); //1, 2, 3, 4
		const T& operator()(uint8_t aRow, uint8_t aColumn) const; //1, 2, 3, 4

		Vector4<T> GetRowVector(uint8_t aRow) const; //1, 2, 3, 4
		Vector4<T> GetColumnVector(uint8_t aColumn) const; //1, 2, 3, 4

		T* data() { return myMatrix; }
		const T* data() const { return myMatrix; }

		void SetIdentity();

		static Matrix4x4<T> Transpose(const Matrix4x4<T>& aMatrixToTranspose);
		void Transpose();

		Matrix4x4<T> GetTranspose() const;

		void operator+=(const Matrix4x4<T>& aMatrix);
		void operator-=(const Matrix4x4<T>& aMatrix);
		void operator*=(const Matrix4x4<T>& aMatrix);
		Matrix4x4<T>& operator=(const Matrix4x4<T>& aMatrix);
		Matrix4x4<T>& operator=(const T aArray[16]);

		static Matrix4x4<T> CreateFromPosRotScale(const Vector3<T>& aTranslation, const Vector3<T>& aRotation, const Vector3<T>& aScale);
		static Matrix4x4<T> CreateFromPosRotScale(const Vector3<float>& aTranslation, const Quaternion& aRotation, const Vector3<float>& aScale);

		static Matrix4x4<T> CreateRotationAroundX(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundY(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundZ(T aAngleInRadians);

		static Matrix4x4<T> CreateRotationMatrix(const Vector3<T>& aEulerAngles);
		static Matrix4x4<float> CreateRotationMatrix(const Quaternion& aQuaternion);

		static Matrix4x4<T> CreateScaleMatrix(const Vector3<T>& aScale);
		static Matrix4x4<T> CreateScaleMatrix(const T& aScale);

		static Matrix4x4<T> CreateTranslationMatrix(Vector3<T> aTranslationVector);

		static Matrix4x4<T> CreateLookAt(const Vector3<T>& aEye, const Vector3<T>& aCenter, const Vector3<T>& aUp);
		static Matrix4x4<T> CreateLookAtLH(const Vector3<T>& aEye, const Vector3<T>& aCenter, const Vector3<T>& aUp);

		static Matrix4x4<T> CreateProjectionMatrixPerspective(float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane, float aFovDegrees);
		static Matrix4x4<T> CreateLeftHandedProjectionMatrixPerspective(float aWidth, float aHeight, float aNearPlane, float aFarPlane, float aFovDegrees);
		static Matrix4x4<T> CreateProjectionMatrixOrthographic(float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane);
		static Matrix4x4<T> CreateProjectionMatrixOrthographic(float aMinX, float aMaxX, float aMinY, float aMaxY, float aNearPlane, float aFarPlane);

		static void Decompose(const Matrix4x4<T>& aMatrix, Vector3<T>& aOutPosition, Vector3<T>& aOutRotation, Vector3<T>& aOutScale);
		static void Decompose(const Matrix4x4<float>& aMatrix, Vector3<float>& aOutPosition, Quaternion& aOutRotation, Vector3<float>& aOutScale);

		static Matrix4x4<T> Lerp(const Matrix4x4<T>& aFirstMatrix, const Matrix4x4<T>& aSecondMatrix, float aFactor);

		// Assumes aTransform is made up of nothing but rotations and translations.
		static Matrix4x4<T> GetFastInverse(const Matrix4x4<T>& aTransform);

		static Matrix4x4<T> GetInverse(const Matrix4x4<T>& aTransform);

	private:
		T myMatrix[16];
	};

	typedef Matrix4x4<float> Matrix4f;
	typedef Matrix4x4<float> Mat4;

	template <class T>
	Matrix4x4<T>::Matrix4x4(const bool aIdentity)
	{
		memset(myMatrix, 0, 16 * sizeof(T));

		if (aIdentity)
		{
			myMatrix[0] = static_cast<T>(1);
			myMatrix[5] = static_cast<T>(1);
			myMatrix[10] = static_cast<T>(1);
			myMatrix[15] = static_cast<T>(1);
		}
	}

	template<class T>
	T& Matrix4x4<T>::operator[](uint8_t aElementIndex)
	{
		assert(aElementIndex <= 15 && "Element index out of range, allowed indices range from 0 to 15");
		return myMatrix[aElementIndex];
	}

	template<class T>
	const T& Matrix4x4<T>::operator[](uint8_t aElementIndex) const
	{
		assert(aElementIndex <= 15 && "Element index out of range, allowed indices range from 0 to 15");
		return myMatrix[aElementIndex];
	}

	template <class T>
	T& Matrix4x4<T>::operator()(const uint8_t aRow, const uint8_t aColumn)
	{
		assert(aRow >= 1 && aRow <= 4 && "Row index out of range, allowed indices for Matrix4x4 row: 1, 2, 3, 4");
		assert(aColumn >= 1 && aColumn <= 4 && "Column index out of range, allowed indices for Matrix4x4 column: 1, 2, 3, 4");

		return myMatrix[4 * (aRow - 1) + (aColumn - 1)];
	}

	template <class T>
	const T& Matrix4x4<T>::operator()(const uint8_t aRow, const uint8_t aColumn) const
	{
		assert(aRow >= 1 && aRow <= 4 && "Row index out of range, allowed indices for Matrix4x4 row: 1, 2, 3, 4");
		assert(aColumn >= 1 && aColumn <= 4 && "Column index out of range, allowed indices for Matrix4x4 column: 1, 2, 3, 4");

		return myMatrix[4 * (aRow - 1) + (aColumn - 1)];
	}

	template <class T>
	Vector4<T> Matrix4x4<T>::GetRowVector(const uint8_t aRow) const
	{
		assert(aRow >= 1 && aRow <= 4 && "Row index out of range, allowed indices for Matrix4x4 row: 1, 2, 3, 4");
		return { myMatrix[4 * (aRow - 1)], myMatrix[4 * (aRow - 1) + 1], myMatrix[4 * (aRow - 1) + 2], myMatrix[4 * (aRow - 1) + 3]};
	}

	template <class T>
	Vector4<T> Matrix4x4<T>::GetColumnVector(const uint8_t aColumn) const
	{
		assert(aColumn >= 1 && aColumn <= 4 && "Column index out of range, allowed indices for Matrix4x4 column: 1, 2, 3, 4");
		return { myMatrix[aColumn - 1], myMatrix[aColumn + 3], myMatrix[aColumn + 7] , myMatrix[aColumn + 11] };
	}

	template<class T>
	inline void Matrix4x4<T>::SetIdentity()
	{
		memset(myMatrix, 0, 16 * sizeof(T));
		myMatrix[0] = static_cast<T>(1);
		myMatrix[5] = static_cast<T>(1);
		myMatrix[10] = static_cast<T>(1);
		myMatrix[15] = static_cast<T>(1);
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(const Vector3<T>& aScale)
	{
		Matrix4x4<T> scaleMat;
		scaleMat[0] = aScale.x;
		scaleMat[5] = aScale.y;
		scaleMat[10] = aScale.z;
		return scaleMat;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(const T& aScale)
	{
		Matrix4x4<T> scaleMat;
		scaleMat[0] = aScale;
		scaleMat[5] = aScale;
		scaleMat[10] = aScale;
		return scaleMat;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslationMatrix(Vector3<T> aTranslationVector)
	{
		Matrix4x4<T> translationMat;
		translationMat[12] = aTranslationVector.x;
		translationMat[13] = aTranslationVector.y;
		translationMat[14] = aTranslationVector.z;
		return translationMat;
	}

	template <class T>
	Matrix4x4<T> operator+(const Matrix4x4<T>& aMat0, const Matrix4x4<T>& aMat1)
	{
		Matrix4x4<T> temp = aMat0;
		temp += aMat1;
		return temp;
	}

	template <class T>
	void Matrix4x4<T>::operator+=(const Matrix4x4<T>& aMatrix)
	{
		myMatrix[0] += aMatrix[0];
		myMatrix[1] += aMatrix[1];
		myMatrix[2] += aMatrix[2];
		myMatrix[3] += aMatrix[3];
		myMatrix[4] += aMatrix[4];
		myMatrix[5] += aMatrix[5];
		myMatrix[6] += aMatrix[6];
		myMatrix[7] += aMatrix[7];
		myMatrix[8] += aMatrix[8];
		myMatrix[9] += aMatrix[9];
		myMatrix[10] += aMatrix[10];
		myMatrix[11] += aMatrix[11];
		myMatrix[12] += aMatrix[12];
		myMatrix[13] += aMatrix[13];
		myMatrix[14] += aMatrix[14];
		myMatrix[15] += aMatrix[15];
	}

	template <class T>
	Matrix4x4<T> operator-(const Matrix4x4<T>& aMat0, const Matrix4x4<T>& aMat1)
	{
		Matrix4x4<T> temp = aMat0;
		temp -= aMat1;
		return temp;
	}

	template <class T>
	void Matrix4x4<T>::operator-=(const Matrix4x4<T>& aMatrix)
	{
		myMatrix[0] -= aMatrix[0];
		myMatrix[1] -= aMatrix[1];
		myMatrix[2] -= aMatrix[2];
		myMatrix[3] -= aMatrix[3];
		myMatrix[4] -= aMatrix[4];
		myMatrix[5] -= aMatrix[5];
		myMatrix[6] -= aMatrix[6];
		myMatrix[7] -= aMatrix[7];
		myMatrix[8] -= aMatrix[8];
		myMatrix[9] -= aMatrix[9];
		myMatrix[10] -= aMatrix[10];
		myMatrix[11] -= aMatrix[11];
		myMatrix[12] -= aMatrix[12];
		myMatrix[13] -= aMatrix[13];
		myMatrix[14] -= aMatrix[14];
		myMatrix[15] -= aMatrix[15];
	}

	template <class T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMat0, const Matrix4x4<T>& aMat1)
	{
		Matrix4x4<T> temp(false);

		//Old multiplication with 3 function calls inside a double for-loop
		/*for (int i = 1; i <= 4; ++i)
		{
			for (int j = 1; j <= 4; ++j)
			{
				temp(i, j) = aMat0.GetRowVector(i).Dot(aMat1.GetColumnVector(j));
			}
		}*/

		/*temp[0][0] = aMat0[0][0] * aMat1[0][0] + aMat0[0][1] * aMat1[1][0] + aMat0[0][2] * aMat1[2][0] + aMat0[0][3] * aMat1[3][0];
		temp[0][1] = aMat0[0][0] * aMat1[0][1] + aMat0[0][1] * aMat1[1][1] + aMat0[0][2] * aMat1[2][1] + aMat0[0][3] * aMat1[3][1];
		temp[0][2] = aMat0[0][0] * aMat1[0][2] + aMat0[0][1] * aMat1[1][2] + aMat0[0][2] * aMat1[2][2] + aMat0[0][3] * aMat1[3][2];
		temp[0][3] = aMat0[0][0] * aMat1[0][3] + aMat0[0][1] * aMat1[1][3] + aMat0[0][2] * aMat1[2][3] + aMat0[0][3] * aMat1[3][3];
		temp[1][0] = aMat0[1][0] * aMat1[0][0] + aMat0[1][1] * aMat1[1][0] + aMat0[1][2] * aMat1[2][0] + aMat0[1][3] * aMat1[3][0];
		temp[1][1] = aMat0[1][0] * aMat1[0][1] + aMat0[1][1] * aMat1[1][1] + aMat0[1][2] * aMat1[2][1] + aMat0[1][3] * aMat1[3][1];
		temp[1][2] = aMat0[1][0] * aMat1[0][2] + aMat0[1][1] * aMat1[1][2] + aMat0[1][2] * aMat1[2][2] + aMat0[1][3] * aMat1[3][2];
		temp[1][3] = aMat0[1][0] * aMat1[0][3] + aMat0[1][1] * aMat1[1][3] + aMat0[1][2] * aMat1[2][3] + aMat0[1][3] * aMat1[3][3];
		temp[2][0] = aMat0[2][0] * aMat1[0][0] + aMat0[2][1] * aMat1[1][0] + aMat0[2][2] * aMat1[2][0] + aMat0[2][3] * aMat1[3][0];
		temp[2][1] = aMat0[2][0] * aMat1[0][1] + aMat0[2][1] * aMat1[1][1] + aMat0[2][2] * aMat1[2][1] + aMat0[2][3] * aMat1[3][1];
		temp[2][2] = aMat0[2][0] * aMat1[0][2] + aMat0[2][1] * aMat1[1][2] + aMat0[2][2] * aMat1[2][2] + aMat0[2][3] * aMat1[3][2];
		temp[2][3] = aMat0[2][0] * aMat1[0][3] + aMat0[2][1] * aMat1[1][3] + aMat0[2][2] * aMat1[2][3] + aMat0[2][3] * aMat1[3][3];
		temp[3][0] = aMat0[3][0] * aMat1[0][0] + aMat0[3][1] * aMat1[1][0] + aMat0[3][2] * aMat1[2][0] + aMat0[3][3] * aMat1[3][0];
		temp[3][1] = aMat0[3][0] * aMat1[0][1] + aMat0[3][1] * aMat1[1][1] + aMat0[3][2] * aMat1[2][1] + aMat0[3][3] * aMat1[3][1];
		temp[3][2] = aMat0[3][0] * aMat1[0][2] + aMat0[3][1] * aMat1[1][2] + aMat0[3][2] * aMat1[2][2] + aMat0[3][3] * aMat1[3][2];
		temp[3][3] = aMat0[3][0] * aMat1[0][3] + aMat0[3][1] * aMat1[1][3] + aMat0[3][2] * aMat1[2][3] + aMat0[3][3] * aMat1[3][3];*/

		/*temp(1, 1) = aMat0(1, 1) * aMat1(1, 1) + aMat0(1, 2) * aMat1(2, 1) + aMat0(1, 3) * aMat1(3, 1) + aMat0(1, 4) * aMat1(4, 1);
		temp(1, 2) = aMat0(1, 1) * aMat1(1, 2) + aMat0(1, 2) * aMat1(2, 2) + aMat0(1, 3) * aMat1(3, 2) + aMat0(1, 4) * aMat1(4, 2);
		temp(1, 3) = aMat0(1, 1) * aMat1(1, 3) + aMat0(1, 2) * aMat1(2, 3) + aMat0(1, 3) * aMat1(3, 3) + aMat0(1, 4) * aMat1(4, 3);
		temp(1, 4) = aMat0(1, 1) * aMat1(1, 4) + aMat0(1, 2) * aMat1(2, 4) + aMat0(1, 3) * aMat1(3, 4) + aMat0(1, 4) * aMat1(4, 4);
		temp(2, 1) = aMat0(2, 1) * aMat1(1, 1) + aMat0(2, 2) * aMat1(2, 1) + aMat0(2, 3) * aMat1(3, 1) + aMat0(2, 4) * aMat1(4, 1);
		temp(2, 2) = aMat0(2, 1) * aMat1(1, 2) + aMat0(2, 2) * aMat1(2, 2) + aMat0(2, 3) * aMat1(3, 2) + aMat0(2, 4) * aMat1(4, 2);
		temp(2, 3) = aMat0(2, 1) * aMat1(1, 3) + aMat0(2, 2) * aMat1(2, 3) + aMat0(2, 3) * aMat1(3, 3) + aMat0(2, 4) * aMat1(4, 3);
		temp(2, 4) = aMat0(2, 1) * aMat1(1, 4) + aMat0(2, 2) * aMat1(2, 4) + aMat0(2, 3) * aMat1(3, 4) + aMat0(2, 4) * aMat1(4, 4);
		temp(3, 1) = aMat0(3, 1) * aMat1(1, 1) + aMat0(3, 2) * aMat1(2, 1) + aMat0(3, 3) * aMat1(3, 1) + aMat0(3, 4) * aMat1(4, 1);
		temp(3, 2) = aMat0(3, 1) * aMat1(1, 2) + aMat0(3, 2) * aMat1(2, 2) + aMat0(3, 3) * aMat1(3, 2) + aMat0(3, 4) * aMat1(4, 2);
		temp(3, 3) = aMat0(3, 1) * aMat1(1, 3) + aMat0(3, 2) * aMat1(2, 3) + aMat0(3, 3) * aMat1(3, 3) + aMat0(3, 4) * aMat1(4, 3);
		temp(3, 4) = aMat0(3, 1) * aMat1(1, 4) + aMat0(3, 2) * aMat1(2, 4) + aMat0(3, 3) * aMat1(3, 4) + aMat0(3, 4) * aMat1(4, 4);
		temp(4, 1) = aMat0(4, 1) * aMat1(1, 1) + aMat0(4, 2) * aMat1(2, 1) + aMat0(4, 3) * aMat1(3, 1) + aMat0(4, 4) * aMat1(4, 1);
		temp(4, 2) = aMat0(4, 1) * aMat1(1, 2) + aMat0(4, 2) * aMat1(2, 2) + aMat0(4, 3) * aMat1(3, 2) + aMat0(4, 4) * aMat1(4, 2);
		temp(4, 3) = aMat0(4, 1) * aMat1(1, 3) + aMat0(4, 2) * aMat1(2, 3) + aMat0(4, 3) * aMat1(3, 3) + aMat0(4, 4) * aMat1(4, 3);
		temp(4, 4) = aMat0(4, 1) * aMat1(1, 4) + aMat0(4, 2) * aMat1(2, 4) + aMat0(4, 3) * aMat1(3, 4) + aMat0(4, 4) * aMat1(4, 4);*/

		temp[0] = aMat0[0] * aMat1[0] + aMat0[1] * aMat1[4] + aMat0[2] * aMat1[8] + aMat0[3] * aMat1[12];
		temp[1] = aMat0[0] * aMat1[1] + aMat0[1] * aMat1[5] + aMat0[2] * aMat1[9] + aMat0[3] * aMat1[13];
		temp[2] = aMat0[0] * aMat1[2] + aMat0[1] * aMat1[6] + aMat0[2] * aMat1[10] + aMat0[3] * aMat1[14];
		temp[3] = aMat0[0] * aMat1[3] + aMat0[1] * aMat1[7] + aMat0[2] * aMat1[11] + aMat0[3] * aMat1[15];
		temp[4] = aMat0[4] * aMat1[0] + aMat0[5] * aMat1[4] + aMat0[6] * aMat1[8] + aMat0[7] * aMat1[12];
		temp[5] = aMat0[4] * aMat1[1] + aMat0[5] * aMat1[5] + aMat0[6] * aMat1[9] + aMat0[7] * aMat1[13];
		temp[6] = aMat0[4] * aMat1[2] + aMat0[5] * aMat1[6] + aMat0[6] * aMat1[10] + aMat0[7] * aMat1[14];
		temp[7] = aMat0[4] * aMat1[3] + aMat0[5] * aMat1[7] + aMat0[6] * aMat1[11] + aMat0[7] * aMat1[15];
		temp[8] = aMat0[8] * aMat1[0] + aMat0[9] * aMat1[4] + aMat0[10] * aMat1[8] + aMat0[11] * aMat1[12];
		temp[9] = aMat0[8] * aMat1[1] + aMat0[9] * aMat1[5] + aMat0[10] * aMat1[9] + aMat0[11] * aMat1[13];
		temp[10] = aMat0[8] * aMat1[2] + aMat0[9] * aMat1[6] + aMat0[10] * aMat1[10] + aMat0[11] * aMat1[14];
		temp[11] = aMat0[8] * aMat1[3] + aMat0[9] * aMat1[7] + aMat0[10] * aMat1[11] + aMat0[11] * aMat1[15];
		temp[12] = aMat0[12] * aMat1[0] + aMat0[13] * aMat1[4] + aMat0[14] * aMat1[8] + aMat0[15] * aMat1[12];
		temp[13] = aMat0[12] * aMat1[1] + aMat0[13] * aMat1[5] + aMat0[14] * aMat1[9] + aMat0[15] * aMat1[13];
		temp[14] = aMat0[12] * aMat1[2] + aMat0[13] * aMat1[6] + aMat0[14] * aMat1[10] + aMat0[15] * aMat1[14];
		temp[15] = aMat0[12] * aMat1[3] + aMat0[13] * aMat1[7] + aMat0[14] * aMat1[11] + aMat0[15] * aMat1[15];

		return temp;
	}

	//matrix times scalar
	template <class T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix, const T& aScalar)
	{
		Matrix4x4<T> temp(false);

		temp[0] = aMatrix[0] * aScalar;
		temp[1] = aMatrix[1] * aScalar;
		temp[2] = aMatrix[2] * aScalar;
		temp[3] = aMatrix[3] * aScalar;
		temp[4] = aMatrix[4] * aScalar;
		temp[5] = aMatrix[5] * aScalar;
		temp[6] = aMatrix[6] * aScalar;
		temp[7] = aMatrix[7] * aScalar;
		temp[8] = aMatrix[8] * aScalar;
		temp[9] = aMatrix[9] * aScalar;
		temp[10] = aMatrix[10] * aScalar;
		temp[11] = aMatrix[11] * aScalar;
		temp[12] = aMatrix[12] * aScalar;
		temp[13] = aMatrix[13] * aScalar;
		temp[14] = aMatrix[14] * aScalar;
		temp[15] = aMatrix[15] * aScalar;

		return temp;
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		return
		{
			aVector.x * aMatrix[0] + aVector.y * aMatrix[4] + aVector.z * aMatrix[8] + aVector.w * aMatrix[12],
			aVector.x * aMatrix[1] + aVector.y * aMatrix[5] + aVector.z * aMatrix[9] + aVector.w * aMatrix[13],
			aVector.x * aMatrix[2] + aVector.y * aMatrix[6] + aVector.z * aMatrix[10] + aVector.w * aMatrix[14],
			aVector.x * aMatrix[3] + aVector.y * aMatrix[7] + aVector.z * aMatrix[11] + aVector.w * aMatrix[15]
		};
	}

	template <class T>
	Vector4<T>& operator*=(Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		aVector = aVector * aMatrix;
		return aVector;
	}

	template <class T>
	void Matrix4x4<T>::operator*=(const Matrix4x4<T>& aMatrix)
	{
		*this = *this * aMatrix;
	}

	template <class T>
	Matrix4x4<T>& Matrix4x4<T>::operator=(const Matrix4x4<T>& aMatrix)
	{
		if (this != &aMatrix)
		{
			memcpy(myMatrix, aMatrix.data(), sizeof(T) * 16);
		}

		return *this;
	}

	template<typename T>
	Matrix4x4<T>& Matrix4x4<T>::operator=(const T aArray[16])
	{
		memcpy(myMatrix, aArray, sizeof(T) * 16);
		return *this;
	}

	template <class T>
	bool operator==(const Matrix4x4<T>& aMat0, const Matrix4x4<T>& aMat1)
	{
		for (uint8_t i = 0; i < 16; ++i)
		{
			if (aMat0(i) != aMat1(i))
			{
				return false;
			}
		}

		return true;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateFromPosRotScale(const Vector3<T>& aTranslation, const Vector3<T>& aRotation, const Vector3<T>& aScale)
	{
		Matrix4x4<T> temp = Matrix4x4<T>::CreateScaleMatrix(aScale) * Matrix4x4<T>::CreateRotationMatrix(aRotation);
		temp(4, 1) = aTranslation.x;
		temp(4, 2) = aTranslation.y;
		temp(4, 3) = aTranslation.z;

		return temp;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateFromPosRotScale(const Vector3<float>& aTranslation, const Quaternion& aRotation, const Vector3<float>& aScale)
	{
		Matrix4x4<T> temp = Matrix4x4<T>::CreateScaleMatrix(aScale) * Matrix4x4<T>::CreateRotationMatrix(aRotation);
		temp(4, 1) = aTranslation.x;
		temp(4, 2) = aTranslation.y;
		temp(4, 3) = aTranslation.z;

		return temp;
	}


	template <class T>
	Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		Matrix4x4<T> temp;

		const T sine = sin(aAngleInRadians);
		const T cosine = cos(aAngleInRadians);

		temp(1, 1) = 1;
		temp(2, 2) = cosine;
		temp(2, 3) = sine;
		temp(3, 2) = -sine;
		temp(3, 3) = cosine;
		temp(4, 4) = 1;

		return temp;
	}

	template <class T>
	Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		Matrix4x4<T> temp;

		const T sine = sin(aAngleInRadians);
		const T cosine = cos(aAngleInRadians);

		temp(1, 1) = cosine;
		temp(1, 3) = -sine;
		temp(2, 2) = 1;
		temp(3, 1) = sine;
		temp(3, 3) = cosine;
		temp(4, 4) = 1;

		return temp;
	}

	template <class T>
	Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		Matrix4x4<T> temp;

		const T sine = sin(aAngleInRadians);
		const T cosine = cos(aAngleInRadians);

		temp(1, 1) = cosine;
		temp(1, 2) = sine;
		temp(2, 1) = -sine;
		temp(2, 2) = cosine;
		temp(3, 3) = 1;
		temp(4, 4) = 1;

		return temp;
	}

	//TODO: Optimize by not using 3 matrix multiplications, create the entire rotation matrix right away
	template<class T>
	Matrix4x4<T> Matrix4x4<T>::CreateRotationMatrix(const Vector3<T>& aEulerAngles)
	{
		Matrix4x4<T> rotationMatrix;

		rotationMatrix *= CreateRotationAroundX(DEGTORAD(aEulerAngles.x));
		rotationMatrix *= CreateRotationAroundY(DEGTORAD(aEulerAngles.y));
		rotationMatrix *= CreateRotationAroundZ(DEGTORAD(aEulerAngles.z));

		return rotationMatrix;
	}

	template <class T >
	Matrix4x4<float> Matrix4x4<T>::CreateRotationMatrix(const Quaternion& aQuaternion)
	{
		/*Matrix4x4<float> result;
		Vector3f right = aQuaternion * Vector3f(1, 0, 0);
		Vector3f up = aQuaternion * Vector3f(0, 1, 0);
		Vector3f forward = aQuaternion * Vector3f(0, 0, 1);
		result(1, 1) = right.x;
		result(1, 2) = right.y;
		result(1, 3) = right.z;
		result(2, 1) = up.x;
		result(2, 2) = up.y;
		result(2, 3) = up.z;
		result(3, 1) = forward.x;
		result(3, 2) = forward.y;
		result(3, 3) = forward.z;
		return result;*/

		//benne premature optimization moment
		Matrix4x4<float> result;
		result(1, 1) = 1 - 2 * aQuaternion.y * aQuaternion.y - 2 * aQuaternion.z * aQuaternion.z;
		result(1, 2) = 2 * aQuaternion.x * aQuaternion.y + 2 * aQuaternion.w * aQuaternion.z;
		result(1, 3) = 2 * aQuaternion.x * aQuaternion.z - 2 * aQuaternion.w * aQuaternion.y;
		result(2, 1) = 2 * aQuaternion.x * aQuaternion.y - 2 * aQuaternion.w * aQuaternion.z;
		result(2, 2) = 1 - 2 * aQuaternion.x * aQuaternion.x - 2 * aQuaternion.z * aQuaternion.z;
		result(2, 3) = 2 * aQuaternion.y * aQuaternion.z + 2 * aQuaternion.w * aQuaternion.x;
		result(3, 1) = 2 * aQuaternion.x * aQuaternion.z + 2 * aQuaternion.w * aQuaternion.y;
		result(3, 2) = 2 * aQuaternion.y * aQuaternion.z - 2 * aQuaternion.w * aQuaternion.x;
		result(3, 3) = 1 - 2 * aQuaternion.x * aQuaternion.x - 2 * aQuaternion.y * aQuaternion.y;
		return result;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateLookAt(const Vector3<T>& aEye, const Vector3<T>& aCenter, const Vector3<T>& aUp)
	{
		Vector3<T> forward = (aCenter - aEye).GetNormalized();
		Vector3<T> right = aUp.Cross(forward).GetNormalized();
		Vector3<T> newup = forward.Cross(right);

		Matrix4x4<T> result;

		result(1, 1) = right.x;
		result(1, 2) = newup.x;
		result(1, 3) = forward.x;
		result(1, 4) = 0;
		result(2, 1) = right.y;
		result(2, 2) = newup.y;
		result(2, 3) = forward.y;
		result(2, 4) = 0;
		result(3, 1) = right.z;
		result(3, 2) = newup.z;
		result(3, 3) = forward.z;
		result(3, 4) = 0;
		result(4, 1) = right.Dot({ -aEye.x,-aEye.y,-aEye.z });
		result(4, 2) = newup.Dot({ -aEye.x,-aEye.y,-aEye.z });
		result(4, 3) = forward.Dot({ -aEye.x,-aEye.y,-aEye.z });
		result(4, 4) = 1;

		return result;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateLookAtLH(const Vector3<T>& aEye, const Vector3<T>& aCenter, const Vector3<T>& aUp)
	{
		Vector3<T> forward = (aEye - aCenter).GetNormalized();
		Vector3<T> right = aUp.Cross(forward).GetNormalized();
		Vector3<T> newup = forward.Cross(right);
		Matrix4x4<T> result;

		result(1, 1) = right.x;
		result(1, 2) = newup.x;
		result(1, 3) = forward.x;
		result(1, 4) = 0;
		result(2, 1) = right.y;
		result(2, 2) = newup.y;
		result(2, 3) = forward.y;
		result(2, 4) = 0;
		result(3, 1) = right.z;
		result(3, 2) = newup.z;
		result(3, 3) = forward.z;
		result(3, 4) = 0;
		result(4, 1) = -right.Dot({ aEye.x, aEye.y, aEye.z });
		result(4, 2) = -newup.Dot({ aEye.x, aEye.y, aEye.z });
		result(4, 3) = -forward.Dot({ aEye.x, aEye.y, aEye.z });
		result(4, 4) = 1;

		return result;
	}

	template<class T>
	Matrix4x4<T> Matrix4x4<T>::CreateProjectionMatrixPerspective(float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane, float aFovDegrees)
	{
		Utils::Matrix4x4<float> projectionMatrix;

		const float hFoVRad = DEGTORAD(aFovDegrees);
		const float hFovCalc = std::tanf(hFoVRad * 0.5f);
		const float vFoVRad = 2 * std::atan(hFovCalc * (aResolutionY / aResolutionX));

		const float xScale = 1.0f / hFovCalc;
		const float yScale = 1.0f / std::tanf(vFoVRad * 0.5f);

		const float Q = aFarPlane / (aFarPlane - aNearPlane);

		projectionMatrix(1, 1) = xScale;
		projectionMatrix(2, 2) = yScale;
		projectionMatrix(3, 3) = Q;
		projectionMatrix(4, 3) = -Q * aNearPlane;
		projectionMatrix(3, 4) = 1.f / Q;
		projectionMatrix(4, 4) = 0;

		return projectionMatrix;
	}

	template<class T>
	Matrix4x4<T> Matrix4x4<T>::CreateLeftHandedProjectionMatrixPerspective(float aWidth, float aHeight, float aNearPlane, float aFarPlane, float aFovDegrees)
	{
		Matrix4x4<T> projectionMatrix;

		const float fovRadians = DEGTORAD(aFovDegrees);
		const float aspectRatio = aWidth / aHeight;
		const float yScale = 1.0f / tanf(fovRadians * 0.5f);
		const float xScale = yScale / aspectRatio;
		const float Q = aFarPlane / (aFarPlane - aNearPlane);

		projectionMatrix(1, 1) = xScale;
		projectionMatrix(2, 2) = yScale;
		projectionMatrix(3, 3) = Q;
		projectionMatrix(4, 3) = -Q * aNearPlane;
		projectionMatrix(3, 4) = 1.f;
		projectionMatrix(4, 4) = 0;

		return projectionMatrix;
	}

	template<class T>
	Matrix4x4<T> Matrix4x4<T>::CreateProjectionMatrixOrthographic(float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane)
	{
		Matrix4x4<T> orthographicProjection;

		orthographicProjection(1, 1) = 2.0f / aResolutionX;
		orthographicProjection(2, 2) = 2.0f / aResolutionY;
		orthographicProjection(3, 3) = 1.0f / (aFarPlane - aNearPlane);
		orthographicProjection(4, 3) = aNearPlane / (aNearPlane - aFarPlane);
		//orthographicProjection(4, 4) = 1.0f;

		return orthographicProjection;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateProjectionMatrixOrthographic(float aMinX, float aMaxX, float aMinY, float aMaxY, float aNearPlane, float aFarPlane)
	{
		Matrix4x4<T> mat;

		const float a = 2 / (aMaxX - aMinX);
		const float b = 2 / (aMaxY - aMinY);
		const float c = -2 / (aFarPlane - aNearPlane);
		const float d = -((aMaxX + aMinX) / (aMaxX - aMinX));
		const float e = -((aMaxY + aMinY) / (aMaxY - aMinY));
		const float f = -((aFarPlane + aNearPlane) / (aFarPlane - aNearPlane));

		mat(1, 1) = a;
		mat(2, 2) = b;
		mat(3, 3) = c;
		mat(4, 1) = d;
		mat(4, 2) = e;
		mat(4, 3) = f;
		mat(4, 4) = 1.f;

		return mat;
	}

	template<class T>
	void Matrix4x4<T>::Decompose(const Matrix4x4<T>& aMatrix, Vector3<T>& aOutPosition, Vector3<T>& aOutRotation, Vector3<T>& aOutScale)
	{
		aOutPosition = Vector3<T>(aMatrix(4, 1), aMatrix(4, 2), aMatrix(4, 3));

		Vector4<T> firstRow = Vector4<T>(aMatrix(1, 1), aMatrix(1, 2), aMatrix(1, 3), aMatrix(1, 4));
		Vector4<T> secondRow = Vector4<T>(aMatrix(2, 1), aMatrix(2, 2), aMatrix(2, 3), aMatrix(2, 4));
		Vector4<T> thirdRow = Vector4<T>(aMatrix(3, 1), aMatrix(3, 2), aMatrix(3, 3), aMatrix(3, 4));

		float xScale = firstRow.Length();
		float yScale = secondRow.Length();
		float zScale = thirdRow.Length();

		aOutScale.x = xScale;
		aOutScale.y = yScale;
		aOutScale.z = zScale;

		/*const bool xZero = xScale == 0;
		const bool yZero = yScale == 0;
		const bool zZero = zScale == 0;*/

		//assert(xScale != 0 && yScale != 0 && zScale != 0 && L"Benne håller på att fixa scale=0 problem");

		//if (xZero)
		//{
		//	if (yZero)
		//	{
		//		if (zZero)
		//		{
		//			// All 3 axis of scale are zero, the inputted matrix represents a 3d point, we cannot extract the rotation for a point
		//			aOutRotation.x = 0;
		//			aOutRotation.y = 0;
		//			aOutRotation.z = 0;

		//			aOutScale.x = 0;
		//			aOutScale.y = 0;
		//			aOutScale.z = 0;
		//			return;
		//		}
		//		else
		//		{
		//			//x && y == 0. set first-&secondrow to orthogonal vectors to z
		//			Vector4f third(thirdRow);
		//			Vector4f first = third * CreateRotationAroundY(90);
		//			firstRow = Vector3<T>(first.x, first.y, first.z);
		//			secondRow = firstRow.Cross(thirdRow);
		//		}
		//	}
		//	else if (zZero)
		//	{
		//		//x && z == 0. set first-&thirdrow to orthogonal vectors to y
		//		Vector4f second(secondRow);
		//		Vector4f first = second * CreateRotationAroundZ(90);
		//		firstRow = Vector3<T>(first.x, first.y, first.z);
		//		thirdRow = firstRow.Cross(secondRow);
		//	}
		//	else
		//	{
		//		//only x == 0, firstrow = Y x Z
		//		firstRow = secondRow.Cross(thirdRow);
		//	}
		//}
		//else if (yZero)
		//{
		//	if (zZero)
		//	{
		//		//y && z == 0. set second-&thirdrow to orthogonal vectors to x
		//		Vector4f first(firstRow);
		//		Vector4f second = first * CreateRotationAroundZ(90);
		//		secondRow = Vector3<T>(second.x, second.y, second.z);
		//		thirdRow = secondRow.Cross(firstRow);
		//	}
		//	else
		//	{
		//		//only y == 0, secondrow = X x Z
		//		secondRow = firstRow.Cross(thirdRow);
		//	}
		//}
		//else if (zZero)
		//{
		//	//Only z == 0, thirdrow = X x Y
		//	thirdRow = firstRow.Cross(secondRow);
		//}

		firstRow.Normalize();
		secondRow.Normalize();
		thirdRow.Normalize();

		float eulerX = atan2(secondRow.z, thirdRow.z);
		float eulerY = atan2(-firstRow.z, sqrt(secondRow.z * secondRow.z + thirdRow.z * thirdRow.z));
		float eulerZ = atan2(firstRow.y, firstRow.x);

		eulerY = asin(-firstRow.z);
		if (cos(eulerY) != 0)
		{
			eulerX = atan2(secondRow.z, thirdRow.z);
			eulerZ = atan2(firstRow.y, firstRow.x);
		}
		else
		{
			eulerX = atan2(-thirdRow.x, secondRow.y);
			eulerZ = 0;
		}

		aOutRotation.x = RADTODEG(eulerX);
		aOutRotation.y = RADTODEG(eulerY);
		aOutRotation.z = RADTODEG(eulerZ);
	}

	template <class T>
	inline void Matrix4x4<T>::Decompose(const Matrix4x4<float>& aMatrix, Vector3<float>& aOutPosition, Quaternion& aOutRotation, Vector3<float>& aOutScale)
	{
		Quaternion result;
		Vector3f right = Vector3f(aMatrix(1, 1), aMatrix(1, 2), aMatrix(1, 3));
		Vector3f up = Vector3f(aMatrix(2, 1), aMatrix(2, 2), aMatrix(2, 3)).GetNormalized();
		Vector3f forward = Vector3f(aMatrix(3, 1), aMatrix(3, 2), aMatrix(3, 3)).GetNormalized();
		right = up.Cross(forward);
		up = forward.Cross(right);
		aOutRotation = Quaternion::CreateLookRotation(forward, up);

		aOutPosition = Vector3f(aMatrix(4, 1), aMatrix(4, 2), aMatrix(4, 3));

		Vector3<float> firstRow = Vector3<float>(aMatrix(1, 1), aMatrix(1, 2), aMatrix(1, 3));
		Vector3<float> secondRow = Vector3<float>(aMatrix(2, 1), aMatrix(2, 2), aMatrix(2, 3));
		Vector3<float> thirdRow = Vector3<float>(aMatrix(3, 1), aMatrix(3, 2), aMatrix(3, 3));

		float xScale = firstRow.Length();
		float yScale = secondRow.Length();
		float zScale = thirdRow.Length();

		aOutScale.x = xScale;
		aOutScale.y = yScale;
		aOutScale.z = zScale;

	}

	template <class T>
	Matrix4x4<T> Matrix4x4<T>::Transpose(const Matrix4x4<T>& aMatrix)
	{
		Matrix4x4<T> temp;

		temp[0] = aMatrix[0];
		temp[1] = aMatrix[4];
		temp[2] = aMatrix[8];
		temp[3] = aMatrix[12];
		temp[4] = aMatrix[1];
		temp[5] = aMatrix[5];
		temp[6] = aMatrix[9];
		temp[7] = aMatrix[13];
		temp[8] = aMatrix[2];
		temp[9] = aMatrix[6];
		temp[10] = aMatrix[10];
		temp[11] = aMatrix[14];
		temp[12] = aMatrix[3];
		temp[13] = aMatrix[7];
		temp[14] = aMatrix[11];
		temp[15] = aMatrix[15];

		return temp;
	}

	template<class T>
	void Matrix4x4<T>::Transpose()
	{
		Matrix4x4<T> temp = *this;
		myMatrix[0] = temp[0];
		myMatrix[1] = temp[4];
		myMatrix[2] = temp[8];
		myMatrix[3] = temp[12];
		myMatrix[4] = temp[1];
		myMatrix[5] = temp[5];
		myMatrix[6] = temp[9];
		myMatrix[7] = temp[13];
		myMatrix[8] = temp[2];
		myMatrix[9] = temp[6];
		myMatrix[10] = temp[10];
		myMatrix[11] = temp[14];
		myMatrix[12] = temp[3];
		myMatrix[13] = temp[7];
		myMatrix[14] = temp[11];
		myMatrix[15] = temp[15];
	}

	template<class T>
	Matrix4x4<T> Matrix4x4<T>::Lerp(const Matrix4x4<T>& aFirstMatrix, const Matrix4x4<T>& aSecondMatrix, float aFactor)
	{
		Matrix4x4<T> result = aFirstMatrix;

		const Vector3f firstRight = { aFirstMatrix(1, 1), aFirstMatrix(1, 2), aFirstMatrix(1, 3) };
		const Vector3f secondRight = { aSecondMatrix(1, 1), aSecondMatrix(1, 2), aSecondMatrix(1, 3) };
		const Vector3f finalRight = Vector3<T>::NormalizedLerp(firstRight, secondRight, aFactor);

		result(1, 1) = finalRight.x;
		result(1, 2) = finalRight.y;
		result(1, 3) = finalRight.z;

		const Vector3f firstUp = { aFirstMatrix(2, 1), aFirstMatrix(2, 2), aFirstMatrix(2, 3) };
		const Vector3f secondUp = { aSecondMatrix(2, 1), aSecondMatrix(2, 2), aSecondMatrix(2, 3) };
		const Vector3f finalUp = Vector3<T>::NormalizedLerp(firstUp, secondUp, aFactor);

		result(2, 1) = finalUp.x;
		result(2, 2) = finalUp.y;
		result(2, 3) = finalUp.z;

		const Vector3f firstFront = { aFirstMatrix(3, 1), aFirstMatrix(3, 2), aFirstMatrix(3, 3) };
		const Vector3f secondFront = { aSecondMatrix(3, 1), aSecondMatrix(3, 2), aSecondMatrix(3, 3) };
		const Vector3f finalFront = Vector3<T>::NormalizedLerp(firstFront, secondFront, aFactor);

		result(3, 1) = finalFront.x;
		result(3, 2) = finalFront.y;
		result(3, 3) = finalFront.z;

		const Vector3<T> pos = { aFirstMatrix(1, 4), aFirstMatrix(2, 4), aFirstMatrix(3, 4) };
		const Vector3<T> secondPos = { aSecondMatrix(1, 4), aSecondMatrix(2, 4), aSecondMatrix(3, 4) };
		const Vector3<T> lerpedPos = Vector3<T>::Lerp(pos, secondPos, aFactor);

		result(1, 4) = lerpedPos.x;
		result(2, 4) = lerpedPos.y;
		result(3, 4) = lerpedPos.z;

		return result;
	}

	template <class T>
	Matrix4x4<T> Matrix4x4<T>::GetTranspose() const
	{
		Matrix4x4<T> temp;

		temp[0] = myMatrix[0];
		temp[1] = myMatrix[4];
		temp[2] = myMatrix[8];
		temp[3] = myMatrix[12];
		temp[4] = myMatrix[1];
		temp[5] = myMatrix[5];
		temp[6] = myMatrix[9];
		temp[7] = myMatrix[13];
		temp[8] = myMatrix[2];
		temp[9] = myMatrix[6];
		temp[10] = myMatrix[10];
		temp[11] = myMatrix[14];
		temp[12] = myMatrix[3];
		temp[13] = myMatrix[7];
		temp[14] = myMatrix[11];
		temp[15] = myMatrix[15];

		return temp;
	}

	template <class T>
	Matrix4x4<T> Matrix4x4<T>::GetFastInverse(const Matrix4x4<T>& aTransform)
	{
		Matrix4x4<T> tempTranslation;

		for (int i = 1; i <= 3; ++i)
		{
			tempTranslation(4, i) = aTransform(4, i) * -1;
		}

		Matrix4x4<T> tempRotation;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				tempRotation(j, i) = aTransform(i, j);
			}
		}

		return tempTranslation * tempRotation;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::GetInverse(const Matrix4x4<T>& aTransform)
	{
		T m[4][4];
		Matrix4x4<T> matrix = Transpose(aTransform);
		memcpy(&m, &matrix, sizeof(T) * 16);

		const T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		const T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		const T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		const T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		const T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		const T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		const T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		const T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		const T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		const T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		const T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		const T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		const T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		const T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		const T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		const T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		const T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		const T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		const Vector4<T> Fac0(Coef00, Coef00, Coef02, Coef03);
		const Vector4<T> Fac1(Coef04, Coef04, Coef06, Coef07);
		const Vector4<T> Fac2(Coef08, Coef08, Coef10, Coef11);
		const Vector4<T> Fac3(Coef12, Coef12, Coef14, Coef15);
		const Vector4<T> Fac4(Coef16, Coef16, Coef18, Coef19);
		const Vector4<T> Fac5(Coef20, Coef20, Coef22, Coef23);

		const Vector4<T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		const Vector4<T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		const Vector4<T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		const Vector4<T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

		const Vector4<T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
		const Vector4<T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
		const Vector4<T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
		const Vector4<T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

		const Vector4<T> SignA(+1, -1, +1, -1);
		const Vector4<T> SignB(-1, +1, -1, +1);
		const std::vector<Vector4<T>> inverse({ Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB });

		const Vector4<T> Dot0(m[0][0] * inverse[0][0], m[0][1] * inverse[1][0], m[0][2] * inverse[2][0], m[0][3] * inverse[3][0]);
		const T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

		const T OneOverDeterminant = static_cast<T>(1) / Dot1;

		memcpy(&matrix, inverse.data(), sizeof(T) * 16);

		matrix = matrix * OneOverDeterminant;
		matrix = Transpose(matrix);

		return matrix;
	}
}