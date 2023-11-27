// Huge thanks to Unreal engine team at Epic games https://sebh.github.io/publications/egsr2020.pdf
// Copyright Epic Games, Inc. All Rights Reserved.

#include <Includes/PostProcessStructs.hlsli>
#include "SkyAtmosphereCommon.hlsl"

static int g_TransmittanceWidth = 256;
static int g_TransmittanceHeight = 64;
struct VertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};
float4 main(VertextoPixel input) : SV_Target
{
    float2 pixelPos = input.position.xy;
    
    AtmosphereParameters atomsphere = GetAtmosphereParameters();
    
    float2 uv = (pixelPos) / float2(g_TransmittanceWidth, g_TransmittanceHeight);
    
    float viewHeight;
    float viewZenithCosAngle;
    UvToLutTransmittanceParams(atomsphere, viewHeight, viewZenithCosAngle, uv);
    
    float3 worldPosition = float3(0.0f, 0.0f, viewHeight);
    float3 worldDirection = float3(0.0f, sqrt(1.0 - viewZenithCosAngle * viewZenithCosAngle), viewZenithCosAngle);
    const bool ground = true;
    const float sampleCountIni = 40.f; // Can go a low as 10 sample but energy lost starts to be visible.
    const float depthBufferValue = -1.0f;
    const bool varibleSampleCount = false;
    const bool mieRayPhase = false;
    
    float3 transmittance = exp(-IntegrateScatteredLuminance(
        pixelPos,
        worldPosition,
        worldDirection,
        sun_direction,
        atomsphere,
        ground,
        sampleCountIni,
        depthBufferValue,
        varibleSampleCount,
        mieRayPhase
    ).OpticalDepth);
    
    return float4(transmittance, 1);
}