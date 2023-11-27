// Huge thanks to Unreal engine team at Epic games https://sebh.github.io/publications/egsr2020.pdf
// Copyright Epic Games, Inc. All Rights Reserved.

#include <Includes/PostProcessStructs.hlsli>
#include "SkyAtmosphereCommon.hlsl"

struct VertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};
float4 main(VertextoPixel input) : SV_Target
{
    float2 pixPos = input.position.xy;
    AtmosphereParameters Atmosphere = GetAtmosphereParameters();

    float3 ClipSpace = float3((pixPos / float2(192.0, 108.0)) * float2(2.0, -2.0) - float2(1.0, -1.0), 1.0);
    float4 HViewPos = mul(gSkyInvProjMat, float4(ClipSpace, 1.0));
    float3 WorldDir = normalize(mul((float3x3) gSkyInvViewMat, HViewPos.xyz / HViewPos.w));
    float3 WorldPos = camera + float3(0, 0, Atmosphere.BottomRadius);

    float2 uv = pixPos / float2(192.0, 108.0);

    float viewHeight = length(WorldPos);

    float viewZenithCosAngle;
    float lightViewCosAngle;
    UvToSkyViewLutParams(Atmosphere, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);


    float3 SunDir;
	{
        float3 UpVector = WorldPos / viewHeight;
        float sunZenithCosAngle = dot(UpVector, sun_direction);
        SunDir = normalize(float3(sqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle), 0.0, sunZenithCosAngle));
    }



    WorldPos = float3(0.0f, 0.0f, viewHeight);

    float viewZenithSinAngle = sqrt(1 - viewZenithCosAngle * viewZenithCosAngle);
    WorldDir = float3(
		viewZenithSinAngle * lightViewCosAngle,
		viewZenithSinAngle * sqrt(1.0 - lightViewCosAngle * lightViewCosAngle),
		viewZenithCosAngle);


	// Move to top atmospehre
    if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
    {
		// Ray is not intersecting the atmosphere
        return float4(0, 0, 0, 1);
    }

    const bool ground = false;
    const float SampleCountIni = 30;
    const float DepthBufferValue = -1.0;
    const bool VariableSampleCount = true;
    const bool MieRayPhase = true;
    SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, SunDir, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

    float3 L = ss.L;

    return float4(L, 1);
}