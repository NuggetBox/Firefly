#pragma once
#include <vector>

#include "BasicTransform.h"
#include "Matrix.h"

namespace Utils
{
	class Transform : public BasicTransform
	{
	public:
		Transform();
		Transform(const Matrix4f& aMatrix);
		Transform(const Vector3f& aPosition, const Vector3f& aRotation, const Vector3f& aScale);
		Transform(const Vector3f& aPosition, const Quaternion& aRotation, const Vector3f& aScale);
		Transform(const BasicTransform& aTransform);

		Transform(const Transform& aOther) = default;
		Transform(Transform&& aOther) = default;

		Transform& operator=(const Transform& aOther);
		Transform& operator=(Transform&& aOther) = default;

		void SetTransform(const Transform& aTransform);

		void SetPosition(const Vector3f& aPosition) override;
		void SetPosition(float aX, float aY, float aZ) override;
		void SetXPosition(float aX) override;
		void SetYPosition(float aY) override;
		void SetZPosition(float aZ) override;

		void SetRotation(const Quaternion& aQuaternion) override;
		void SetRotation(const Vector3f& aRotation) override;
		void SetRotation(float aX, float aY, float aZ) override;

		void SetScale(const Vector3f& aScale) override;
		void SetScale(float aX, float aY, float aZ) override;
		void SetXScale(float aX) override;
		void SetYScale(float aY) override;
		void SetZScale(float aZ) override;

		void SetLocalPosition(const Vector3f& aPosition);
		void SetLocalPosition(float aX, float aY, float aZ);
		void SetLocalXPosition(float aX);
		void SetLocalYPosition(float aY);
		void SetLocalZPosition(float aZ);

		void SetLocalRotation(const Quaternion& aQuaternion);
		void SetLocalRotation(const Vector3f& aRotation);
		void SetLocalRotation(float aX, float aY, float aZ);

		void SetLocalScale(const Vector3f& aScale);
		void SetLocalScale(float aX, float aY, float aZ);
		void SetLocalXScale(float aX);
		void SetLocalYScale(float aY);
		void SetLocalZScale(float aZ);

		void AddPosition(const Vector3f& aPositionDelta);
		void AddPosition(float aX, float aY, float aZ);

		void AddLocalPosition(const Vector3f& aPositionDelta);
		void AddLocalPosition(float aX, float aY, float aZ);

		void AddRotation(const Quaternion& aQuaternion);
		void AddRotation(const Vector3f& aRotationDelta);

		void AddLocalRotation(const Quaternion& aQuaternion);
		void AddLocalRotation(const Vector3f& aRotationDelta);

		//Scales the transform by multiplying with aScale
		void ScaleBy(const Vector3f& aScale);

		void LookAt(const Vector3f& aTargetPosition) override;

		//Converts the world parts of this transform to a BasicTransform
		Utils::BasicTransform ToBasic();

		const Matrix4f& GetMatrix();
		const Matrix4f& GetLocalMatrix();

		Vector3f GetPosition() const override;
		float GetXPosition() const override;
		float GetYPosition() const override;
		float GetZPosition() const override;

		const Vector3f& GetLocalPosition() const;
		float GetLocalXPosition() const;
		float GetLocalYPosition() const;
		float GetLocalZPosition() const;

		Quaternion GetQuaternion() const override;
		const Quaternion& GetLocalQuaternion() const;

		/**
		 * \brief Returns the global rotation in Euler Angles
		 * \return A Vector3 representing the global rotation in Euler Angles: X, Y, Z
		 */
		Vector3f GetRotation() const override;

		/**
		 * \brief Returns the local rotation in Euler Angles
		 * \return A Vector3 representing the local rotation in Euler Angles: X, Y, Z
		 */
		Vector3f GetLocalRotation() const;

		Vector3f GetScale() const override;
		const Vector3f& GetLocalScale() const;

		Vector3f GetRight() override;
		Vector3f GetLeft() override;

		Vector3f GetUp() override;
		Vector3f GetDown() override;

		Vector3f GetForward() override;
		Vector3f GetBackward() override;

		Vector3f GetLocalRight();
		Vector3f GetLocalLeft();

		Vector3f GetLocalUp();
		Vector3f GetLocalDown();

		Vector3f GetLocalForward();
		Vector3f GetLocalBackward();

		void SetAsModified();

		void SetParent(Transform* aParent, bool aKeepWorldTransformations = true);
		Transform* GetParent();
		void AddChild(Transform* aChild);
		void RemoveChild(const Transform* aChild);

		static Transform LocalLerp(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp = true);
		static Transform Lerp(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp = true);
		/**
		 * \brief Calls the EaseInOut in Lerp.hpp, however, only calls Slerp for the rotation
		 * \return 
		 */
		static Transform EaseInOut(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp = true);
		static Transform Combine(const Transform& aTransform, const Transform& anotherTransform);

	private:
		void RecalculateTransform();

		Transform* myParent = nullptr;
		std::vector<Transform*> myChildren;
		Matrix4f myLocalMatrix;
		Matrix4f myWorldMatrix;

		bool myWasModified;
	};
}
