#pragma once
#include "Matrix4x4.hpp"
#include "Vector3.hpp"

namespace Utils
{
	class BasicTransform
	{
	public:
		BasicTransform();
		BasicTransform(const Matrix4f& aMatrix);
		BasicTransform(const Vector3f& aPosition, const Vector3f& aRotation, const Vector3f& aScale);
		BasicTransform(const Vector3f& aPosition, const Quaternion& aRotation, const Vector3f& aScale);
		virtual ~BasicTransform() = default;

		virtual void SetPosition(const Vector3f& aPosition);
		virtual void SetPosition(float aX, float aY, float aZ);
		virtual void SetXPosition(float aX);
		virtual void SetYPosition(float aY);
		virtual void SetZPosition(float aZ);

		virtual void SetRotation(const Quaternion& aRotation);
		virtual void SetRotation(const Vector3f& aRotation);
		virtual void SetRotation(float aX, float aY, float aZ);

		virtual void SetScale(const Vector3f& aScale);
		virtual void SetScale(float aX, float aY, float aZ);
		virtual void SetXScale(float aX);
		virtual void SetYScale(float aY);
		virtual void SetZScale(float aZ);

		Matrix4f CreateMatrix() const;

		virtual Vector3f GetPosition() const;
		virtual float GetXPosition() const;
		virtual float GetYPosition() const;
		virtual float GetZPosition() const;

		virtual Quaternion GetQuaternion() const;

		virtual Vector3f GetRotation() const;

		virtual Vector3f GetScale() const;

		virtual Vector3f GetRight();
		virtual Vector3f GetLeft();

		virtual Vector3f GetUp();
		virtual Vector3f GetDown();

		virtual Vector3f GetForward();
		virtual Vector3f GetBackward();

		virtual void LookAt(const Vector3f& aTargetPosition);

		void* data() { return &myPosition; }

		static BasicTransform Lerp(const BasicTransform& aTransform, const BasicTransform& anotherTransform, float aLerpFactor);
		static BasicTransform Combine(const BasicTransform& aTransform, const BasicTransform& anotherTransform);
		static BasicTransform Inverse(const BasicTransform& aTransform);

	protected:
		Vector3f myPosition;
		Quaternion myRotation;
		Vector3f myScale;
	};
}