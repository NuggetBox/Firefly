#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/PBRFunctions.hlsli>
#include <Includes/PostProcessStructs.hlsli>
#include <includes/ConstStructs.hlsli>
#include <includes/Math.hlsli>
#include <includes/ShadowFunctions.hlsli>

cbuffer SKYATMOSPHERE_BUFFER : register(b8)
{
	//
	// From AtmosphereParameters
	//

    float3 solar_irradiance;
    float sun_angular_radius;

    float3 absorption_extinction;
    float mu_s_min;

    float3 rayleigh_scattering;
    float mie_phase_function_g;

    float3 mie_scattering;
    float bottom_radius;

    float3 mie_extinction;
    float top_radius;

    float3 mie_absorption;
    float pad00;

    float3 ground_albedo;
    float pad0;

    float4 rayleigh_density[3];
    float4 mie_density[3];
    float4 absorption_density[3];

	//
	// Add generated static header constant
	//

    int TRANSMITTANCE_TEXTURE_WIDTH;
    int TRANSMITTANCE_TEXTURE_HEIGHT;
    int IRRADIANCE_TEXTURE_WIDTH;
    int IRRADIANCE_TEXTURE_HEIGHT;

    int SCATTERING_TEXTURE_R_SIZE;
    int SCATTERING_TEXTURE_MU_SIZE;
    int SCATTERING_TEXTURE_MU_S_SIZE;
    int SCATTERING_TEXTURE_NU_SIZE;

    float3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
    float pad3;
    float3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
    float pad4;

	//
	// Other globals
	//
    float4x4 gSkyViewProjMat;
    float4x4 gSkyInvViewProjMat;
    float4x4 gSkyInvProjMat;
    float4x4 gSkyInvViewMat;
    float4x4 gShadowmapViewProjMat;

    float3 camera;
    float pad5;
    float3 sun_direction;
    float pad6;
    float3 view_ray;
    float pad7;

    float MultipleScatteringFactor;
    float MultiScatteringLUTRes;
    float pad9;
    float pad10;
};

struct AtmosphereParameters
{
	// Radius of the planet (center to ground)
    float BottomRadius;
	// Maximum considered atmosphere height (center to atmosphere top)
    float TopRadius;

	// Rayleigh scattering exponential distribution scale in the atmosphere
    float RayleighDensityExpScale;
	// Rayleigh scattering coefficients
    float3 RayleighScattering;

	// Mie scattering exponential distribution scale in the atmosphere
    float MieDensityExpScale;
	// Mie scattering coefficients
    float3 MieScattering;
	// Mie extinction coefficients
    float3 MieExtinction;
	// Mie absorption coefficients
    float3 MieAbsorption;
	// Mie phase function excentricity
    float MiePhaseG;

	// Another medium type in the atmosphere
    float AbsorptionDensity0LayerWidth;
    float AbsorptionDensity0ConstantTerm;
    float AbsorptionDensity0LinearTerm;
    float AbsorptionDensity1ConstantTerm;
    float AbsorptionDensity1LinearTerm;
	// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
    float3 AbsorptionExtinction;

	// The albedo of the ground.
    float3 GroundAlbedo;
};

AtmosphereParameters GetAtmosphereParameters()
{
    AtmosphereParameters Parameters;
    Parameters.AbsorptionExtinction = absorption_extinction;

	// Traslation from Bruneton2017 parameterisation.
    Parameters.RayleighDensityExpScale = rayleigh_density[1].w;
    Parameters.MieDensityExpScale = mie_density[1].w;
    Parameters.AbsorptionDensity0LayerWidth = absorption_density[0].x;
    Parameters.AbsorptionDensity0ConstantTerm = absorption_density[1].x;
    Parameters.AbsorptionDensity0LinearTerm = absorption_density[0].w;
    Parameters.AbsorptionDensity1ConstantTerm = absorption_density[2].y;
    Parameters.AbsorptionDensity1LinearTerm = absorption_density[2].x;

    Parameters.MiePhaseG = mie_phase_function_g;
    Parameters.RayleighScattering = rayleigh_scattering;
    Parameters.MieScattering = mie_scattering;
    Parameters.MieAbsorption = mie_absorption;
    Parameters.MieExtinction = mie_extinction;
    Parameters.GroundAlbedo = ground_albedo;
    Parameters.BottomRadius = bottom_radius;
    Parameters.TopRadius = top_radius;
    return Parameters;
}

void LutTransmittanceParamsToUv(AtmosphereParameters Atmosphere, in float viewHeight, in float viewZenithCosAngle, out float2 uv)
{
    float H = sqrt(max(0.0f, Atmosphere.TopRadius * Atmosphere.TopRadius - Atmosphere.BottomRadius * Atmosphere.BottomRadius));
    float rho = sqrt(max(0.0f, viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius));

    float discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0) + Atmosphere.TopRadius * Atmosphere.TopRadius;
    float d = max(0.0, (-viewHeight * viewZenithCosAngle + sqrt(discriminant))); // Distance to atmosphere boundary

    float d_min = Atmosphere.TopRadius - viewHeight;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    uv = float2(x_mu, x_r);
	//uv = float2(fromUnitToSubUvs(uv.x, TRANSMITTANCE_TEXTURE_WIDTH), fromUnitToSubUvs(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT)); // No real impact so off
}

Texture2D u_DepthTexture : register(t0);
Texture2DArray u_ShadowTexture : register(t1);
Texture2D u_ColorTexture : register(t2);
Texture2D u_TransmittanceTexture : register(t3);

float3 ReconstructPositionFromDepth(in float2 uv, in float depth, in float4x4 inverseViewProj)
{
    // improved normal reconstructed from Wicked engine.
    // https://wickedengine.net/2019/09/22/improved-normal-reconstruction-from-depth/
    float x = uv.x * 2.f - 1.f;
    float y = (1.f - uv.y) * 2.f - 1.f;
    
    float4 positionS = float4(x, y, depth, 1.f);
    float4 positionV = mul(inverseViewProj, positionS);
    return positionV.xyz / positionV.w;
}

#define NUM_SAMPLES 128
#define NUM_SAMPLES_RCP 0.0078125
#define TAU 0.0001f
#define PHI 1000000.f
#define PI_RCP 0.31830988618379067153776752674503

void ExecuteRaymarch(inout float4 rayPosition, float3 negViewDirection, float stepSize, int shadowLayer, float interation, inout float3 vli)
{
    rayPosition.xyz += stepSize * negViewDirection.xyz;
    
    float3 shadowTerm = 0.f;
    float4 shadowCoord = mul(dirLight[shadowLayer].projMatrix, rayPosition);
    shadowTerm = (1.f - DirectionalHardShadows(u_ShadowTexture, ClampSampler, shadowCoord, float3(0, 1, 0), normalize(dirLight[0].direction.xyz), shadowLayer, dirLight[shadowLayer].direction.w)).xxx;
    float d = length(rayPosition.xyz);
    float dRcp = rcp(d);
    
    float3 intens = TAU * (shadowTerm * ((dirLight[0].colorAndIntensity.w * PHI) * 0.25 * PI_RCP) * dRcp * dRcp) * exp(-d * TAU) * exp(-interation * TAU) * stepSize;
    
    vli += intens;
}


float4 main(VertextoPixel input) : SV_Target
{
    // max distance, may be longer idk.
    float rayMarchDistanceLimit = 999.f;
    
    float depth = u_DepthTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0).r;
    
    float4x4 inverseVP = mul(inverse(toView), inverse(toProjection));
    
    float4 FragWorldPos = float4(ReconstructPositionFromDepth(input.uv, depth, inverseVP), 1.f);
    float4 viewPos = mul(toView, FragWorldPos);
    float depthValue = abs(viewPos.z);
    int layer = -1.f;
    for (int layerID = 0; layerID < 5; layerID++)
    {
        if (depthValue < dirLight[layerID].direction.w)
        {
            layer = layerID;
            break;
        }
    }
            
    if (layer == -1)
    {
        layer = 5;
    }
    float4 lightSpaceFragPos = mul(dirLight[layer].viewMatrix, FragWorldPos).xyzw;
    float3 lightSpaceCamPos = mul(dirLight[layer].viewMatrix, cameraPosition).xyz;
    
    
    float3 negDir = normalize(lightSpaceCamPos.xyz - lightSpaceFragPos.xyz);
    
    float raymarchDistance = (clamp(length(lightSpaceCamPos.xyz - lightSpaceFragPos.xyz), 0.0f, rayMarchDistanceLimit));
    
    // setup ray stepping.
    float stepSize = raymarchDistance * NUM_SAMPLES_RCP;
    float4 rayPosition = lightSpaceFragPos.xyzw;
    
    float3 VLI = 0.0f;
    [loop]
    for (float l = raymarchDistance; l > stepSize; l -= stepSize)
    {
       
        FragWorldPos = mul(inverse(dirLight[layer].viewMatrix), rayPosition).xyzw;
        viewPos = mul(toView, FragWorldPos);
        depthValue = abs(viewPos.z);
        layer = -1.f;
        for (int layerID = 0; layerID < 5; layerID++)
        {
            if (depthValue < dirLight[layerID].direction.w)
            {
                layer = layerID;
                break;
            }
        }
            
        if (layer == -1)
        {
            layer = 5;
        }
        lightSpaceFragPos = mul(dirLight[layer].viewMatrix, FragWorldPos).xyzw;
        lightSpaceCamPos = mul(dirLight[layer].viewMatrix, cameraPosition).xyz;
    
    
        negDir = normalize(lightSpaceCamPos.xyz - lightSpaceFragPos.xyz);
        raymarchDistance = (clamp(length(lightSpaceCamPos.xyz - lightSpaceFragPos.xyz), 0.0f, rayMarchDistanceLimit));
        ExecuteRaymarch(rayPosition, negDir, stepSize, layer, l, VLI);
    }
    
    float3 color = u_ColorTexture.SampleLevel(ClampSampler, input.uv, 0).rgb;
    
    
    AtmosphereParameters Atmosphere = GetAtmosphereParameters();
    float4 ueWorldPos = float4(0.0f, 0.0f, 0.0f, 1.f);
    float3 P0 = ueWorldPos.xyz / ueWorldPos.w + float3(0, 0, Atmosphere.BottomRadius);
    float viewHeight = length(P0);
    const float3 UpVector = P0 / viewHeight;
    float viewZenithCosAngle = dot(sun_direction, UpVector);
    float2 uvTrans;
    LutTransmittanceParamsToUv(Atmosphere, viewHeight, viewZenithCosAngle, uvTrans);
    float3 col = u_TransmittanceTexture.SampleLevel(ClampSampler, uvTrans, 0).xyz;
    float3 finalColor = (dirLight[0].colorAndIntensity.rgb * col) * clamp(VLI, VLI, float3(0.2, 0.2, 0.2));
 
        color += finalColor;
    
    
    
    return float4(color, 1.f);
}