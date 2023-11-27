// Copyright Epic Games, Inc. All Rights Reserved.

#define PI 3.1415926535897932384626433832795f
sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);
Texture2D<float4> TransmittanceLutTexture : register(t40);
Texture2D<float4> MultiScatTexture : register(t50);

cbuffer HelperValues : register(b7)
{
    float4x4 gViewProjMat;

    float4 gColor;

    float3 gSunIlluminance;
    int gScatteringMaxPathDepth;

    uint2 gResolution;
    float gFrameTimeSec;
    float gTimeSec;

    uint2 gMouseLastDownPos;
    uint gFrameId;
    uint gTerrainResolution;
    float gScreenshotCaptureActive;

    float2 RayMarchMinMaxSPP;
    float pad60057457;
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

// - r0: ray origin
// - rd: normalized ray direction
// - s0: sphere center
// - sR: sphere radius
// - Returns distance from r0 to first intersecion with sphere,
//   or -1.0 if no intersection.
static float NEG_ONE = -1.f;
#pragma warning( disable : 4000 )
float raySphereIntersectNearest(float3 r0, float3 rd, float3 s0, float sR)
{
    float a = dot(rd, rd);
    float3 s0_r0 = r0 - s0;
    float b = 2.0f * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sR * sR);
    float delta = b * b - 4.0f * a * c;
    if (delta < 0.0 || a == 0.0f)
    {
        return NEG_ONE;
    }
    float sol0 = (-b - sqrt(delta)) / (2.0f * a);
    float sol1 = (-b + sqrt(delta)) / (2.0f * a);
    if (sol0 < 0.0f && sol1 < 0.0f)
    {
        return NEG_ONE;
    }
    if (sol0 < 0.0f)
    {
        return max(0.0f, sol1);
    }
    else if (sol1 < 0.0f)
    {
        return max(0.0f, sol0);
    }
    return max(0.0f, min(sol0, sol1));
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




#define DEBUGENABLED 0
#define ToDebugWorld float3(0, 0, -ptc.Atmosphere.BottomRadius)

#define RAYDPOS 0.00001f

#ifndef GROUND_GI_ENABLED
#define GROUND_GI_ENABLED 0
#endif
#ifndef TRANSMITANCE_METHOD
#define TRANSMITANCE_METHOD 2
#endif
#ifndef MULTISCATAPPROX_ENABLED 
#define MULTISCATAPPROX_ENABLED 0 
#endif
#ifndef GAMEMODE_ENABLED 
#define GAMEMODE_ENABLED 0 
#endif
#ifndef COLORED_TRANSMITTANCE_ENABLED 
#define COLORED_TRANSMITTANCE_ENABLED 0 
#endif
#ifndef FASTSKY_ENABLED 
#define FASTSKY_ENABLED 0 
#endif
#ifndef FASTAERIALPERSPECTIVE_ENABLED 
#define FASTAERIALPERSPECTIVE_ENABLED 0 
#endif
#ifndef SHADOWMAP_ENABLED 
#define SHADOWMAP_ENABLED 0 
#endif
#ifndef MEAN_ILLUM_MODE 
#define MEAN_ILLUM_MODE 0 
#endif

#define RENDER_SUN_DISK 1

#if 1
#define USE_CornetteShanks
#define MIE_PHASE_IMPORTANCE_SAMPLING 0
#else
// Beware: untested, probably faulty code path.
// Mie importance sampling is only used for multiple scattering. Single scattering is fine and noise only due to sample selection on view ray.
// A bit more expenssive so off for now.
#define MIE_PHASE_IMPORTANCE_SAMPLING 1
#endif


#define PLANET_RADIUS_OFFSET 0.01f

struct Ray
{
    float3 o;
    float3 d;
};

Ray createRay(in float3 p, in float3 d)
{
    Ray r;
    r.o = p;
    r.d = d;
    return r;
}


////////////////////////////////////////////////////////////
// LUT functions
////////////////////////////////////////////////////////////



// Transmittance LUT function parameterisation from Bruneton 2017 https://github.com/ebruneton/precomputed_atmospheric_scattering
// uv in [0,1]
// viewZenithCosAngle in [-1,1]
// viewHeight in [bottomRAdius, topRadius]

// We should precompute those terms from resolutions (Or set resolution as #defined constants)
float fromUnitToSubUvs(float u, float resolution)
{
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f));
}
float fromSubUvsToUnit(float u, float resolution)
{
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f));
}

void UvToLutTransmittanceParams(AtmosphereParameters Atmosphere, out float viewHeight, out float viewZenithCosAngle, in float2 uv)
{
	//uv = float2(fromSubUvsToUnit(uv.x, TRANSMITTANCE_TEXTURE_WIDTH), fromSubUvsToUnit(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT)); // No real impact so off
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(Atmosphere.TopRadius * Atmosphere.TopRadius - Atmosphere.BottomRadius * Atmosphere.BottomRadius);
    float rho = H * x_r;
    viewHeight = sqrt(rho * rho + Atmosphere.BottomRadius * Atmosphere.BottomRadius);

    float d_min = Atmosphere.TopRadius - viewHeight;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    viewZenithCosAngle = d == 0.0 ? 1.0f : (H * H - rho * rho - d * d) / (2.0 * viewHeight * d);
    viewZenithCosAngle = clamp(viewZenithCosAngle, -1.0, 1.0);
}

#define NONLINEARSKYVIEWLUT 1
void UvToSkyViewLutParams(AtmosphereParameters Atmosphere, out float viewZenithCosAngle, out float lightViewCosAngle, in float viewHeight, in float2 uv)
{
	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
    uv = float2(fromSubUvsToUnit(uv.x, 192.0f), fromSubUvsToUnit(uv.y, 108.0f));

    float Vhorizon = sqrt(viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius);
    float CosBeta = Vhorizon / viewHeight; // GroundToHorizonCos
    float Beta = acos(CosBeta);
    float ZenithHorizonAngle = PI - Beta;

    if (uv.y < 0.5f)
    {
        float coord = 2.0 * uv.y;
        coord = 1.0 - coord;
#if NONLINEARSKYVIEWLUT
        coord *= coord;
#endif
        coord = 1.0 - coord;
        viewZenithCosAngle = cos(ZenithHorizonAngle * coord);
    }
    else
    {
        float coord = uv.y * 2.0 - 1.0;
#if NONLINEARSKYVIEWLUT
        coord *= coord;
#endif
        viewZenithCosAngle = cos(ZenithHorizonAngle + Beta * coord);
    }

    float coord = uv.x;
    coord *= coord;
    lightViewCosAngle = -(coord * 2.0 - 1.0);
}

float3 GetSunLuminance(float3 WorldPos, float3 WorldDir, float PlanetRadius)
{
#if RENDER_SUN_DISK
    if (dot(WorldDir, sun_direction) > cos(0.5 * 0.505 * 3.14159 / 180.0))
    {
        float t = raySphereIntersectNearest(WorldPos, WorldDir, float3(0.0f, 0.0f, 0.0f), PlanetRadius);
        if (t < 0.0f) // no intersection
        {
            const float3 SunLuminance = 1000000.0; // arbitrary. But fine, not use when comparing the models
            return SunLuminance * (1.0 - gScreenshotCaptureActive);
        }
    }
#endif
    return 0;
}


float3 GetMultipleScattering(AtmosphereParameters Atmosphere, float3 scattering, float3 extinction, float3 worlPos, float viewZenithCosAngle)
{
    float2 uv = saturate(float2(viewZenithCosAngle * 0.5f + 0.5f, (length(worlPos) - Atmosphere.BottomRadius) / (Atmosphere.TopRadius - Atmosphere.BottomRadius)));
    uv = float2(fromUnitToSubUvs(uv.x, MultiScatteringLUTRes), fromUnitToSubUvs(uv.y, MultiScatteringLUTRes));

    float3 multiScatteredLuminance = MultiScatTexture.SampleLevel(ClampSampler, uv, 0).rgb;
    return multiScatteredLuminance;
}

void SkyViewLutParamsToUv(AtmosphereParameters Atmosphere, in bool IntersectGround, in float viewZenithCosAngle, in float lightViewCosAngle, in float viewHeight, out float2 uv)
{
    float Vhorizon = sqrt(viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius);
    float CosBeta = Vhorizon / viewHeight; // GroundToHorizonCos
    float Beta = acos(CosBeta);
    float ZenithHorizonAngle = PI - Beta;

    if (!IntersectGround)
    {
        float coord = acos(viewZenithCosAngle) / ZenithHorizonAngle;
        coord = 1.0 - coord;
#if NONLINEARSKYVIEWLUT
        coord = sqrt(coord);
#endif
        coord = 1.0 - coord;
        uv.y = coord * 0.5f;
    }
    else
    {
        float coord = (acos(viewZenithCosAngle) - ZenithHorizonAngle) / Beta;
#if NONLINEARSKYVIEWLUT
        coord = sqrt(coord);
#endif
        uv.y = coord * 0.5f + 0.5f;
    }

	{
        float coord = -lightViewCosAngle * 0.5f + 0.5f;
        coord = sqrt(coord);
        uv.x = coord;
    }

	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
    uv = float2(fromUnitToSubUvs(uv.x, 192.0f), fromUnitToSubUvs(uv.y, 108.0f));
}


float getAlbedo(float scattering, float extinction)
{
    return scattering / max(0.001, extinction);
}
float3 getAlbedo(float3 scattering, float3 extinction)
{
    return scattering / max(0.001, extinction);
}


struct MediumSampleRGB
{
    float3 scattering;
    float3 absorption;
    float3 extinction;

    float3 scatteringMie;
    float3 absorptionMie;
    float3 extinctionMie;

    float3 scatteringRay;
    float3 absorptionRay;
    float3 extinctionRay;

    float3 scatteringOzo;
    float3 absorptionOzo;
    float3 extinctionOzo;

    float3 albedo;
};

MediumSampleRGB sampleMediumRGB(in float3 WorldPos, in AtmosphereParameters Atmosphere)
{
    const float viewHeight = length(WorldPos) - Atmosphere.BottomRadius;

    const float densityMie = exp(Atmosphere.MieDensityExpScale * viewHeight);
    const float densityRay = exp(Atmosphere.RayleighDensityExpScale * viewHeight);
    const float densityOzo = saturate(viewHeight < Atmosphere.AbsorptionDensity0LayerWidth ?
		Atmosphere.AbsorptionDensity0LinearTerm * viewHeight + Atmosphere.AbsorptionDensity0ConstantTerm :
		Atmosphere.AbsorptionDensity1LinearTerm * viewHeight + Atmosphere.AbsorptionDensity1ConstantTerm);

    MediumSampleRGB s;

    s.scatteringMie = densityMie * Atmosphere.MieScattering;
    s.absorptionMie = densityMie * Atmosphere.MieAbsorption;
    s.extinctionMie = densityMie * Atmosphere.MieExtinction;

    s.scatteringRay = densityRay * Atmosphere.RayleighScattering;
    s.absorptionRay = 0.0f;
    s.extinctionRay = s.scatteringRay + s.absorptionRay;

    s.scatteringOzo = 0.0;
    s.absorptionOzo = densityOzo * Atmosphere.AbsorptionExtinction;
    s.extinctionOzo = s.scatteringOzo + s.absorptionOzo;

    s.scattering = s.scatteringMie + s.scatteringRay + s.scatteringOzo;
    s.absorption = s.absorptionMie + s.absorptionRay + s.absorptionOzo;
    s.extinction = s.extinctionMie + s.extinctionRay + s.extinctionOzo;
    s.albedo = getAlbedo(s.scattering, s.extinction);

    return s;
}

////////////////////////////////////////////////////////////
// Sampling functions
////////////////////////////////////////////////////////////



// Generates a uniform distribution of directions over a sphere.
// Random zetaX and zetaY values must be in [0, 1].
// Top and bottom sphere pole (+-zenith) are along the Y axis.
float3 getUniformSphereSample(float zetaX, float zetaY)
{
    float phi = 2.0f * 3.14159f * zetaX;
    float theta = 2.0f * acos(sqrt(1.0f - zetaY));
    float3 dir = float3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
    return dir;
}

// Generate a sample (using importance sampling) along an infinitely long path with a given constant extinction.
// Zeta is a random number in [0,1]
float infiniteTransmittanceIS(float extinction, float zeta)
{
    return -log(1.0f - zeta) / extinction;
}
// Normalized PDF from a sample on an infinitely long path according to transmittance and extinction.
float infiniteTransmittancePDF(float extinction, float transmittance)
{
    return extinction * transmittance;
}

// Same as above but a sample is generated constrained within a range t,
// where transmittance = exp(-extinction*t) over that range.
float rangedTransmittanceIS(float extinction, float transmittance, float zeta)
{
    return -log(1.0f - zeta * (1.0f - transmittance)) / extinction;
}



float RayleighPhase(float cosTheta)
{
    float factor = 3.0f / (16.0f * PI);
    return factor * (1.0f + cosTheta * cosTheta);
}

float CornetteShanksMiePhaseFunction(float g, float cosTheta)
{
    float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + cosTheta * cosTheta) / pow(1.0 + g * g - 2.0 * g * -cosTheta, 1.5);
}

float hgPhase(float g, float cosTheta)
{
	// Reference implementation (i.e. not schlick approximation). 
	// See http://www.pbr-book.org/3ed-2018/Volume_Scattering/Phase_Functions.html
    float numer = 1.0f - g * g;
    float denom = 1.0f + g * g + 2.0f * g * cosTheta;
    if (denom <= 0)
    {
        return 0.f;
    }
    return numer / (4.0f * PI * denom * sqrt(denom));
}

float dualLobPhase(float g0, float g1, float w, float cosTheta)
{
    return lerp(hgPhase(g0, cosTheta), hgPhase(g1, cosTheta), w);
}

float uniformPhase()
{
    return 1.0f / (4.0f * PI);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool MoveToTopAtmosphere(inout float3 WorldPos, in float3 WorldDir, in float AtmosphereTopRadius)
{
    float viewHeight = length(WorldPos);
    if (viewHeight > AtmosphereTopRadius)
    {
        float tTop = raySphereIntersectNearest(WorldPos, WorldDir, float3(0.0f, 0.0f, 0.0f), AtmosphereTopRadius);
        if (tTop >= 0.0f)
        {
            float3 UpVector = WorldPos / viewHeight;
            float3 UpOffset = UpVector * -PLANET_RADIUS_OFFSET;
            WorldPos = WorldPos + WorldDir * tTop + UpOffset;
        }
        else
        {
			// Ray is not intersecting the atmosphere
            return false;
        }
    }
    return true; // ok to start tracing
}

struct SingleScatteringResult
{
    float3 L; // Scattered light (luminance)
    float3 OpticalDepth; // Optical depth (1/m)
    float3 Transmittance; // Transmittance in [0,1] (unitless)
    float3 MultiScatAs1;

    float3 NewMultiScatStep0Out;
    float3 NewMultiScatStep1Out;
};

SingleScatteringResult IntegrateScatteredLuminance(
	in float2 pixPos, in float3 WorldPos, in float3 WorldDir, in float3 SunDir, in AtmosphereParameters Atmosphere,
	in bool ground, in float SampleCountIni, in float DepthBufferValue, in bool VariableSampleCount,
	in bool MieRayPhase, in float tMaxMax = 9000000.0f)
{
    const bool debugEnabled = all(uint2(pixPos.xx) == gMouseLastDownPos.xx) && uint(pixPos.y) % 10 == 0 && DepthBufferValue != -1.0f;
    SingleScatteringResult result = (SingleScatteringResult) 0;

    float3 ClipSpace = float3((pixPos / float2(gResolution)) * float2(2.0, -2.0) - float2(1.0, -1.0), 1.0);

	// Compute next intersection with atmosphere or ground 
    float3 earthO = float3(0.0f, 0.0f, 0.0f);
    float tBottom = raySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.BottomRadius);
    float tTop = raySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.TopRadius);
    float tMax = 0.0f;
    if (tBottom < 0.0f)
    {
        if (tTop < 0.0f)
        {
            tMax = 0.0f; // No intersection with earth nor atmosphere: stop right away  
            return result;
        }
        else
        {
            tMax = tTop;
        }
    }
    else
    {
        if (tTop > 0.0f)
        {
            tMax = min(tTop, tBottom);
        }
    }

    if (DepthBufferValue >= 0.0f)
    {
        ClipSpace.z = DepthBufferValue;
        if (ClipSpace.z < 1.0f)
        {
            float4 DepthBufferWorldPos = mul(gSkyInvViewProjMat, float4(ClipSpace, 1.0));
            DepthBufferWorldPos /= DepthBufferWorldPos.w;

            float tDepth = length(DepthBufferWorldPos.xyz - (WorldPos + float3(0.0, 0.0, -Atmosphere.BottomRadius))); // apply earth offset to go back to origin as top of earth mode. 
            if (tDepth < tMax)
            {
                tMax = tDepth;
            }
        }
		//		if (VariableSampleCount && ClipSpace.z == 1.0f)
		//			return result;
    }
    tMax = min(tMax, tMaxMax);

	// Sample count 
    float SampleCount = SampleCountIni;
    float SampleCountFloor = SampleCountIni;
    float tMaxFloor = tMax;
    if (VariableSampleCount)
    {
        SampleCount = lerp(RayMarchMinMaxSPP.x, RayMarchMinMaxSPP.y, saturate(tMax * 0.01));
        SampleCountFloor = floor(SampleCount);
        tMaxFloor = tMax * SampleCountFloor / SampleCount; // rescale tMax to map to the last entire step segment.
    }
    float dt = tMax / SampleCount;

	// Phase functions
    const float uniformPhase = 1.0 / (4.0 * PI);
    const float3 wi = SunDir;
    const float3 wo = WorldDir;
    float cosTheta = dot(wi, wo);
    float MiePhaseValue = hgPhase(Atmosphere.MiePhaseG, -cosTheta); // mnegate cosTheta because due to WorldDir being a "in" direction. 
    float RayleighPhaseValue = RayleighPhase(cosTheta);

#ifdef ILLUMINANCE_IS_ONE
	// When building the scattering factor, we assume light illuminance is 1 to compute a transfert function relative to identity illuminance of 1.
	// This make the scattering factor independent of the light. It is now only linked to the atmosphere properties.
	float3 globalL = 1.0f;
#else
    float3 globalL = gSunIlluminance;
#endif

	// Ray march the atmosphere to integrate optical depth
    float3 L = 0.0f;
    float3 throughput = 1.0;
    float3 OpticalDepth = 0.0;
    float t = 0.0f;
    float tPrev = 0.0;
    const float SampleSegmentT = 0.3f;
    for (float s = 0.0f; s < SampleCount; s += 1.0f)
    {
        if (VariableSampleCount)
        {
			// More expenssive but artefact free
            float t0 = (s) / SampleCountFloor;
            float t1 = (s + 1.0f) / SampleCountFloor;
			// Non linear distribution of sample within the range.
            t0 = t0 * t0;
            t1 = t1 * t1;
			// Make t0 and t1 world space distances.
            t0 = tMaxFloor * t0;
            if (t1 > 1.0)
            {
                t1 = tMax;
				//	t1 = tMaxFloor;	// this reveal depth slices
            }
            else
            {
                t1 = tMaxFloor * t1;
            }
			//t = t0 + (t1 - t0) * (whangHashNoise(pixPos.x, pixPos.y, gFrameId * 1920 * 1080)); // With dithering required to hide some sampling artefact relying on TAA later? This may even allow volumetric shadow?
            t = t0 + (t1 - t0) * SampleSegmentT;
            dt = t1 - t0;
        }
        else
        {
			//t = tMax * (s + SampleSegmentT) / SampleCount;
			// Exact difference, important for accuracy of multiple scattering
            float NewT = tMax * (s + SampleSegmentT) / SampleCount;
            dt = NewT - t;
            t = NewT;
        }
        float3 P = WorldPos + t * WorldDir;

#if DEBUGENABLED 
		if (debugEnabled)
		{
			float3 Pprev = WorldPos + tPrev * WorldDir;
			float3 TxToDebugWorld = float3(0, 0, -Atmosphere.BottomRadius);
			addGpuDebugLine(TxToDebugWorld + Pprev, TxToDebugWorld + P, float3(0.2, 1, 0.2));
			addGpuDebugCross(TxToDebugWorld + P, float3(0.2, 0.2, 1.0), 0.2);
		}
#endif

        MediumSampleRGB medium = sampleMediumRGB(P, Atmosphere);
        const float3 SampleOpticalDepth = medium.extinction * dt;
        const float3 SampleTransmittance = exp(-SampleOpticalDepth);
        OpticalDepth += SampleOpticalDepth;

        float pHeight = length(P);
        const float3 UpVector = P / pHeight;
        float SunZenithCosAngle = dot(SunDir, UpVector);
        float2 uv;
        LutTransmittanceParamsToUv(Atmosphere, pHeight, SunZenithCosAngle, uv);
        float3 TransmittanceToSun = TransmittanceLutTexture.SampleLevel(ClampSampler, uv, 0).rgb;

        float3 PhaseTimesScattering;
        if (MieRayPhase)
        {
            PhaseTimesScattering = medium.scatteringMie * MiePhaseValue + medium.scatteringRay * RayleighPhaseValue;
        }
        else
        {
            PhaseTimesScattering = medium.scattering * uniformPhase;
        }

		// Earth shadow 
        float tEarth = raySphereIntersectNearest(P, SunDir, earthO + PLANET_RADIUS_OFFSET * UpVector, Atmosphere.BottomRadius);
        float earthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;

		// Dual scattering for multi scattering 

        float3 multiScatteredLuminance = 0.0f;
#if MULTISCATAPPROX_ENABLED
		multiScatteredLuminance = GetMultipleScattering(Atmosphere, medium.scattering, medium.extinction, P, SunZenithCosAngle);
#endif

        float shadow = 1.0f;
#if SHADOWMAP_ENABLED
		// First evaluate opaque shadow
		shadow = getShadow(Atmosphere, P);
#endif

        float3 S = globalL * (earthShadow * shadow * TransmittanceToSun * PhaseTimesScattering + multiScatteredLuminance * medium.scattering);

		// When using the power serie to accumulate all sattering order, serie r must be <1 for a serie to converge.
		// Under extreme coefficient, MultiScatAs1 can grow larger and thus result in broken visuals.
		// The way to fix that is to use a proper analytical integration as proposed in slide 28 of http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/
		// However, it is possible to disable as it can also work using simple power serie sum unroll up to 5th order. The rest of the orders has a really low contribution.
#define MULTI_SCATTERING_POWER_SERIE 1

#if MULTI_SCATTERING_POWER_SERIE==0
		// 1 is the integration of luminance over the 4pi of a sphere, and assuming an isotropic phase function of 1.0/(4*PI)
		result.MultiScatAs1 += throughput * medium.scattering * 1 * dt;
#else
        float3 MS = medium.scattering * 1;
        float3 MSint = (MS - MS * SampleTransmittance) / medium.extinction;
        result.MultiScatAs1 += throughput * MSint;
#endif

		// Evaluate input to multi scattering 
		{
            float3 newMS;

            newMS = earthShadow * TransmittanceToSun * medium.scattering * uniformPhase * 1;
            result.NewMultiScatStep0Out += throughput * (newMS - newMS * SampleTransmittance) / medium.extinction;
			//	result.NewMultiScatStep0Out += SampleTransmittance * throughput * newMS * dt;

            newMS = medium.scattering * uniformPhase * multiScatteredLuminance;
            result.NewMultiScatStep1Out += throughput * (newMS - newMS * SampleTransmittance) / medium.extinction;
			//	result.NewMultiScatStep1Out += SampleTransmittance * throughput * newMS * dt;
        }

#if 0
		L += throughput * S * dt;
		throughput *= SampleTransmittance;
#else
		// See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
        float3 Sint = (S - S * SampleTransmittance) / medium.extinction; // integrate along the current step segment 
        L += throughput * Sint; // accumulate and also take into account the transmittance from previous steps
        throughput *= SampleTransmittance;
#endif

        tPrev = t;
    }

    if (ground && tMax == tBottom && tBottom > 0.0)
    {
		// Account for bounced light off the earth
        float3 P = WorldPos + tBottom * WorldDir;
        float pHeight = length(P);

        const float3 UpVector = P / pHeight;
        float SunZenithCosAngle = dot(SunDir, UpVector);
        float2 uv;
        LutTransmittanceParamsToUv(Atmosphere, pHeight, SunZenithCosAngle, uv);
        float3 TransmittanceToSun = TransmittanceLutTexture.SampleLevel(ClampSampler, uv, 0).rgb;

        const float NdotL = saturate(dot(normalize(UpVector), normalize(SunDir)));
        L += globalL * TransmittanceToSun * throughput * NdotL * Atmosphere.GroundAlbedo / PI;
    }

    result.L = L;
    result.OpticalDepth = OpticalDepth;
    result.Transmittance = throughput;
    return result;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
