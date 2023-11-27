#include "Transform.h"

#include "Vector.h"

namespace Utils
{
#define TRANSFORM_EPSILON (std::numeric_limits<float>::epsilon())

	Transform::Transform()
	{
		RecalculateTransform();
	}

	Transform::Transform(const Matrix4f& aMatrix) : BasicTransform(aMatrix)
	{
		RecalculateTransform();
	}

	Transform::Transform(const Vector3f& aPosition, const Vector3f& aRotation, const Vector3f& aScale) : BasicTransform(aPosition, aRotation, aScale)
	{
		RecalculateTransform();
	}

	Transform::Transform(const Vector3f& aPosition, const Quaternion& aRotation, const Vector3f& aScale) : BasicTransform(aPosition, aRotation, aScale)
	{
		RecalculateTransform();
	}

	Transform::Transform(const BasicTransform& aTransform) :
		BasicTransform(aTransform.GetPosition(), aTransform.GetQuaternion(), aTransform.GetScale())
	{
		RecalculateTransform();
	}

	Transform& Transform::operator=(const Transform& aOther)
	{
		if (this == &aOther)
		{
			return *this;
		}

		myParent = aOther.myParent;
		myChildren = aOther.myChildren;
		myLocalMatrix = aOther.myLocalMatrix;
		myWorldMatrix = aOther.myWorldMatrix;
		myWasModified = aOther.myWasModified;
		myPosition = aOther.myPosition;
		myRotation = aOther.myRotation;
		myScale = aOther.myScale;

		SetAsModified();

		return *this;
	}

	void Transform::SetTransform(const Transform& aTransform)
	{
		SetPosition(aTransform.GetPosition());
		SetRotation(aTransform.GetQuaternion());
		SetScale(aTransform.GetScale());
		SetAsModified();
	}

	void Transform::SetPosition(const Vector3f& aPosition)
	{
		SetPosition(aPosition.x, aPosition.y, aPosition.z);
	}

	void Transform::SetPosition(const float aX, const float aY, const float aZ)
	{
		Vector3f newPos(aX, aY, aZ);

		if (myParent)
		{
			const Vector4f posVec4 = { aX, aY, aZ, 1 };
			const Vector4f relativeToCurParent = posVec4 * Matrix4f::GetInverse(myParent->GetMatrix());
			newPos = Vec4ToVec3(relativeToCurParent);
		}

		if (myPosition.IsAlmostEqual(newPos, TRANSFORM_EPSILON))
		{
			return;
		}
		myPosition = newPos;
		SetAsModified();
	}

	void Transform::SetXPosition(const float aX)
	{
		SetPosition(aX, GetYPosition(), GetZPosition());

		//float newX = aX;

		//if (myParent)
		//{
		//	const Vector4f posVec4 = { aX, GetYPosition(), GetZPosition(), 1 };
		//	const Vector4f relativeToCurParent = posVec4 * Matrix4f::GetFastInverse(myParent->GetMatrix()); //Does scale matter? If so, use GetInverse instead
		//	newX = relativeToCurParent.x;
		//}

		//if (Abs(myPosition.x - newX) > TRANSFORM_EPSILON)
		//{
		//	SetAsModified();
		//}

		//myPosition.x = newX;
	}

	void Transform::SetYPosition(const float aY)
	{
		SetPosition(GetXPosition(), aY, GetZPosition());

		//float newY = aY;

		//if (myParent)
		//{
		//	const Vector4f posVec4 = { GetXPosition(), aY, GetZPosition(), 1 };
		//	const Vector4f relativeToCurParent = posVec4 * Matrix4f::GetFastInverse(myParent->GetMatrix()); //Does scale matter? If so, use GetInverse instead
		//	newY = relativeToCurParent.y;
		//}

		//if (Abs(myPosition.x - newY) > TRANSFORM_EPSILON)
		//{
		//	SetAsModified();
		//}

		//myPosition.y = newY;
	}

	void Transform::SetZPosition(const float aZ)
	{
		SetPosition(GetXPosition(), GetYPosition(), aZ);

		//float newZ = aZ;

		//if (myParent)
		//{
		//	const Vector4f posVec4 = { GetXPosition(), GetYPosition(), newZ, 1 };
		//	const Vector4f relativeToCurParent = posVec4 * Matrix4f::GetFastInverse(myParent->GetMatrix()); //Does scale matter? If so, use GetInverse instead
		//	newZ = relativeToCurParent.z;
		//}

		//if (Abs(myPosition.x - newZ) > TRANSFORM_EPSILON)
		//{
		//	SetAsModified();
		//}

		//myPosition.z = newZ;
	}

	void Transform::SetRotation(const Quaternion& aQuaternion)
	{
		Quaternion newRotation = aQuaternion;

		if (myParent)
		{
			newRotation = myParent->GetQuaternion().GetInverse() * aQuaternion; //TODO: SetRotation, Unsure about this one, other multiplication order maybe
		}

		if (newRotation.IsSameOrientationAs(myRotation))
		{
			return;
		}

		myRotation = newRotation;
		SetAsModified();
	}

	void Transform::SetRotation(const Vector3f& aRotation)
	{
		SetRotation(Quaternion::CreateFromEulerAngles(aRotation));
	}

	void Transform::SetRotation(float aX, float aY, float aZ)
	{
		SetRotation(Quaternion::CreateFromEulerAngles(aX, aY, aZ));
	}

	void Transform::SetScale(const Vector3f& aScale)
	{
		SetScale(aScale.x, aScale.y, aScale.z);

		/*Vector3f newLocalScale;

		if (myParent)
		{
			const Vector3f parentScale = myParent->GetScale();
			newLocalScale = aScale / parentScale;
		}
		else
		{
			newLocalScale = aScale;
		}

		if (!myScale.IsAlmostEqual(newLocalScale, TRANSFORM_EPSILON))
		{
			SetAsModified();

			myScale = newLocalScale;
		}*/
	}

	void Transform::SetScale(float aX, float aY, float aZ)
	{
		Vector3f newLocalScale;

		if (myParent)
		{
			const Vector3f parentScale = myParent->GetScale();
			newLocalScale.x = aX / parentScale.x;
			newLocalScale.y = aY / parentScale.y;
			newLocalScale.z = aZ / parentScale.z;
		}
		else
		{
			newLocalScale.x = aX;
			newLocalScale.y = aY;
			newLocalScale.z = aZ;
		}
		if (myScale.IsAlmostEqual(newLocalScale, TRANSFORM_EPSILON))
		{
			return;
		}
		myScale = newLocalScale;
		SetAsModified();
	}

	void Transform::SetXScale(float aX)
	{
		SetScale(aX, GetScale().y, GetScale().z);
	}

	void Transform::SetYScale(float aY)
	{
		SetScale(GetScale().x, aY, GetScale().z);
	}

	void Transform::SetZScale(float aZ)
	{
		SetScale(GetScale().x, GetScale().y, aZ);
	}

	void Transform::SetLocalPosition(const Vector3f& aPosition)
	{
		/*if ((myPosition - aPosition).LengthSqr() > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/


		if (myPosition.IsAlmostEqual(aPosition, TRANSFORM_EPSILON))
		{
			return;
		}

		myPosition = aPosition;
		SetAsModified();
	}

	void Transform::SetLocalPosition(const float aX, const float aY, const float aZ)
	{
		/*if (Abs(myPosition.x - aX) > TRANSFORM_EPSILON ||
			Abs(myPosition.y - aY) > TRANSFORM_EPSILON ||
			Abs(myPosition.z - aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/


		if ((Abs(myPosition.x - aX) < TRANSFORM_EPSILON &&
			Abs(myPosition.y - aY) < TRANSFORM_EPSILON &&
			Abs(myPosition.z - aZ) < TRANSFORM_EPSILON))
		{
			return;
		}


		myPosition.x = aX;
		myPosition.y = aY;
		myPosition.z = aZ;
		SetAsModified();
	}

	void Transform::SetLocalXPosition(const float aX)
	{
		/*if (Abs(myPosition.x - aX) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(myPosition.x - aX) < TRANSFORM_EPSILON)
		{
			return;
		}


		myPosition.x = aX;
		SetAsModified();
	}

	void Transform::SetLocalYPosition(const float aY)
	{
		/*if (Abs(myPosition.y - aY) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/
		if (Abs(myPosition.y - aY) < TRANSFORM_EPSILON)
		{
			return;
		}
		myPosition.y = aY;
		SetAsModified();
	}

	void Transform::SetLocalZPosition(const float aZ)
	{
		/*if (Abs(myPosition.z - aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/
		if (Abs(myPosition.z - aZ) < TRANSFORM_EPSILON)
		{
			return;
		}

		myPosition.z = aZ;
		SetAsModified();
	}

	void Transform::SetLocalRotation(const Quaternion& aQuaternion)
	{
		/*if (!myRotation.IsSameOrientationAs(aQuaternion))
		{
			SetAsModified();
		}*/

		if (myRotation.IsSameOrientationAs(aQuaternion))
		{
			return;
		}

		myRotation = aQuaternion;
		SetAsModified();
	}

	void Transform::SetLocalRotation(const Vector3f& aRotation)
	{
		SetLocalRotation(aRotation.x, aRotation.y, aRotation.z);

		/*if ((myRotation.GetEulerAngles() - aRotation).LengthSqr() > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}

		myRotation = Quaternion::CreateFromEulerAngles(aRotation);*/
	}

	void Transform::SetLocalRotation(const float aX, const float aY, const float aZ)
	{
		/*if (Abs(myRotation.x - aX) > TRANSFORM_EPSILON ||
			Abs(myRotation.y - aY) > TRANSFORM_EPSILON ||
			Abs(myRotation.z - aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/



		const auto newRot = Quaternion::CreateFromEulerAngles(aX, aY, aZ);
		if (myRotation.IsSameOrientationAs(newRot))
		{
			return;
		}
		myRotation = newRot;
		SetAsModified();
	}

	void Transform::SetLocalScale(const Vector3f& aScale)
	{
		SetLocalScale(aScale.x, aScale.y, aScale.z);
	}

	void Transform::SetLocalScale(const float aX, const float aY, const float aZ)
	{
		/*if (Abs(myScale.x - aX) > TRANSFORM_EPSILON ||
			Abs(myScale.y - aY) > TRANSFORM_EPSILON ||
			Abs(myScale.z - aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(myScale.x - aX) < TRANSFORM_EPSILON &&
			Abs(myScale.y - aY) < TRANSFORM_EPSILON &&
			Abs(myScale.z - aZ) < TRANSFORM_EPSILON)
		{
			return;
		}

		myScale.x = aX;
		myScale.y = aY;
		myScale.z = aZ;
		SetAsModified();
	}

	void Transform::SetLocalXScale(const float aX)
	{
		/*if (Abs(myScale.x - aX) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(myScale.x - aX) < TRANSFORM_EPSILON)
		{
			return;
		}

		myScale.x = aX;
		SetAsModified();
	}

	void Transform::SetLocalYScale(const float aY)
	{
		/*if (Abs(myScale.y - aY) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(myScale.y - aY) < TRANSFORM_EPSILON)
		{
			return;
		}
		myScale.y = aY;
		SetAsModified();
	}

	void Transform::SetLocalZScale(const float aZ)
	{
		/*if (Abs(myScale.z - aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(myScale.z - aZ) < TRANSFORM_EPSILON)
		{
			return;
		}
		myScale.z = aZ;
		SetAsModified();
	}

	void Transform::AddPosition(const Vector3f& aPositionDelta)
	{
		AddPosition(aPositionDelta.x, aPositionDelta.y, aPositionDelta.z);
	}

	void Transform::AddPosition(const float aX, const float aY, const float aZ)
	{
		SetPosition(GetPosition() + Vector3f(aX, aY, aZ));
	}

	void Transform::AddLocalPosition(const Vector3f& aPositionDelta)
	{
		AddLocalPosition(aPositionDelta.x, aPositionDelta.y, aPositionDelta.z);

		///*if (aPositionDelta.LengthSqr() > TRANSFORM_EPSILON)
		//{
		//	SetAsModified();
		//}*/

		//myPosition += aPositionDelta;
		//SetAsModified();
	}

	void Transform::AddLocalPosition(const float aX, const float aY, const float aZ)
	{
		/*if (Abs(aX) > TRANSFORM_EPSILON ||
			Abs(aY) > TRANSFORM_EPSILON ||
			Abs(aZ) > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/

		if (Abs(aX) < TRANSFORM_EPSILON &&
			Abs(aY) < TRANSFORM_EPSILON &&
			Abs(aZ) < TRANSFORM_EPSILON)
		{
			return;
		}

		myPosition.x += aX;
		myPosition.y += aY;
		myPosition.z += aZ;
		SetAsModified();
	}

	void Transform::AddRotation(const Quaternion& aQuaternion)
	{
		/*if (!Quaternion::IsSameOrientation(Quaternion(), aQuaternion))
		{
			SetAsModified();
		}*/


		if (Quaternion::IsSameOrientation(Quaternion(), aQuaternion))
		{
			return;
		}

		myRotation = aQuaternion * myRotation;
		SetAsModified();
	}

	void Transform::AddRotation(const Vector3f& aRotationDelta)
	{
		AddRotation(Quaternion::CreateFromEulerAngles(aRotationDelta));
	}

	void Transform::AddLocalRotation(const Quaternion& aQuaternion)
	{
		if (Quaternion::IsSameOrientation(Quaternion(), aQuaternion))
		{
			return;
		}

		myRotation = myRotation * aQuaternion;
		SetAsModified();
	}

	void Transform::AddLocalRotation(const Vector3f& aRotationDelta)
	{
		AddLocalRotation(Quaternion::CreateFromEulerAngles(aRotationDelta));
	}

	void Transform::ScaleBy(const Vector3f& aScale)
	{
		/*if ((aScale - Vector3f::One()).LengthSqr() > TRANSFORM_EPSILON)
		{
			SetAsModified();
		}*/


		if ((aScale - Vector3f::One()).LengthSqr() < TRANSFORM_EPSILON)
		{
			return;
		}

		myScale *= aScale;
		SetAsModified();
	}

	void Transform::LookAt(const Utils::Vector3f& aTargetPosition)
	{
		SetRotation(Quaternion::CreateLookRotation(aTargetPosition - GetPosition()));
	}

	Utils::BasicTransform Transform::ToBasic()
	{
		BasicTransform basic;
		basic.SetPosition(GetPosition());
		basic.SetRotation(GetQuaternion());
		basic.SetScale(GetScale());
		return basic;
	}

	Transform Transform::LocalLerp(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp)
	{
		if (aClamp)
		{
			aLerpFactor = Clamp(aLerpFactor, 0.0f, 1.0f);
		}

		const Vector3f lerpedPos = Vector3f::Lerp(aTransform.GetLocalPosition(), anotherTransform.GetLocalPosition(), aLerpFactor);
		const Quaternion lerpedRot = Quaternion::SLerp(aTransform.GetLocalQuaternion(), anotherTransform.GetLocalQuaternion(), aLerpFactor);
		const Vector3f lerpedScale = Vector3f::Lerp(aTransform.GetLocalScale(), anotherTransform.GetLocalScale(), aLerpFactor);

		return { lerpedPos, lerpedRot, lerpedScale };
	}

	Transform Transform::Lerp(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp)
	{
		if (aClamp)
		{
			aLerpFactor = Clamp(aLerpFactor, 0.0f, 1.0f);
		}

		const Vector3f lerpedPos = Vector3f::Lerp(aTransform.GetPosition(), anotherTransform.GetPosition(), aLerpFactor);
		const Quaternion lerpedRot = Quaternion::SLerp(aTransform.GetQuaternion(), anotherTransform.GetQuaternion(), aLerpFactor);
		const Vector3f lerpedScale = Vector3f::Lerp(aTransform.GetScale(), anotherTransform.GetScale(), aLerpFactor);

		return { lerpedPos, lerpedRot, lerpedScale };
	}

	Transform Transform::EaseInOut(const Transform& aTransform, const Transform& anotherTransform, float aLerpFactor, bool aClamp)
	{
		if (aClamp)
		{
			aLerpFactor = Clamp(aLerpFactor, 0.0f, 1.0f);
		}

		const Vector3f lerpedPos = Utils::EaseInOut(aTransform.GetPosition(), anotherTransform.GetPosition(), aLerpFactor);
		const Quaternion lerpedRot = Quaternion::SLerp(aTransform.GetQuaternion(), anotherTransform.GetQuaternion(), aLerpFactor);
		const Vector3f lerpedScale = Utils::EaseInOut(aTransform.GetScale(), anotherTransform.GetScale(), aLerpFactor);

		return { lerpedPos, lerpedRot, lerpedScale };
	}

	Transform Transform::Combine(const Transform& aTransform, const Transform& anotherTransform)
	{
		Transform out;
		out.SetScale(aTransform.GetScale() * anotherTransform.GetScale());

		out.SetRotation(aTransform.GetQuaternion() * anotherTransform.GetQuaternion());

		out.SetPosition(aTransform.GetQuaternion() * (aTransform.GetScale() * anotherTransform.GetPosition()));
		out.SetPosition(aTransform.GetPosition() + out.GetPosition());

		return out;
	}

	void Transform::RecalculateTransform()
	{
		myLocalMatrix = Matrix4f::CreateFromPosRotScale(myPosition, myRotation, myScale);
		myWasModified = false;

		if (myParent)
		{
			myWorldMatrix = myLocalMatrix * myParent->GetMatrix();
		}
		else
		{
			myWorldMatrix = myLocalMatrix;
		}

		for (const auto& child : myChildren)
		{
			child->RecalculateTransform();
		}
	}

	const Matrix4f& Transform::GetMatrix()
	{
		if (myWasModified)
		{
			RecalculateTransform();
		}

		return myWorldMatrix;
	}

	const Matrix4f& Transform::GetLocalMatrix()
	{
		if (myWasModified)
		{
			RecalculateTransform();
		}

		return myLocalMatrix;
	}

	Vector3f Transform::GetPosition() const
	{
		if (myParent)
		{
			Vector4f posVec4 = Vec3ToVec4(myPosition);
			posVec4 = posVec4 * myParent->GetMatrix();
			return Vec4ToVec3(posVec4);
		}

		return myPosition;
	}

	float Transform::GetXPosition() const
	{
		return GetPosition().x;
	}

	float Transform::GetYPosition() const
	{
		return GetPosition().y;
	}

	float Transform::GetZPosition() const
	{
		return GetPosition().z;
	}

	const Vector3f& Transform::GetLocalPosition() const
	{
		return myPosition;
	}

	float Transform::GetLocalXPosition() const
	{
		return myPosition.x;
	}

	float Transform::GetLocalYPosition() const
	{
		return myPosition.y;
	}

	float Transform::GetLocalZPosition() const
	{
		return myPosition.z;
	}

	Quaternion Transform::GetQuaternion() const
	{
		if (myParent)
		{
			return myParent->GetQuaternion() * myRotation;
		}

		return myRotation;
	}

	const Quaternion& Transform::GetLocalQuaternion() const
	{
		return myRotation;
	}

	Vector3f Transform::GetRotation() const
	{
		if (myParent)
		{
			return (myRotation * myParent->GetQuaternion()).GetEulerAngles();
		}

		return myRotation.GetEulerAngles();
	}

	Vector3f Transform::GetLocalRotation() const
	{
		return myRotation.GetEulerAngles();
	}

	Vector3f Transform::GetScale() const
	{
		if (myParent)
		{
			return myScale * myParent->GetScale();
		}

		return myScale;
	}

	const Vector3f& Transform::GetLocalScale() const
	{
		return myScale;
	}

	Vector3f Transform::GetRight()
	{
		Matrix4f transform = GetMatrix();
		return Vector3f(transform(1, 1), transform(1, 2), transform(1, 3)).GetNormalized();
	}

	Vector3f Transform::GetLeft()
	{
		return GetRight() * -1.0f;
	}

	Vector3f Transform::GetUp()
	{
		Matrix4f transform = GetMatrix();
		return Vector3f(transform(2, 1), transform(2, 2), transform(2, 3)).GetNormalized();
	}

	Vector3f Transform::GetDown()
	{
		return GetUp() * -1.0f;
	}

	Vector3f Transform::GetForward()
	{
		Matrix4f transform = GetMatrix();
		return Vector3f(transform(3, 1), transform(3, 2), transform(3, 3)).GetNormalized();
	}

	Vector3f Transform::GetBackward()
	{
		return GetForward() * -1.0f;
	}

	Vector3f Transform::GetLocalRight()
	{
		Matrix4f transform = GetLocalMatrix();
		return Vector3f(transform(1, 1), transform(1, 2), transform(1, 3)).GetNormalized();
	}

	Vector3f Transform::GetLocalLeft()
	{
		return GetLocalRight() * -1.f;
	}

	Vector3f Transform::GetLocalUp()
	{
		Matrix4f transform = GetLocalMatrix();
		return Vector3f(transform(2, 1), transform(2, 2), transform(2, 3)).GetNormalized();
	}

	Vector3f Transform::GetLocalDown()
	{
		return GetLocalUp() * -1.f;
	}

	Vector3f Transform::GetLocalForward()
	{
		Matrix4f transform = GetLocalMatrix();
		return Vector3f(transform(3, 1), transform(3, 2), transform(3, 3)).GetNormalized();
	}

	Vector3f Transform::GetLocalBackward()
	{
		return GetLocalForward() * -1.f;
	}

	void Transform::SetAsModified()
	{
		myWasModified = true;

		for (int i = 0; i < std::ssize(myChildren); i++)
		{
			myChildren[i]->SetAsModified();
		}
	}

	void Transform::SetParent(Transform* aParent, bool aKeepWorldTransformations)
	{
		if (aKeepWorldTransformations)
		{
			//Gather the old global transformation data
			const Vector3f oldPosition = GetPosition();
			const Quaternion oldRotation = GetQuaternion();
			const Vector3f oldScale = GetScale();

			myParent = aParent;

			//With the new parent, set the world transformations relative to new parent
			//This results in the transformations being the same after parenting
			SetScale(oldScale);
			SetRotation(oldRotation);
			SetPosition(oldPosition);
			SetAsModified();
		}
		else
		{
			myParent = aParent;
		}
	}

	Transform* Transform::GetParent()
	{
		return myParent;
	}

	void Transform::AddChild(Transform* aChild)
	{
		myChildren.push_back(aChild);
	}

	void Transform::RemoveChild(const Transform* aChild)
	{
		for (int i = 0; i < std::ssize(myChildren); i++)
		{
			if (myChildren[i] == aChild)
			{
				myChildren.erase(myChildren.begin() + i);
				return;
			}
		}
	}
}
