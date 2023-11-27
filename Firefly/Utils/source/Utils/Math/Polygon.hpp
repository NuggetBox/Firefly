#pragma once
#include "Vector3.hpp"
#include "Line.hpp"
#include "LineVolume.hpp"
#include "Vector2.hpp"

namespace Utils
{
	class Polygon
	{
	public:
		Polygon() = default;

		//Contructs a Polygon with the normal pointing in clockwise winding order of the points
		Polygon(const std::initializer_list<Vector3f>& somePoints);
		Polygon(const std::vector<Vector3f>& somePoints);
		void InitWithPoints(const std::initializer_list<Vector3f>& somePoints);
		void InitWithPoints(const std::vector<Vector3f>& somePoints);

		//Returns if a point is inside of the plane surface within a given tolerance distance
		bool IsInside(const Vector3f& aPoint, float aTolerance = 0.001f) const;

		int GetNumPoints() const;

		const Vector3f& GetNormal() const;

		const Vector3f& GetPoint(unsigned int anIndex) const;
		const Vector3f& operator[](unsigned int anIndex) const;

	private:
		void BuildLineVolume();
		void SetNormal();
		void VerifyPointCount() const;

		std::vector<Vector3f> myPoints;
		LineVolume<float> myLineVolume;
		Vector3f myNormal;
	};

	inline Polygon::Polygon(const std::initializer_list<Vector3f>& somePoints)
	{
		InitWithPoints(somePoints);
	}

	inline Polygon::Polygon(const std::vector<Vector3f>& somePoints)
	{
		InitWithPoints(somePoints);
	}

	inline void Polygon::InitWithPoints(const std::initializer_list<Vector3f>& somePoints)
	{
		for (auto& point : somePoints)
		{
			myPoints.push_back(point);
		}

		BuildLineVolume();
	}

	inline void Polygon::InitWithPoints(const std::vector<Vector3f>& somePoints)
	{
		for (auto& point : somePoints)
		{
			myPoints.push_back(point);
		}

		BuildLineVolume();
	}

	inline bool Polygon::IsInside(const Vector3f& aPoint, float aTolerance) const
	{
		const Vector3f planeToPoint = aPoint - myPoints[0];
		const float positionalDot = planeToPoint.Dot(GetNormal());

		// Check if point literally originates from inside the plane
		if (positionalDot > -aTolerance && positionalDot < aTolerance)
		{
			Vector2f point2d = { aPoint.x, aPoint.z };

			if (myLineVolume.IsInside(point2d))
			{
				return true;
			}
		}

		return false;
	}

	inline int Polygon::GetNumPoints() const
	{
		return myPoints.size();
	}

	inline const Vector3f& Polygon::GetNormal() const
	{
		return myNormal;
	}

	inline const Vector3f& Polygon::GetPoint(unsigned int anIndex) const
	{
		assert(anIndex < myPoints.size() && L"Point out of index in polygon");

		return myPoints[anIndex];
	}

	inline const Vector3f& Polygon::operator[](unsigned int anIndex) const
	{
		return GetPoint(anIndex);
	}

	inline void Polygon::BuildLineVolume()
	{
		VerifyPointCount();

		std::vector<Vector2f> points2d;
		points2d.reserve(myPoints.size());

		points2d.emplace_back(myPoints[0].x, myPoints[0].z);

		for (int i = 1; i < myPoints.size(); ++i)
		{
			points2d.emplace_back(myPoints[i].x, myPoints[i].z);
			Line<float> line(points2d[i - 1], points2d[i]);
			myLineVolume.AddLine(line);
		}

		Line<float> lastLine(points2d[points2d.size() - 1], points2d[0]);
		myLineVolume.AddLine(lastLine);

		SetNormal();
	}

	inline void Polygon::SetNormal()
	{
		VerifyPointCount();

		if (myPoints.size() >= 3)
		{
			myNormal = (myPoints[1] - myPoints[0]).Cross(myPoints[myPoints.size() - 1] - myPoints[0]).GetNormalized();
			constexpr float tolerance = 0.0001f;

			for (int i = 2; i < myPoints.size() - 1; i++)
			{
				Vector3f dir = myPoints[i] - myPoints[0];

				const float positionalDot = myNormal.Dot(dir);

				assert(positionalDot < tolerance || positionalDot > -tolerance && L"The points are not aligned on the same plane");
			}
		}
	}

	inline void Polygon::VerifyPointCount() const
	{
		assert(myPoints.size() >= 3);
	}
}
