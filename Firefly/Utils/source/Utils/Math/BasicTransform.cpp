#include "BasicTransform.h"

namespace Utils
{
	BasicTransform::BasicTransform() : myScale(1.0f, 1.0f, 1.0f)
	{
	}

	BasicTransform::BasicTransform(const Matrix4f& aMatrix)
	{
		Matrix4f::Decompose(aMatrix, myPosition, myRotation, myScale);
	}

	BasicTransform::BasicTransform(const Vector3f& aPosition, const Vector3f& aRotation, const Vector3f& aScale)
	{
		myPosition = aPosition;
		myRotation = Quaternion::CreateFromEulerAngles(aRotation);
		myScale = aScale;
	}

	BasicTransform::BasicTransform(const Vector3f& aPosition, const Quaternion& aRotation, const Vector3f& aScale)
	{
		myPosition = aPosition;
		myRotation = aRotation;
		myScale = aScale;
	}

	void BasicTransform::SetPosition(const Vector3f& aPosition)
	{
		myPosition = aPosition;
	}

	void BasicTransform::SetPosition(const float aX, const float aY, const float aZ)
	{
		myPosition.x = aX;
		myPosition.y = aY;
		myPosition.z = aZ;
	}

	void BasicTransform::SetXPosition(const float aX)
	{
		myPosition.x = aX;
	}

	void BasicTransform::SetYPosition(const float aY)
	{
		myPosition.y = aY;
	}

	void BasicTransform::SetZPosition(const float aZ)
	{
		myPosition.z = aZ;
	}

	void BasicTransform::SetRotation(const Quaternion& aRotation)
	{
		myRotation = aRotation;
	}

	void BasicTransform::SetRotation(const Vector3f& aRotation)
	{
		myRotation = Quaternion::CreateFromEulerAngles(aRotation);
	}

	void BasicTransform::SetRotation(const float aX, const float aY, const float aZ)
	{
		myRotation.x = aX;
		myRotation.y = aY;
		myRotation.z = aZ;
	}

	void BasicTransform::SetScale(const Vector3f& aScale)
	{
		myScale = aScale;
	}

	void BasicTransform::SetScale(const float aX, const float aY, const float aZ)
	{
		myScale.x = aX;
		myScale.y = aY;
		myScale.z = aZ;
	}

	void BasicTransform::SetXScale(const float aX)
	{
		myScale.x = aX;
	}

	void BasicTransform::SetYScale(const float aY)
	{
		myScale.y = aY;
	}

	void BasicTransform::SetZScale(const float aZ)
	{
		myScale.z = aZ;
	}

	Matrix4f BasicTransform::CreateMatrix() const
	{
		return Matrix4f::CreateFromPosRotScale(myPosition, myRotation, myScale);
	}

	Vector3f BasicTransform::GetPosition() const
	{
		return myPosition;
	}

	float BasicTransform::GetXPosition() const
	{
		return myPosition.x;
	}

	float BasicTransform::GetYPosition() const
	{
		return myPosition.y;
	}

	float BasicTransform::GetZPosition() const
	{
		return myPosition.z;
	}

	Quaternion BasicTransform::GetQuaternion() const
	{
		return myRotation;
	}

	Vector3f BasicTransform::GetRotation() const
	{
		return myRotation.GetEulerAngles();
	}

	Vector3f BasicTransform::GetScale() const
	{
		return myScale;
	}

	Vector3f BasicTransform::GetRight()
	{
		Matrix4f transform = CreateMatrix();
		return Vector3f(transform(1, 1), transform(1, 2), transform(1, 3)).GetNormalized();
	}

	Vector3f BasicTransform::GetLeft()
	{
		return GetRight() * -1.0f;
	}

	Vector3f BasicTransform::GetUp()
	{
		Matrix4f transform = CreateMatrix();
		return Vector3f(transform(2, 1), transform(2, 2), transform(2, 3)).GetNormalized();
	}

	Vector3f BasicTransform::GetDown()
	{
		return GetUp() * -1.0f;
	}

	Vector3f BasicTransform::GetForward()
	{
		Matrix4f transform = CreateMatrix();
		return Vector3f(transform(3, 1), transform(3, 2), transform(3, 3)).GetNormalized();
	}

	Vector3f BasicTransform::GetBackward()
	{
		return GetForward() * -1.0f;
	}

	void BasicTransform::LookAt(const Vector3f& aTargetPosition)
	{
		myRotation = Quaternion::CreateLookRotation(aTargetPosition - myPosition);
	}

	BasicTransform BasicTransform::Lerp(const BasicTransform& aTransform, const BasicTransform& anotherTransform, float aLerpFactor)
	{
		const Vector3f lerpedPos = Vector3f::Lerp(aTransform.GetPosition(), anotherTransform.GetPosition(), aLerpFactor);
		const Quaternion lerpedRot = Quaternion::SLerp(aTransform.GetQuaternion(), anotherTransform.GetQuaternion(), aLerpFactor);
		const Vector3f lerpedScale = Vector3f::Lerp(aTransform.GetScale(), anotherTransform.GetScale(), aLerpFactor);

		return { lerpedPos, lerpedRot, lerpedScale };
	}

	BasicTransform BasicTransform::Combine(const BasicTransform& aTransform, const BasicTransform& anotherTransform)
	{
		BasicTransform out;
		out.myScale = aTransform.myScale * anotherTransform.myScale;

		out.myRotation = aTransform.myRotation * anotherTransform.myRotation;

		out.myPosition = aTransform.myRotation * (aTransform.myScale * anotherTransform.myPosition);
		out.myPosition = aTransform.myPosition + out.myPosition;

		return out;
	}
	
	BasicTransform BasicTransform::Inverse(const BasicTransform& aTransform)
	{
		BasicTransform inv;
		inv.myRotation = aTransform.myRotation.GetInverse();
		inv.myScale.x = fabs(aTransform.myScale.x) < 0.0001f ? 0.0f : 1.0f / aTransform.myScale.x;
		inv.myScale.y = fabs(aTransform.myScale.y) < 0.0001f ? 0.0f : 1.0f / aTransform.myScale.y;
		inv.myScale.z = fabs(aTransform.myScale.z) < 0.0001f ? 0.0f : 1.0f / aTransform.myScale.z;
		Utils::Vector3f invTrans = aTransform.myPosition * -1.0f;
		inv.myPosition = inv.myRotation * (inv.myScale * invTrans);
		return inv;
	}
}