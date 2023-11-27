#include "Quaternion.h"

#include "MathDefines.h"

namespace Utils
{
#define QUATERNION_EPSILON 0.00001f

	Quaternion Quaternion::CreateFromEulerAngles(const Vector3f& someEulerAngles)
	{
		return CreateFromEulerAngles(someEulerAngles.x, someEulerAngles.y, someEulerAngles.z);
	}

	Quaternion Quaternion::CreateFromEulerAngles(const float aX, const float aY, const float aZ)
	{
		const float pitch = DEGTORAD(aX);
		const float yaw = DEGTORAD(aY);
		const float roll = DEGTORAD(aZ);

		const float halfPitch = pitch * 0.5f;
		const float halfYaw = yaw * 0.5f;
		const float halfRoll = roll * 0.5f;

		const float cosX = cosf(halfPitch);
		const float cosY = cosf(halfYaw);
		const float cosZ = cosf(halfRoll);

		const float sinX = sinf(halfPitch);
		const float sinY = sinf(halfYaw);
		const float sinZ = sinf(halfRoll);

		//Don't know which one we should use

		//Object to world rotation
		return
		{
			cosY * sinX * cosZ + sinY * cosX * sinZ,
			sinY * cosX * cosZ - cosY * sinX * sinZ,
			cosY * cosX * sinZ - sinY * sinX * cosZ,
			cosY * cosX * cosZ + sinY * sinX * sinZ
		};

		//Upright to object rotation
		/*return
		{
			-cosY * sinX * cosZ - sinY * cosX * sinZ,
			cosY * sinX * sinZ - sinY * cosX * cosZ,
			sinY * sinX * cosZ - cosY * cosX * sinZ,
			cosY * cosX * cosZ + sinY * sinX * sinZ
		};*/

		//Wikipedia Y, X, Z
		/*return
		{
			sinZ * cosY * cosX - cosZ * sinY * sinX,
			cosZ * sinY * cosX + sinZ * cosY * sinX,
			cosZ * cosY * sinX - sinZ * sinY * cosX,
			cosZ * cosY * cosX + sinZ * sinY * sinX
		};*/
	}

	Quaternion Quaternion::CreateFromAxisAngle(const Vector3f& anAxis, float anAngle)
	{
		const Vector3f normalized = anAxis.GetNormalized();
		const float halfAngle = anAngle * 0.5f;
		const float sinHalfAngle = sinf(halfAngle);

		return { normalized.x * sinHalfAngle, normalized.y * sinHalfAngle, normalized.z * sinHalfAngle, cosf(halfAngle) };
	}

	Quaternion Quaternion::FromTo(const Vector3f& aFrom, const Vector3f& aTo)
	{
		auto from = aFrom.GetNormalized();
		auto to = aTo.GetNormalized();
		if (from.IsAlmostEqual(to))
		{
			return Quaternion();
		}

		if (from.IsAlmostEqual(to * -1.f))
		{
			Vector3f ortho = { 1,0,0 };

			if (std::fabsf(from.y) < std::fabsf(from.x))
			{
				ortho = { 0, 1, 0 };
			}
			if (std::fabsf(from.z) < std::fabsf(from.y) && std::fabsf(from.z) < std::fabsf(from.x))
			{
				ortho = { 0, 0, 1 };
			}
			auto axis = from.Cross(ortho).GetNormalized();
			return Quaternion(axis.x, axis.y, axis.z, 0);
		}
		Vector3f half = (from + to).GetNormalized();
		auto axis = from.Cross(half);
		return Quaternion(axis.x, axis.y, axis.z, from.Dot(half));
	}

	Vector3f Quaternion::GetEulerAngles() const
	{
		float sinPitch = -2.0f * (y * z - w * x);

		//out angles radians
		//heading, pitch, bank
		//y, x, z
		float h, p, b;

		//Assuming object to world quaternion
		if (std::fabsf(sinPitch) > 1 - QUATERNION_EPSILON) //Check for gimbal lock
		{
			p = PI * 0.5f * sinPitch;
			h = atan2f(-x * z + w * y, 0.5f - y * y - z * z);
			b = 0.0f;
		}
		else
		{
			p = asinf(sinPitch);
			h = atan2f(x * z + w * y, 0.5f - x * x - y * y);
			b = atan2f(x * y + w * z, 0.5f - x * x - z * z);
		}

		return { RADTODEG(p), RADTODEG(h), RADTODEG(b) };
		//

		//Assuming world to object quaternion
		//sinPitch = -2.0f * (y * z + w * x);
		//if (std::fabsf(sinPitch) > 1 - QUATERNION_EPSILON) //Check for gimbal lock
		//{
		//	p = PI * 0.5f * sinPitch;

		//	h = atan2f(-x * z - w * y, 0.5f - y * y - z * z);
		//	b = 0.0f;
		//}
		//else
		//{
		//	p = asinf(sinPitch);
		//	h = atan2f(x * z - w * y, 0.5f - x * x - y * y);
		//	b = atan2f(x * y - w * z, 0.5f - x * x - z * z);
		//}
		//return { RADTODEG(p), RADTODEG(h), RADTODEG(b) };
		//
	}

	Vector3f Quaternion::GetAxis(const Quaternion& aQuaternion)
	{
		return Vector3f(aQuaternion.x, aQuaternion.y, aQuaternion.z).GetNormalized();
	}

	float Quaternion::GetAngle(const Quaternion& aQuaternion)
	{
		return 2.0f * acosf(aQuaternion.w);
	}

	Vector3f Quaternion::GetAxis() const
	{
		return Vector3f(x, y, z).GetNormalized();
	}

	float Quaternion::GetAngle() const
	{
		return 2.0f * acosf(w);
	}

	bool Quaternion::IsSameOrientationAs(const Quaternion& aQuaternion) const
	{
		return
			(std::fabsf(x - aQuaternion.x) <= QUATERNION_EPSILON &&
				std::fabsf(y - aQuaternion.y) <= QUATERNION_EPSILON &&
				std::fabsf(z - aQuaternion.z) <= QUATERNION_EPSILON &&
				std::fabsf(w - aQuaternion.w) <= QUATERNION_EPSILON)
			||
			(std::fabsf(x + aQuaternion.x) <= QUATERNION_EPSILON &&
				std::fabsf(y + aQuaternion.y) <= QUATERNION_EPSILON &&
				std::fabsf(z + aQuaternion.z) <= QUATERNION_EPSILON &&
				std::fabsf(w + aQuaternion.w) <= QUATERNION_EPSILON);
	}

	bool Quaternion::IsSameOrientation(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return aLeft.IsSameOrientationAs(aRight);
	}

	float Quaternion::Dot(const Quaternion& aQuaternion) const
	{
		return x * aQuaternion.x + y * aQuaternion.y + z * aQuaternion.z + w * aQuaternion.w;
	}

	float Quaternion::Dot(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return aLeft.x * aRight.x + aLeft.y * aRight.y + aLeft.z * aRight.z + aLeft.w * aRight.w;
	}

	Quaternion Quaternion::GetConjugate() const
	{
		return { -x, -y, -z, w };
	}

	Quaternion Quaternion::GetInverse() const
	{
		const float lengthSqr = x * x + y * y + z * z + w * w;

		if (lengthSqr < QUATERNION_EPSILON)
		{
			return {};
		}

		const float factor = 1.0f / lengthSqr;
		return { -x * factor, -y * factor, -z * factor, w * factor };
	}

	float Quaternion::LengthSqr() const
	{
		return x * x + y * y + z * z + w * w;
	}

	float Quaternion::Length() const
	{
		const float lengthSqr = x * x + y * y + z * z + w * w;

		if (lengthSqr > 0.0f)
		{
			return sqrtf(lengthSqr);
		}

		return 0.0f;
	}

	void Quaternion::Normalize()
	{
		const float length = Length();

		if (length > 0.0f)
		{
			const float invLength = 1.0f / length;
			x *= invLength;
			y *= invLength;
			z *= invLength;
			w *= invLength;
		}
	}

	Quaternion Quaternion::GetNormalized() const
	{
		Quaternion result = *this;
		result.Normalize();
		return result;
	}

	Quaternion Quaternion::Lerp(const Quaternion& aFrom, const Quaternion& aTo, float aT)
	{
		return aFrom * (1.0f - aT) + aTo * aT;
	}

	Quaternion Quaternion::NLerp(const Quaternion& aFrom, const Quaternion& aTo, float aT)
	{
		return (aFrom + (aTo - aFrom) * aT).GetNormalized();
	}

	Quaternion Quaternion::SLerp(const Quaternion& aFrom, const Quaternion& aTo, float aT)
	{
		float cosAngle = aFrom.Dot(aTo);
		bool negateSecond = false;

		if (cosAngle < 0.0f)
		{
			negateSecond = true;
			cosAngle = -cosAngle;
		}

		float interp1, interp2;

		//Divide by zero check
		if (cosAngle > 1.0f - QUATERNION_EPSILON)
		{
			//Just use linear interpolation
			interp1 = 1.0f - aT;
			interp2 = aT;
		}
		else
		{
			const float sinAngle = sqrtf(1.0f - cosAngle * cosAngle);
			const float angle = atan2f(sinAngle, cosAngle);
			const float invSinAngle = 1.0f / sinAngle;

			interp1 = sinf((1.0f - aT) * angle) * invSinAngle;
			interp2 = sinf(aT * angle) * invSinAngle;
		}

		if (negateSecond)
		{
			interp2 = -interp2;
		}

		return { aFrom.x * interp1 + aTo.x * interp2, aFrom.y * interp1 + aTo.y * interp2, aFrom.z * interp1 + aTo.z * interp2, aFrom.w * interp1 + aTo.w * interp2 };
	}

	//TODO: Optimize?
	Quaternion Quaternion::CreateLookRotation(const Vector3f& aDirection, const Vector3f& aUp)
	{
		const Vector3f forward = aDirection.GetNormalized();
		Vector3f up = aUp.GetNormalized();
		const Vector3f right = up.Cross(forward);
		up = forward.Cross(right);

		//from world forward to Object forward
		const Quaternion f2d = FromTo({ 0,0,1 }, forward);

		//what direction is the new object up?
		const Vector3f objectUp = f2d * Vector3f(0, 1, 0);
		//from object up to desired up
		const Quaternion u2u = FromTo(objectUp, up);

		//rotate to forward direction first, then twist to correct up
		return (u2u * f2d).GetNormalized();
	}

	Quaternion operator*(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return
		{
			aLeft.w * aRight.x + aLeft.x * aRight.w + aLeft.y * aRight.z - aLeft.z * aRight.y,
			aLeft.w * aRight.y + aLeft.y * aRight.w + aLeft.z * aRight.x - aLeft.x * aRight.z,
			aLeft.w * aRight.z + aLeft.z * aRight.w + aLeft.x * aRight.y - aLeft.y * aRight.x,
			aLeft.w * aRight.w - aLeft.x * aRight.x - aLeft.y * aRight.y - aLeft.z * aRight.z
		};
	}

	Quaternion operator+(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return { aLeft.x + aRight.x, aLeft.y + aRight.y, aLeft.z + aRight.z, aLeft.w + aRight.w };
	}

	Quaternion operator-(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return { aLeft.x - aRight.x, aLeft.y - aRight.y, aLeft.z - aRight.z, aLeft.w - aRight.w };
	}

	Quaternion operator*(const Quaternion& aLeft, float aRight)
	{
		return { aLeft.x * aRight, aLeft.y * aRight, aLeft.z * aRight, aLeft.w * aRight };
	}

	Quaternion operator^(const Quaternion& aLeft, float anExponent)
	{
		if (fabsf(aLeft.w) < (1.0f - QUATERNION_EPSILON))
		{
			const float halfAngle = acosf(aLeft.w);
			const float newHalfAngle = halfAngle * anExponent;
			const float newW = cosf(newHalfAngle);

			const float multiplier = sinf(newHalfAngle) / sinf(halfAngle);
			return { aLeft.x * multiplier, aLeft.y * multiplier, aLeft.z * multiplier, newW };
		}

		return aLeft;
	}

	Vector3f operator*(const Quaternion& aQuat, const Vector3f& aVec)
	{
		return aQuat.vector * 2.0f * aQuat.vector.Dot(aVec) +
			aVec * (aQuat.scalar * aQuat.scalar - aQuat.vector.Dot(aQuat.vector)) +
			aQuat.vector.Cross(aVec) * 2.0f * aQuat.scalar;
	}

	Vector3f operator*(const Vector3f& aVec, const Quaternion& aQuat)
	{
		return aQuat * aVec;
	}

	Quaternion operator-(const Quaternion& aQuaternion)
	{
		return { -aQuaternion.x, -aQuaternion.y, -aQuaternion.z, -aQuaternion.w };
	}

	bool operator!=(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return !(aLeft == aRight);
	}

	bool operator==(const Quaternion& aLeft, const Quaternion& aRight)
	{
		return
			std::fabsf(aLeft.x - aRight.x) <= QUATERNION_EPSILON &&
			std::fabsf(aLeft.y - aRight.y) <= QUATERNION_EPSILON &&
			std::fabsf(aLeft.z - aRight.z) <= QUATERNION_EPSILON &&
			std::fabsf(aLeft.w - aRight.w) <= QUATERNION_EPSILON;
	}
}