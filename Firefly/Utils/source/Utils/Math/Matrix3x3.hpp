#pragma once
#include <array>

#include "Matrix4x4.hpp"
#include "Vector3.hpp"
#include "Vector2.hpp"

namespace Utils
{
	template <class T>
	class Matrix3x3
	{
	public:
		// Creates the identity matrix.
		Matrix3x3<T>();
		// Copy Constructor.
		Matrix3x3<T>(const Matrix3x3<T>& aMatrix);
		// Copies the top left 3x3 part of the Matrix4x4.
		Matrix3x3<T>(const Matrix4x4<T>& aMatrix);

		// () operator for read/writing element (row, column)
		T& operator()(int aRow, int aColumn);
		// () operator for reading element (row, column)
		const T& operator()(int aRow, int aColumn) const;

		Vector3<T> GetRowVector(int aRow) const;
		Vector3<T> GetColumnVector(int aColumn) const;

		void operator+=(const Matrix3x3<T>& aMatrix);
		void operator-=(const Matrix3x3<T>& aMatrix);
		void operator*=(const Matrix3x3<T>& aMatrix);
		Matrix3x3<T>& operator=(const Matrix3x3<T>& aMatrix);

		// Create a rotation matrix that rotates a Vector3 around the X axis
		static Matrix3x3<T> CreateRotationAroundX(T aAngleRadians);
		// Create a rotation matrix that rotates a Vector3 around the Y axis
		static Matrix3x3<T> CreateRotationAroundY(T aAngleRadians);
		// Create a rotation matrix that rotates a Vector3 around the Z axis
		static Matrix3x3<T> CreateRotationAroundZ(T aAngleRadians);
		
		static Matrix3x3<T> CreateScaleMatrix(const Vector2<T>& aScale);

		//X, Y, Z rotation in degrees
		static Matrix3x3<T> CreateRotationMatrix(Vector3<T> aRotationVector);
		static Matrix3x3<T> CreateRotationMatrix(const Quaternion& aQuaternion);

		// Static function for creating a transpose of a matrix.
		static Matrix3x3<T> Transpose(const Matrix3x3<T>& aMatrixToTranspose);

		static Matrix3x3<T> GetFastInverse(const Matrix3x3<T>& aMatrixToInverse);
		
		//inverse
		static Matrix3x3<T> GetInverse(const Matrix3x3<T>& aMatrixToInverse);

	private:
		std::array<Vector3<T>, 3> myMatrix;
	};

	typedef Matrix3x3<float> Matrix3f;

	template <class T>
	Matrix3x3<T>::Matrix3x3()
	{
		myMatrix[0] = Vector3<T>(1, 0, 0);
		myMatrix[1] = Vector3<T>(0, 1, 0);
		myMatrix[2] = Vector3<T>(0, 0, 1);
	}

	template <class T>
	Matrix3x3<T>::Matrix3x3(const Matrix3x3<T>& aMatrix)
	{
		*this = aMatrix;
	}

	template <class T>
	Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& aMatrix)
	{
		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				myMatrix[i - 1][j - 1] = aMatrix(i, j);
			}
		}
	}

	template <class T>
	T& Matrix3x3<T>::operator()(const int aRow, const int aColumn)
	{
		assert(aRow >= 1 && aRow <= 3 && "Row index out of range, allowed indexes for Matrix3x3 row: 1, 2, 3");
		assert(aColumn >= 1 && aColumn <= 3 && "Column index out of range, allowed indexes for Matrix3x3 column: 1, 2, 3");

		return myMatrix[aRow - 1][aColumn - 1];
	}

	template <class T>
	const T& Matrix3x3<T>::operator()(const int aRow, const int aColumn) const
	{
		assert(aRow >= 1 && aRow <= 3 && "Row index out of range, allowed indexes for Matrix3x3 row: 1, 2, 3");
		assert(aColumn >= 1 && aColumn <= 3 && "Column index out of range, allowed indexes for Matrix3x3 column: 1, 2, 3");

		return myMatrix[aRow - 1][aColumn - 1];
	}

	template <class T>
	Vector3<T> Matrix3x3<T>::GetRowVector(const int aRow) const
	{
		assert(aRow >= 1 && aRow <= 3 && "Row index out of range, allowed indexes for Matrix3x3 row: 1, 2, 3");

		return myMatrix[aRow - 1];
	}

	template <class T>
	Vector3<T> Matrix3x3<T>::GetColumnVector(const int aColumn) const
	{
		assert(aColumn >= 1 && aColumn <= 3 && "Column index out of range, allowed indexes for Matrix3x3 column: 1, 2, 3");

		switch (aColumn)
		{
			case 1:
				return Vector3<T>(myMatrix[0].x, myMatrix[1].x, myMatrix[2].x);
			case 2:
				return Vector3<T>(myMatrix[0].y, myMatrix[1].y, myMatrix[2].y);
			case 3:
				return Vector3<T>(myMatrix[0].z, myMatrix[1].z, myMatrix[2].z);
		}
	}

	template <class T>
	Matrix3x3<T> operator+(const Matrix3x3<T>& aFirstMatrix, const Matrix3x3<T>& aSecondMatrix)
	{
		Matrix3x3<T> result;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				result(i, j) = aFirstMatrix(i, j) + aSecondMatrix(i, j);
			}
		}

		return result;
	}

	template <class T>
	void Matrix3x3<T>::operator+=(const Matrix3x3<T>& aMatrix)
	{
		*this = (*this) + aMatrix;
	}

	template <class T>
	Matrix3x3<T> operator-(const Matrix3x3<T>& aFirstMatrix, const Matrix3x3<T>& aSecondMatrix)
	{
		Matrix3x3<T> result;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				result(i, j) = aFirstMatrix(i, j) - aSecondMatrix(i, j);
			}
		}

		return result;
	}

	template <class T>
	void Matrix3x3<T>::operator-=(const Matrix3x3<T>& aMatrix)
	{
		*this = (*this) - aMatrix;
	}

	template <class T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aFirstMatrix, const Matrix3x3<T>& aSecondMatrix)
	{
		Matrix3x3<T> temp;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				temp(i, j) = aFirstMatrix.GetRowVector(i).Dot(aSecondMatrix.GetColumnVector(j));
			}
		}

		return temp;
	}

	template <class T>
	Vector3<T> operator*(const Vector3<T>& aVector, const Matrix3x3<T>& aMatrix)
	{
		Vector3<T> temp;

		temp.x = aVector.Dot(aMatrix.GetColumnVector(1));
		temp.y = aVector.Dot(aMatrix.GetColumnVector(2));
		temp.z = aVector.Dot(aMatrix.GetColumnVector(3));

		return temp;
	}
	
	template <class T>
	Matrix3x3<T> operator*(const T& aScalar, const Matrix3x3<T>& aMatrix)
	{
		Matrix3x3<T> temp;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				temp(i, j) = aMatrix(i, j) * aScalar;
			}
		}

		return temp;
	}
	
	template <class T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix, const T& aScalar)
	{
		Matrix3x3<T> temp;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				temp(i, j) = aMatrix(i, j) * aScalar;
			}
		}

		return temp;
	}

	template <class T>
	void Matrix3x3<T>::operator*=(const Matrix3x3<T>& aMatrix)
	{
		*this = *this * aMatrix;
	}

	template <class T>
	Matrix3x3<T>& Matrix3x3<T>::operator=(const Matrix3x3<T>& aMatrix)
	{
		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				myMatrix[i - 1][j - 1] = aMatrix(i, j);
			}
		}

		return *this;
	}

	template <class T>
	bool operator==(const Matrix3x3<T>& aFirstMatrix, const Matrix3x3<T>& aSecondMatrix)
	{
		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				if (aFirstMatrix(i, j) != aSecondMatrix(i, j))
				{
					return false;
				}
			}
		}

		return true;
	}

	template <class T>
	Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundX(T aAngleRadians)
	{
		Matrix3x3<T> temp;

		T sine = sin(aAngleRadians);
		T cosine = cos(aAngleRadians);

		temp(1, 1) = 1;
		//temp(1, 2) = 0;
		//temp(1, 3) = 0;
		
		//temp(2, 1) = 0;
		temp(2, 2) = cosine;
		temp(2, 3) = sine;

		//temp(3, 1) = 0;
		temp(3, 2) = -sine;
		temp(3, 3) = cosine;

		return temp;
	}

	template <class T>
	Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundY(T aAngleRadians)
	{
		Matrix3x3<T> temp;

		T sine = sin(aAngleRadians);
		T cosine = cos(aAngleRadians);

		temp(1, 1) = cosine;
		//temp(1, 2) = 0;
		temp(1, 3) = -sine;

		//temp(2, 1) = 0;
		temp(2, 2) = 1;
		//temp(2, 3) = 0;

		temp(3, 1) = sine;
		//temp(3, 2) = 0;
		temp(3, 3) = cosine;

		return temp;
	}

	template <class T>
	Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundZ(T aAngleRadians)
	{
		Matrix3x3<T> temp;

		T sine = sin(aAngleRadians);
		T cosine = cos(aAngleRadians);

		temp(1, 1) = cosine;
		temp(1, 2) = sine;
		//temp(1, 3) = 0;

		temp(2, 1) = -sine;
		temp(2, 2) = cosine;
		//temp(2, 3) = 0;
		
		//temp(3, 1) = 0;
		//temp(3, 2) = 0;
		temp(3, 3) = 1;

		return temp;
	}

	template<class T>
	Matrix3x3<T> Matrix3x3<T>::CreateRotationMatrix(Vector3<T> aRotationVector)
	{
		Matrix3x3<T> rotationMatrix;

		rotationMatrix *= CreateRotationAroundX(DEGTORAD(aRotationVector.x));
		rotationMatrix *= CreateRotationAroundY(DEGTORAD(aRotationVector.y));
		rotationMatrix *= CreateRotationAroundZ(DEGTORAD(aRotationVector.z));

		return rotationMatrix;
	}

	template<class T>
	Matrix3x3<T> Matrix3x3<T>::CreateRotationMatrix(const Quaternion& aQuaternion)
	{
		Matrix3x3<float> result;
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
	inline Matrix3x3<T> Matrix3x3<T>::CreateScaleMatrix(const Vector2<T>& aScale)
	{
		Matrix3x3<T> scaleMatrix;
		
		scaleMatrix(1, 1) = aScale.x;
		scaleMatrix(2, 2) = aScale.y;
		return scaleMatrix;
	}

	template <class T>
	Matrix3x3<T> Matrix3x3<T>::Transpose(const Matrix3x3<T>& aMatrixToTranspose)
	{
		Matrix3x3<T> temp;

		for (int i = 1; i <= 3; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				temp(j, i) = aMatrixToTranspose(i, j);
			}
		}

		return temp;
	}


	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::GetInverse(const Matrix3x3<T>& aMatrixToInverse)
	{
		Matrix3x3<T> inverseMatrix;

		//det
		T det = aMatrixToInverse(1, 1) * (aMatrixToInverse(2, 2) * aMatrixToInverse(3, 3) - aMatrixToInverse(2, 3) * aMatrixToInverse(3, 2))
			- aMatrixToInverse(1, 2) * (aMatrixToInverse(2, 1) * aMatrixToInverse(3, 3) - aMatrixToInverse(2, 3) * aMatrixToInverse(3, 1))
			+ aMatrixToInverse(1, 3) * (aMatrixToInverse(2, 1) * aMatrixToInverse(3, 2) - aMatrixToInverse(2, 2) * aMatrixToInverse(3, 1));

		if (det != 0)
		{
			inverseMatrix(1, 1) = aMatrixToInverse(2, 2) * aMatrixToInverse(3, 3) - aMatrixToInverse(2, 3) * aMatrixToInverse(3, 2);
			inverseMatrix(1, 2) = aMatrixToInverse(1, 3) * aMatrixToInverse(3, 2) - aMatrixToInverse(1, 2) * aMatrixToInverse(3, 3);
			inverseMatrix(1, 3) = aMatrixToInverse(1, 2) * aMatrixToInverse(2, 3) - aMatrixToInverse(1, 3) * aMatrixToInverse(2, 2);

			inverseMatrix(2, 1) = aMatrixToInverse(2, 3) * aMatrixToInverse(3, 1) - aMatrixToInverse(2, 1) * aMatrixToInverse(3, 3);
			inverseMatrix(2, 2) = aMatrixToInverse(1, 1) * aMatrixToInverse(3, 3) - aMatrixToInverse(1, 3) * aMatrixToInverse(3, 1);
			inverseMatrix(2, 3) = aMatrixToInverse(1, 3) * aMatrixToInverse(2, 1) - aMatrixToInverse(1, 1) * aMatrixToInverse(2, 3);

			inverseMatrix(3, 1) = aMatrixToInverse(2, 1) * aMatrixToInverse(3, 2) - aMatrixToInverse(2, 2) * aMatrixToInverse(3, 1);
			inverseMatrix(3, 2) = aMatrixToInverse(1, 2) * aMatrixToInverse(3, 1) - aMatrixToInverse(1, 1) * aMatrixToInverse(3, 2);
			inverseMatrix(3, 3) = aMatrixToInverse(1, 1) * aMatrixToInverse(2, 2) - aMatrixToInverse(1, 2) * aMatrixToInverse(2, 1);

			inverseMatrix = inverseMatrix * (1/ det);

		}
		
		return inverseMatrix;
	}


	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::GetFastInverse(const Matrix3x3<T>& aMatrixToInverse)
	{
		Matrix3x3<T> temp = aMatrixToInverse;
		
		Vector3f pos = aMatrixToInverse.GetRowVector(3);
		pos *= -1;
		
		temp(3, 1) = 0;
		temp(3, 2) = 0;

		Transpose(temp);
		
		temp(3, 1) = pos.x;
		temp(3, 2) = pos.y;
		
		
		return temp;
	}

		
}