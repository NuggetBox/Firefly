// Huge thanks to Unreal engine team at Epic games https://sebh.github.io/publications/egsr2020.pdf
// Copyright Epic Games, Inc. All Rights Reserved.

#include <Includes/PostProcessStructs.hlsli>
#include "SkyAtmosphereCommon.hlsl"
#include <Includes/VolumetricFogCommon.hlsli>
#include <Includes/ConstStructs.hlsli>

static int g_TransmittanceWidth = 256;
static int g_TransmittanceHeight = 64;

Texture2D<float4> depthTexture : register(t4);
Texture3D<float4> u_VolumetricFogVolume : register(t48);
struct RayMarchPixelOutputStruct
{
    float4 Luminance : SV_TARGET0;
#if COLORED_TRANSMITTANCE_ENABLED
	float4 Transmittance	: SV_TARGET1;
#endif
};
struct VertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

float3 InscatteredLight(float3 color, float3 worldPos)
{
    worldPos.y *= -1.f;

    float3 uv = WorldToUv(worldPos, nearPlane, 500000, 0, mul(toProjection, toView));
    uv.z = 1.f;
    
    float4 scatteredLight = tex3DTricubic(u_VolumetricFogVolume, WrapSampler, float3(worldPos.x / (resolution.x - 1.f), worldPos.y / (resolution.y - 1.f), 1.f), (float3(FROXEL_GRID_SIZE_X, FROXEL_GRID_SIZE_Y, FROXEL_GRID_SIZE_Z)));
    if (scatteredLight.w == 0)
    {
        return color;
    }
    
    float transmittance = scatteredLight.a;
    float3 col = color * transmittance + scatteredLight.rgb;
    return col;
}

RayMarchPixelOutputStruct main(VertextoPixel Input)
{
    RayMarchPixelOutputStruct output = (RayMarchPixelOutputStruct) 0;
#if COLORED_TRANSMITTANCE_ENABLED
	output.Transmittance = float4(0, 0, 0, 1);
#endif

    float2 pixPos = Input.position.xy;
    AtmosphereParameters Atmosphere = GetAtmosphereParameters();

    float3 ClipSpace = float3((pixPos / float2(gResolution)) * float2(2.0, -2.0) - float2(1.0, -1.0), 1.0);
    float4 HViewPos = mul(gSkyInvProjMat, float4(ClipSpace, 1.0));
    float3 WorldDir = normalize(mul((float3x3) gSkyInvViewMat, HViewPos.xyz / HViewPos.w));
    float3 cameraPos = camera;
    float3 WorldPos = cameraPos + float3(0, 0, Atmosphere.BottomRadius);
    
    float DepthBufferValue = -1.0;


	//if (pixPos.x < 512 && pixPos.y < 512)
	//{
	//	output.Luminance = float4(MultiScatTexture.SampleLevel(samplerLinearClamp, pixPos / float2(512, 512), 0).rgb, 1.0);
	//	return output;
	//}


    float viewHeight = length(WorldPos);
    float3 L = 0;
    DepthBufferValue = depthTexture[pixPos].r;
#if FASTSKY_ENABLED
	if (viewHeight < Atmosphere.TopRadius && DepthBufferValue == 1.0f)
	{
		float2 uv;
		float3 UpVector = normalize(WorldPos);
		float viewZenithCosAngle = dot(WorldDir, UpVector);

		float3 sideVector = normalize(cross(UpVector, WorldDir));		// assumes non parallel vectors
		float3 forwardVector = normalize(cross(sideVector, UpVector));	// aligns toward the sun light but perpendicular to up vector
		float2 lightOnPlane = float2(dot(sun_direction, forwardVector), dot(sun_direction, sideVector));
		lightOnPlane = normalize(lightOnPlane);
		float lightViewCosAngle = lightOnPlane.x;

		bool IntersectGround = raySphereIntersectNearest(WorldPos, WorldDir, float3(0, 0, 0), Atmosphere.BottomRadius) >= 0.0f;

		SkyViewLutParamsToUv(Atmosphere, IntersectGround, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);


		//output.Luminance = float4(SkyViewLutTexture.SampleLevel(samplerLinearClamp, pixPos / float2(gResolution), 0).rgb + GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius), 1.0);
		output.Luminance = float4(SkyViewLutTexture.SampleLevel(samplerLinearClamp, uv, 0).rgb + GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius), 1.0);
		return output;
	}
#else
    if (DepthBufferValue == 1.0f)
        L += GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius);
#endif

#if FASTAERIALPERSPECTIVE_ENABLED

#if COLORED_TRANSMITTANCE_ENABLED
#error The FASTAERIALPERSPECTIVE_ENABLED path does not support COLORED_TRANSMITTANCE_ENABLED.
#else

	ClipSpace = float3((pixPos / float2(gResolution))*float2(2.0, -2.0) - float2(1.0, -1.0), DepthBufferValue);
	float4 DepthBufferWorldPos = mul(gSkyInvViewProjMat, float4(ClipSpace, 1.0));
	DepthBufferWorldPos /= DepthBufferWorldPos.w;
	float tDepth = length(DepthBufferWorldPos.xyz - (WorldPos + float3(0.0, 0.0, -Atmosphere.BottomRadius)));
	float Slice = AerialPerspectiveDepthToSlice(tDepth);
	float Weight = 1.0;
	if (Slice < 0.5)
	{
		// We multiply by weight to fade to 0 at depth 0. That works for luminance and opacity.
		Weight = saturate(Slice * 2.0);
		Slice = 0.5;
	}
	float w = sqrt(Slice / AP_SLICE_COUNT);	// squared distribution

	const float4 AP = Weight * AtmosphereCameraScatteringVolume.SampleLevel(samplerLinearClamp, float3(pixPos / float2(gResolution), w), 0);
	L.rgb += AP.rgb;
	float Opacity = AP.a;

	output.Luminance = float4(L, Opacity);
	//output.Luminance *= frac(clamp(w*AP_SLICE_COUNT, 0, AP_SLICE_COUNT));
#endif

#else // FASTAERIALPERSPECTIVE_ENABLED

	// Move to top atmosphere as the starting point for ray marching.
	// This is critical to be after the above to not disrupt above atmosphere tests and voxel selection.
    if (!MoveToTopAtmosphere(WorldPos, WorldDir, Atmosphere.TopRadius))
    {
		// Ray is not intersecting the atmosphere		
        output.Luminance = float4(GetSunLuminance(WorldPos, WorldDir, Atmosphere.BottomRadius), 1.0);
        return output;
    }

    const bool ground = false;
    const float SampleCountIni = 0.0f;
    const bool VariableSampleCount = true;
    const bool MieRayPhase = true;
    SingleScatteringResult ss = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, sun_direction, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

    L += ss.L;
    float3 throughput = ss.Transmittance;

#if COLORED_TRANSMITTANCE_ENABLED
	output.Luminance = float4(L, 1.0f);
	output.Transmittance = float4(throughput, 1.0f);
#else
    const float Transmittance = dot(throughput, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));
    if (renPadd.x > 0)
    {
        float3 volumetricFogColor = InscatteredLight(L, float3(Input.position.xy, 0.f));
        L = lerp(L, volumetricFogColor, FogSettings.w);
    }

    output.Luminance = float4(L, 1.f - Transmittance);
#endif

#endif // FASTAERIALPERSPECTIVE_ENABLED

    return output;
}