#pragma once
#include "Firefly/Rendering/RenderingDefines.h"
#include "Utils/Math/Matrix.h"

namespace Firefly
{
	struct CameraData
	{
		Utils::Matrix4x4<float> CameraSpace;
		Utils::Matrix4x4<float> ToProjectionSpace;
		Utils::Vector4f CameraPosition;
		float NearPlane;
		float FarPlane;
		Utils::Vector2f Resolution;
		Utils::Mat4 OldViewProj;
	};
	struct ModelData
	{
		uint32_t matrixBufferOffset;
		uint32_t boneStartOffset;
		uint32_t ObjectBufferPadding[2];
	};

	struct DirLightPacket
	{
		Utils::Vector4f ColorAndIntensity;
		Utils::Vector4f Direction;
		Utils::Matrix4f ViewMatrix;
		Utils::Matrix4f ProjMatrix;
		Utils::Vector4<uint32_t> dirLightInfo; // x = should cast, y = should cast hard shadows
	};

	struct DirLightData
	{
		DirLightPacket DirectionLightPacket[6];
		Utils::Vector4<int> Count;
	};

	struct PointLightPacket
	{
		Utils::Vector4f ColorAndIntensity = { 0,0,0,0 };
		Utils::Vector3f Position = { 0,0,0 };
		float Radius = 0;
		Utils::Matrix4f Transforms[6];
		Utils::Vec4 PointlightCustomData;
	};

	struct PointLightData
	{
		PointLightPacket PointLightPackets[FF_MAX_POINTLIGHTS];
		Utils::Vector4<int> Count;
	};

	struct SpotLightPacket
	{
		Vector4f ColorAndIntensity = { 0,0,0,0 };
		Vector4f Position = { 0,0,0, 0 };
		Vector4f Direction = { 0,0,0,0 };
		Vector4f Range_Inner_Outer_ShouldCastShadow = { 0, 0, 0, 0 };
		Utils::Matrix4f ViewProjMatrix;
	};

	struct SpotLightData
	{
		SpotLightPacket SpotLightPackets[FF_MAX_SPOTLIGHTS];
		Utils::Vector4<int> Count;
	};

	struct ParticleBufferData
	{
		Utils::Matrix4x4<float> ToWorld;
	};

	struct TimeData
	{
		Utils::Vector4f TimeInfo; // x = scaledTotalTime, y = unscaledTotalTime, z = deltatime, w = unscaledDeltaTime.
	};

	struct PostProcessData
	{
		Utils::Vector4f FogColor = { 1,1,1,1 };
		float Fogthreshold;
		float FogDensity;
		float WindSpeed;
		float FogWaveFrekvency;
		float FogWaveHeight;
		Utils::Vector3f Padding;
		Utils::Vector4f OutlineColor;
		float EnableLUT = 0.f;
		Utils::Vector3f Pad;
		Utils::Vector4f Saturation;
		Utils::Vector4f Constrast;
		Utils::Vector4f Gamma;
		Utils::Vector4f Gain;
		Utils::Vector4f Intensities;
		Utils::Vector4f Enables;
		Utils::Vector4f LogFog;
		Utils::Vector4f SSAOSettings;
	};
}