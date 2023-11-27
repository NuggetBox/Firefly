#pragma once
#include "Utils/Math/Vector3.hpp"

namespace Utils
{
	class Quaternion
	{
	public:
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			struct
			{
				Vector3f vector;
				float scalar;
			};
			float v[4];
		};

		Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
		Quaternion(float aX, float aY, float aZ, float aW) : x(aX), y(aY), z(aZ), w(aW) {}

		static Quaternion CreateFromEulerAngles(const Vector3f& someEulerAngles);
		static Quaternion CreateFromEulerAngles(float aX, float aY, float aZ);
		static Quaternion CreateFromAxisAngle(const Vector3f& anAxis, float anAngle);
		static Quaternion FromTo(const Vector3f& aFrom, const Vector3f& aTo);

		Vector3f GetEulerAngles() const;

		Vector3f GetAxis() const;
		static Vector3f GetAxis(const Quaternion& aQuaternion);

		float GetAngle() const;
		static float GetAngle(const Quaternion& aQuaternion);

		bool IsSameOrientationAs(const Quaternion& aQuaternion) const;
		static bool IsSameOrientation(const Quaternion& aLeft, const Quaternion& aRight);

		float Dot(const Quaternion& aQuaternion) const;
		static float Dot(const Quaternion& aLeft, const Quaternion& aRight);

		Quaternion GetConjugate() const;
		Quaternion GetInverse() const;

		float LengthSqr() const;
		float Length() const;

		void Normalize();
		Quaternion GetNormalized() const;

		static Quaternion Lerp(const Quaternion& aFrom, const Quaternion& aTo, float aT);
		static Quaternion NLerp(const Quaternion& aFrom, const Quaternion& aTo, float aT);
		static Quaternion SLerp(const Quaternion& aFrom, const Quaternion& aTo, float aT);

		static Quaternion CreateLookRotation(const Vector3f& aDirection, const Vector3f& aUp = Vector3f::Up());
	};

	Quaternion operator*(const Quaternion& aLeft, const Quaternion& aRight);
	Quaternion operator+(const Quaternion& aLeft, const Quaternion& aRight);
	Quaternion operator-(const Quaternion& aLeft, const Quaternion& aRight);

	Quaternion operator*(const Quaternion& aLeft, float aRight);
	Quaternion operator^(const Quaternion& aLeft, float anExponent);

	Vector3f operator*(const Quaternion& aQuat, const Vector3f& aVec);
	Vector3f operator*(const Vector3f& aVec, const Quaternion& aQuat);

	Quaternion operator-(const Quaternion& aQuaternion);

	bool operator==(const Quaternion& aLeft, const Quaternion& aRight);
	bool operator!=(const Quaternion& aLeft, const Quaternion& aRight);
}