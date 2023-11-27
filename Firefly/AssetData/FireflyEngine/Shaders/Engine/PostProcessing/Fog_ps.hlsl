#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/PostProcessStructs.hlsli>
#include <Includes/Math.hlsli>

struct DrawData
{
    float4 color : SV_TARGET;
};
struct Output
{
    float4 position : SV_POSITION;
    float3 near : NEARPOINT;
    float3 far : FARPOINT;
    float4x4 fragView : FRAGVIEW;
    float4x4 fragProj : FRAGPROJ;
};
// MUST match SKYATMOSPHERE_BUFFER in SkyAtmosphereBruneton.hlsl
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

float computeDepth(float3 pos, float4x4 fragProj, float4x4 FragView)
{
    float4 clip_space_pos = mul(fragProj, mul(FragView, float4(pos.xyz, 1.0)));
    return (clip_space_pos.z / clip_space_pos.w);
}

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
float computeLinearDepth(float3 pos, float4x4 fragProj, float4x4 fragView)
{
    float far = 10000.f;
    float near = 0.1f;
    float4 clip_space_pos = mul(fragProj, mul(fragView, float4(pos.xyz, 1.0)));
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linearDepth / far;
}
Texture2D u_DepthTexture : register(t0);
Texture2D u_Transmittance : register(t1);
Texture2D u_WorldPosition : register(t2);

DrawData main(Output input)
{
    DrawData drawData = (DrawData) 0;
    float2 uv = input.position.xy / resolution;
    float t = -input.near.y / (input.far.y - input.near.y);
    if ((t > 0) == false)
    {
        return drawData;
    }
    float3 fragPos3D = input.near + t * (input.far - input.near);
    
    float linearDepth = computeLinearDepth(fragPos3D, input.fragProj, input.fragView);
    float fading = max(0, (0.5f - linearDepth));
    float alpha = 0.5f;
    float planeDepth = computeDepth(fragPos3D, input.fragProj, input.fragView);
    float worldDepth = u_DepthTexture.SampleLevel(ClampSampler, uv, 0).r;
    
    float3 worldPos = u_WorldPosition.SampleLevel(ClampSampler, uv, 0).rgb;
    float lerpValue = 1.f;
    alpha = clamp(worldPos.y, -fogthreshold, 0) / -fogthreshold;
    
    float3 cameraZero = float3(camera.x, 0, camera.z);
    
    float l = length(cameraZero - fragPos3D);
    
    l = l / farPlane;
    
    if (worldDepth < planeDepth)
    {
        alpha = 0.f;
    }
  
    if(worldDepth == 1.f)
    {
        alpha = 1.f;
    }
    if(fogDensity > 0)
    {
        AtmosphereParameters Atmosphere = GetAtmosphereParameters();
        
        float4 ueWorldPos = float4(fragPos3D.z / 1000.f, fragPos3D.x / 1000.f, fragPos3D.y / 1000.f, 1.f);
        float3 P0 = ueWorldPos.xyz / ueWorldPos.w + float3(0, 0, Atmosphere.BottomRadius);
        float viewHeight = length(P0);
        const float3 UpVector = P0 / viewHeight;
        float viewZenithCosAngle = dot(sun_direction, UpVector);
        float2 uvTrans;
        LutTransmittanceParamsToUv(Atmosphere, viewHeight, viewZenithCosAngle, uvTrans);
        float3 col = u_Transmittance.SampleLevel(ClampSampler, uvTrans, 0).xyz;
        drawData.color = float4(col, alpha);
    }
    else
    {
        drawData.color = float4(fogColor.xyz, alpha);
    }
    drawData.color.a *=  1.f - l;
    return drawData;
}