#pragma once
#include "Firefly/Core/Core.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Math/Plane.hpp"
#include "Utils/Math/PlaneVolume.hpp"
#include "Utils/Math/Sphere.hpp"
#include "Utils/Math/Transform.h"

namespace Firefly
{
	struct CameraInfo
	{
		float Fov = 90.0f;
		float ResolutionX = 1280.0f;
		float ResolutionY = 720.0f;
		float NearPlane = 10.0f;
		float FarPlane = 1000000.0f;
	};

	struct Frustrum
	{
		union
		{
			struct  
			{
				Utils::Vec3 NearTopLeft;
				Utils::Vec3 NearTopRight;
				Utils::Vec3 NearBottomRight;
				Utils::Vec3 NearBottomLeft;

				Utils::Vec3 FarTopLeft;
				Utils::Vec3 FarTopRight;
				Utils::Vec3 FarBottomRight;
				Utils::Vec3 FarBottomLeft;
			};
			std::array<Utils::Vec3, 8> Corners;
		};
		Utils::PlaneVolume<float> Frustum;

		bool MeshIsVisible(const Utils::Sphere<float>& aSphere)
		{
			if (!Frustum.GetPlanes().empty())
			{
				auto& frustumPlanes = Frustum.GetPlanes();

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
	};

	struct Plan
	{
		Utils::Vector3f Normal;
		float Dist = 0;
	};

	class Camera
	{
	public:
		Camera(const CameraInfo& aInfo);
		Camera(const Camera& aCam);
		Utils::Matrix4x4<float>& GetProjectionMatrixPerspective();
		Utils::Matrix4x4<float> GetProjectionMatrixOrthographic() const;
		Utils::Matrix4x4<float>& GetViewMatrix();
		Utils::Transform& GetTransform() { return myTransform; }

		//RIP
		//Utils::Vector4<float> ConvertToPostProjectionSpace(Utils::Vector3<float> aWorldSpacePoint);
		//

		//Gets full fov in degrees
		float GetFov() const;
		float GetFovRad() const;
		void SetFov(float someDegrees);

		void SetSize(float aSizeX, float aSizeY);

		void LookAt(const Utils::Vector3f& aPoint);
		//bool TransformIsVisible(Utils::Transform& aTransform);

		float GetSizeX() const { return myInfo.ResolutionX; }
		void SetSizeX(float aSizeX);

		float GetSizeY() const { return myInfo.ResolutionY; }
		void SetSizeY(float aSizeY);

		float GetNearPlane() const { return myInfo.NearPlane; }
		void SetNearPlane(float aNearPlane);

		float GetFarPlane() const { return myInfo.FarPlane; }
		void SetFarPlane(float aFarPlane);

		CameraInfo& GetInfo() { return myInfo; }

		static Ref<Camera> Create(const CameraInfo& aInfo);

		Utils::Vector3f ScreenPosToWorldDirection(const Utils::Vector2f& aMousePos, const Utils::Vector2f& aSize);

		inline const Frustrum& GetFrustrum() { return myFrustum; }

		static Frustrum CreateViewFrustum(Utils::Transform& aTransform, float aResolutionX, float aResolutionY, float aNearPlane, float aFarPlane, float someFovDegrees);

		void UpdateFrustum();
		bool MeshIsVisible(const Utils::Sphere<float>& aSphere);

	private:
		Utils::Transform myTransform;
		
		CameraInfo myInfo;

		Utils::Matrix4f myCachedProjectionMatrixPerspective;
		Utils::Matrix4f myCachedViewMatrix;
		bool myWasModified;

		Frustrum myFrustum{};
	};
}