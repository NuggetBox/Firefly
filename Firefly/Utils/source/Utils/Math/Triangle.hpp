#pragma once
#include "Vector3.hpp"

namespace Utils
{
	class Triangle
	{
	public:
		Triangle();

		//Contructs a triangle with the normal pointing in clockwise winding order of the 3 points
		Triangle(const Vector3f& aPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint);

		//Contructs a triangle with the normal pointing in clockwise winding order of the 3 points
		void InitWithThreePoints(const Vector3f& aPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint);

		//Returns if a point is inside of the plane surface within a given tolerance distance
		bool IsInside(const Vector3f& aPoint, float aTolerance = 0.001f) const;

		const Vector3f GetNormal() const;
		const Vector3f GetCenter() const;

		const Vector3f& GetFirstPoint() const;
		const Vector3f& GetSecondPoint() const;
		const Vector3f& GetThirdPoint() const;

	private:
		Vector3f myFirstPoint;
		Vector3f mySecondPoint;
		Vector3f myThirdPoint;
	};

	inline Triangle::Triangle()
	{
		myFirstPoint = Vector3f();
		mySecondPoint = Vector3f();
		myThirdPoint = Vector3f();
	}

	inline Triangle::Triangle(const Vector3f& aPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint)
	{
	}

	inline void Triangle::InitWithThreePoints(const Vector3f& aPoint, const Vector3f& aSecondPoint, const Vector3f& aThirdPoint)
	{
	}

	inline bool Triangle::IsInside(const Vector3f& aPoint, float aTolerance) const
	{
	}

	inline const Vector3f Triangle::GetNormal() const
	{
	}

	inline const Vector3f Triangle::GetCenter() const
	{

	}

	inline const Vector3f& Triangle::GetFirstPoint() const
	{
		return myFirstPoint;
	}

	inline const Vector3f& Triangle::GetSecondPoint() const
	{
		return mySecondPoint;
	}

	inline const Vector3f& Triangle::GetThirdPoint() const
	{
		return myThirdPoint;
	}
}
