#include "FFpch.h"
#include "Camera.h"
#include "Utils/Math/MathDefines.h"
#include <Utils/Math/Sphere.hpp>
#include <Utils/Math/Intersection.hpp>
#include "Firefly/Rendering/Renderer.h"

namespace Firefly
{
	Camera::Camera(const CameraInfo& aInfo)
	{
		myInfo = aInfo;

		if (aInfo.NearPlane < 1.f)
		{
			LOGINFO("DUMB DUMB NEAR PLANE TO SMALL");
			myInfo.NearPlane = 10.f;
		}
		myWasModified = true;
	}

	Camera::Camera(const Camera& aCam)
	{
		myFrustum = aCam.myFrustum;
		myInfo = aCam.myInfo;
		myTransform = aCam.myTransform;
		myWasModified = aCam.myWasModified;
	}

	float Camera::GetFov() const
	{
		return myInfo.Fov;
	}

	float Camera::GetFovRad() const
	{
		return DEGTORAD(myInfo.Fov);
	}

	void Camera::SetFov(float someDegrees)
	{
		myInfo.Fov = someDegrees;
		myWasModified = true;
	}

	void Camera::SetSize(float aSizeX, float aSizeY)
	{
		SetSizeX(aSizeX);
		SetSizeY(aSizeY);
	}

	void Camera::LookAt(const Utils::Vector3f& aPoint)
	{
		myTransform.LookAt(aPoint);
	}

	//bool Camera::TransformIsVisible(Utils::Transform& aTransform)
	//{
	//	return (myTransform.GetPosition() - aTransform.GetPosition()).Length() <= 8000;
	//	Utils::Sphere<float> sphere;
	//	float biggestSize = -1;
	//	const float sizeArray[3](aTransform.GetScale().x, aTransform.GetScale().y, aTransform.GetScale().z);
	//	for (size_t i = 0; i < 3; ++i)
	//	{
	//		if (sizeArray[i] > biggestSize)
	//		{
	//			biggestSize = sizeArray[i];
	//		}
	//	}
	//	sphere.InitWithCenterAndRadius(aTransform.GetPosition(), biggestSize * 100);
	//	//Frustum fustrum = CreateFustrum();
	//}

	void Camera::SetSizeX(float aSizeX)
	{
		myInfo.ResolutionX = aSizeX;
		myWasModified = true;
	}

	void Camera::SetSizeY(float aSizeY)
	{
		myInfo.ResolutionY = aSizeY;
		myWasModified = true;
	}

	void Camera::SetNearPlane(float aNearPlane)
	{
		myInfo.NearPlane = aNearPlane;
		myWasModified = true;
	}

	void Camera::SetFarPlane(float aFarPlane)
	{
		myInfo.FarPlane = aFarPlane;
		myWasModified = true;
	}

	Ref<Camera> Camera::Create(const CameraInfo& aInfo)
	{
		return CreateRef<Camera>(aInfo);
	}

	Utils::Vector3f Camera::ScreenPosToWorldDirection(const Utils::Vector2f& aMousePos, const Utils::Vector2f& aSize)
	{
		float x = (aMousePos.x / aSize.x) * 2.f - 1.f;
		float y = (aMousePos.y / aSize.y) * 2.f - 1.f;

		Utils::Matrix4f matInv = Utils::Matrix4f::GetInverse(GetViewMatrix() * GetProjectionMatrixPerspective());

		Utils::Vector4f rayOrgin = Utils::Vector4f(x, -y, 0, 1) * matInv;
		Utils::Vector4f rayEnd = Utils::Vector4f(x, -y, 1, 1) * matInv;

		if (rayOrgin.w == 0 || rayEnd.w == 0)
			return { 0,0,0 };

		rayOrgin /= rayOrgin.w;
		rayEnd /= rayEnd.w;

		Utils::Vector4f rayDir = (rayEnd - rayOrgin).GetNormalized();
		Utils::Vector3f secondRayDir = { rayDir.x, rayDir.y, rayDir.z };

		return secondRayDir;
	}

	Frustrum Camera::CreateViewFrustum(Utils::Transform& aTransform, float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane, float someFovDegrees)
	{
		const Utils::Vector3f nearCenter = aTransform.GetPosition() + aTransform.GetForward() * aNearPlane;
		const Utils::Vector3f farCenter = aTransform.GetPosition() + aTransform.GetForward() * aFarPlane;

		const float verticalFovCalc = tan(DEGTORAD(someFovDegrees) / 2.0f);

		const float nearHalfHeight = verticalFovCalc * aNearPlane;
		const float nearHalfWidth = nearHalfHeight * aResolutionX / aResolutionY;

		const float farHalfHeight = verticalFovCalc * aFarPlane;
		const float farHalfWidth = farHalfHeight * aResolutionX / aResolutionY;

		Frustrum frustum{};

		frustum.NearTopLeft = nearCenter + aTransform.GetLeft() * nearHalfWidth + aTransform.GetUp() * nearHalfHeight;
		frustum.NearTopRight = nearCenter + aTransform.GetRight() * nearHalfWidth + aTransform.GetUp() * nearHalfHeight;
		frustum.NearBottomRight = nearCenter + aTransform.GetRight() * nearHalfWidth + aTransform.GetDown() * nearHalfHeight;
		frustum.NearBottomLeft = nearCenter + aTransform.GetLeft() * nearHalfWidth + aTransform.GetDown() * nearHalfHeight;

		frustum.FarTopLeft = farCenter + aTransform.GetLeft() * farHalfWidth + aTransform.GetUp() * farHalfHeight;
		frustum.FarTopRight = farCenter + aTransform.GetRight() * farHalfWidth + aTransform.GetUp() * farHalfHeight;
		frustum.FarBottomRight = farCenter + aTransform.GetRight() * farHalfWidth + aTransform.GetDown() * farHalfHeight;
		frustum.FarBottomLeft = farCenter + aTransform.GetLeft() * farHalfWidth + aTransform.GetDown() * farHalfHeight;

		const Utils::Plane upPlane(frustum.NearTopLeft, frustum.FarTopLeft, frustum.NearTopRight);
		const Utils::Plane downPlane(frustum.NearBottomRight, frustum.FarBottomRight, frustum.NearBottomLeft);
		const Utils::Plane leftPlane(frustum.NearBottomLeft, frustum.FarBottomLeft, frustum.NearTopLeft);
		const Utils::Plane rightPlane(frustum.NearTopRight, frustum.FarTopRight, frustum.NearBottomRight);
		const Utils::Plane nearPlane(frustum.NearBottomLeft, frustum.NearTopLeft, frustum.NearBottomRight);
		const Utils::Plane farPlane(frustum.FarTopLeft, frustum.FarBottomLeft, frustum.FarTopRight);

		frustum.Frustum.AddPlane(nearPlane);
		frustum.Frustum.AddPlane(downPlane);
		frustum.Frustum.AddPlane(leftPlane);
		frustum.Frustum.AddPlane(rightPlane);
		frustum.Frustum.AddPlane(farPlane);
		frustum.Frustum.AddPlane(upPlane);

		return frustum;
	}

	void Camera::UpdateFrustum()
	{
		myFrustum = CreateViewFrustum(myTransform, myInfo.ResolutionX, myInfo.ResolutionY, myInfo.NearPlane, myInfo.FarPlane, myInfo.Fov);
	}

	bool Camera::MeshIsVisible(const Utils::Sphere<float>& aSphere)
	{
		if (!myFrustum.Frustum.GetPlanes().empty())
		{
			auto& frustumPlanes = myFrustum.Frustum.GetPlanes();

			for (auto& frustumPlane : frustumPlanes)
			{
				if (!frustumPlane.IsInside(aSphere))
				{
					return false;
				}
			}
		}

		return true;
	}

	Utils::Matrix4x4<float>& Camera::GetProjectionMatrixPerspective()
	{
		//if (myWasModified)
		//{
		//	myWasModified = false;
			myCachedProjectionMatrixPerspective = Utils::Matrix4f::CreateProjectionMatrixPerspective(myInfo.ResolutionX, myInfo.ResolutionY, myInfo.NearPlane, myInfo.FarPlane, myInfo.Fov);
		//}

		return myCachedProjectionMatrixPerspective;
	}

	Utils::Matrix4x4<float> Camera::GetProjectionMatrixOrthographic() const
	{
		return Utils::Matrix4f::CreateProjectionMatrixOrthographic(myInfo.ResolutionX, myInfo.ResolutionY, myInfo.NearPlane, myInfo.FarPlane);
	}

	Utils::Matrix4x4<float>& Camera::GetViewMatrix()
	{
		myCachedViewMatrix = Utils::Matrix4x4<float>::GetFastInverse(myTransform.GetMatrix());
		return myCachedViewMatrix;
	}
}