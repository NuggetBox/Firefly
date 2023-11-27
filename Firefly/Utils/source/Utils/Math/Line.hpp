#pragma once
#include "Vector2.hpp"

namespace Utils
{
	template <class T>
	class Line
	{
	public:
		// Default constructor: there is no line, the normal is the zero vector.
		Line();
		// Copy constructor.
		Line(const Line <T>& aLine);
		// Constructor that takes two points that define the line, the direction is aPoint1 - aPoint0.
		Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);
		// Init the line with two points, the same as the constructor above.
		void InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);
		// Init the line with a point and a direction.
		void InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection);
		// Returns whether a point is inside the line: it is inside when the point is on the line or on the side the normal is pointing away from.
		bool IsInside(const Vector2<T>& aPosition) const;
		// Returns true if 2 lines are fasing eachother
		bool IsFasing(const Line<T>& aLine) const;
		// Returns the origin of the line
		const Vector2<T>& GetOrigin() const;
		// Returns the direction of the line.
		const Vector2<T>& GetDirection() const;
		// Returns the normal of the line, which is (-direction.y, direction.x).
		const Vector2<T> GetNormal() const;
		// Returns the closesed point on line from given point
		const Vector2<T> GetClosesedPoint(const Vector2<T>& aPoint);
		// Returns the length of the line
		const T GetLength();
		const Vector2<T> GetEnd() const { return myOrigin + myDirection; }

		Vector2<T> GetMiddle() { return myMiddle; }

	private:

		Vector2<T> myOrigin;
		Vector2<T> myDirection;
		T myLength;
		Vector2<T> myMiddle;
	};

	template <class T>
	Line<T>::Line()
	{
		myOrigin = Vector2<T>(0, 0);
		myDirection = Vector2<T>(0, 0);
	}

	template <class T>
	Line<T>::Line(const Line<T>& aLine)
	{
		myOrigin = aLine.GetOrigin();
		myDirection = aLine.GetDirection();
		myLength = aLine.myLength;
		myMiddle = myOrigin + (myDirection / (T)2);
	}

	template <class T>
	Line<T>::Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myOrigin = aPoint0;
		myDirection = aPoint1 - aPoint0;
		myLength = myDirection.Length();
		myMiddle = myOrigin + (myDirection / (T)2);
	}

	template <class T>
	void Line<T>::InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myOrigin = aPoint0;
		myDirection = aPoint1 - aPoint0;
		myLength = myDirection.Length();
		myMiddle = myOrigin + (myDirection / (T)2);
	}

	template <class T>
	void Line<T>::InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection)
	{
		myOrigin = aPoint;
		myDirection = aDirection;
	}

	template <class T>
	bool Line<T>::IsInside(const Vector2<T>& aPosition) const
	{
		return GetNormal().Dot(aPosition - myOrigin) <= 0;
	}

	template<class T>
	inline bool Utils::Line<T>::IsFasing(const Line<T>& aLine) const
	{
		if (!IsInside(aLine.myMiddle))
		{
			if (!aLine.IsInside(myMiddle))
			{
				return true;
			}
		}

		return false;
	}

	template <class T>
	const Vector2<T>& Line<T>::GetOrigin() const
	{
		return myOrigin;
	}

	template <class T>
	const Vector2<T>& Line<T>::GetDirection() const
	{
		return myDirection;
	}

	template <class T>
	const Vector2<T> Line<T>::GetNormal() const
	{
		return Vector2<T>(-myDirection.y, myDirection.x);
	}

	template<class T>
	inline const Vector2<T> Utils::Line<T>::GetClosesedPoint(const Vector2<T>& aPoint)
	{
		Vector2<T> lhs = aPoint - myOrigin;
		float dotP = lhs.Dot(myDirection.GetNormalized());
		dotP = Clamp<T>(dotP, 0, myLength);

		return myOrigin + (myDirection.GetNormalized() * dotP);
	}

	template<class T>
	inline const T Utils::Line<T>::GetLength()
	{
		return myLength;
	}
}
