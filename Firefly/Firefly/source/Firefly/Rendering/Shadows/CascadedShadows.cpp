#include "FFpch.h"
#include "CascadedShadows.h"
#include <Utils/Math/Vector.h>

namespace Firefly
{
	CascadeBuilder::CascadeBuilder(DirLightData& aDirLight) : myDirlight(aDirLight)
	{
		myAmountOfCascades = 0;
	}
	CascadeBuilder& CascadeBuilder::SetCascadeCount(uint32_t aCascadeCount)
	{
		myAmountOfCascades = aCascadeCount;
		myShadowLevels.resize(aCascadeCount);
		return *this;
	}
	CascadeBuilder& CascadeBuilder::SetActiveCamera(Ptr<Camera> aCamera)
	{
		myCam = aCamera;
		return *this;
	}
	std::vector<Cascade> CascadeBuilder::Build()
	{
		auto cam = myCam.lock();
		auto clampedFarplane = Utils::Min(cam->GetFarPlane(), 15000.f);
		myShadowLevels[0] = clampedFarplane / 50.f;
		myShadowLevels[1] = clampedFarplane / 25.f;
		myShadowLevels[2] = clampedFarplane / 10.f;
		myShadowLevels[3] = clampedFarplane / 2.f;
		std::vector<Cascade> cascades(myAmountOfCascades + 2);
		for (size_t i = 0; i < myShadowLevels.size() + 1; ++i)
		{
			if(i == 0)
			{
				cascades[i] = CreateCascade(cam->GetNearPlane(), myShadowLevels[i]);
			}
			else if (i < myShadowLevels.size())
			{
				cascades[i] = CreateCascade(myShadowLevels[i - 1], myShadowLevels[i]);
			}
			else
			{
				cascades[i] = CreateCascade(myShadowLevels[i - 1], clampedFarplane);
			}
		}
		cascades[5] = CreateCascade(cam->GetNearPlane(), Utils::Lerp(myShadowLevels[1], myShadowLevels[2], 0.5f));
		return cascades;
	}
	Cascade CascadeBuilder::CreateCascade(float aNearPlane, float aFarPlane)
	{
		auto& dirData = myDirlight.DirectionLightPacket[0];
		auto camera = myCam.lock();
		CameraInfo info{};
		info.NearPlane = aNearPlane;
		info.FarPlane = aFarPlane;
		info.Fov = camera->GetFov();
		info.ResolutionX = camera->GetSizeX();
		info.ResolutionY = camera->GetSizeY();
		Camera cam(info);
		cam.GetTransform().SetPosition(camera->GetTransform().GetPosition());
		cam.GetTransform().SetRotation(camera->GetTransform().GetQuaternion());
		cam.GetTransform().SetScale(camera->GetTransform().GetScale());
		cam.UpdateFrustum();
		Utils::Vec3 center;
		for (const auto& v : cam.GetFrustrum().Corners)
		{
			center += v;
		}
		center /= (float)cam.GetFrustrum().Corners.size();

		static auto up = Utils::Vec3::Up();
		const auto lightView = Utils::Mat4::CreateLookAtLH(center + Utils::Vec4ToVec3(dirData.Direction), center , up);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		for (const auto& v : cam.GetFrustrum().Corners)
		{
			const auto trf = Utils::Vec3ToVec4(v) * lightView;
			minX = Utils::Min(minX, trf.x);
			maxX = Utils::Max(maxX, trf.x);
			minY = Utils::Min(minY, trf.y);
			maxY = Utils::Max(maxY, trf.y);
			minZ = Utils::Min(minZ, trf.z);
			maxZ = Utils::Max(maxZ, trf.z);
		}

		constexpr float zMult = 10.f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}

		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}



		Cascade cascade;
		cascade.Projection = Utils::Mat4::CreateProjectionMatrixOrthographic(minX, maxX, minY, maxY, minZ * 10.f, maxZ);
		cascade.View = lightView;
		cascade.Levels = aFarPlane;
		return cascade;
	}
}
