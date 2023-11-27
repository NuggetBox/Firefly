// Huge thanks to Unreal engine team at Epic games https://sebh.github.io/publications/egsr2020.pdf
// Copyright Epic Games, Inc. All Rights Reserved.

#include <Includes/PostProcessStructs.hlsli>
#include "SkyAtmosphereCommon.hlsl"


RWTexture2D<float4> multiScatter : register(u0);

groupshared float3 MultiScatAs1SharedMem[64];
groupshared float3 LSharedMem[64];
[numthreads(1, 1, 64)]
void main(uint3 ThreadId : SV_DispatchThreadID)
{
    float2 pixPos = float2(ThreadId.xy) + 0.5f;
    float2 uv = pixPos / MultiScatteringLUTRes;


    uv = float2(fromSubUvsToUnit(uv.x, MultiScatteringLUTRes), fromSubUvsToUnit(uv.y, MultiScatteringLUTRes));

    AtmosphereParameters Atmosphere = GetAtmosphereParameters();

    float cosSunZenithAngle = uv.x * 2.0 - 1.0;
    float3 sunDir = float3(0.0, sqrt(saturate(1.0 - cosSunZenithAngle * cosSunZenithAngle)), cosSunZenithAngle);
	// We adjust again viewHeight according to PLANET_RADIUS_OFFSET to be in a valid range.
    float viewHeight = Atmosphere.BottomRadius + saturate(uv.y + PLANET_RADIUS_OFFSET) * (Atmosphere.TopRadius - Atmosphere.BottomRadius - PLANET_RADIUS_OFFSET);

    float3 WorldPos = float3(0.0f, 0.0f, viewHeight);
    float3 WorldDir = float3(0.0f, 0.0f, 1.0f);


    const bool ground = true;
    const float SampleCountIni = 20; // a minimum set of step is required for accuracy unfortunately
    const float DepthBufferValue = -1.0;
    const bool VariableSampleCount = false;
    const bool MieRayPhase = false;

    const float SphereSolidAngle = 4.0 * PI;
    const float IsotropicPhase = 1.0 / SphereSolidAngle;


	// Reference. Since there are many sample, it requires MULTI_SCATTERING_POWER_SERIE to be true for accuracy and to avoid divergences (see declaration for explanations)
#define SQRTSAMPLECOUNT 8
    const float sqrtSample = float(SQRTSAMPLECOUNT);
    float i = 0.5f + float(ThreadId.z / SQRTSAMPLECOUNT);
    float j = 0.5f + float(ThreadId.z - float((ThreadId.z / SQRTSAMPLECOUNT) * SQRTSAMPLECOUNT));
	{
        float randA = i / sqrtSample;
        float randB = j / sqrtSample;
        float theta = 2.0f * PI * randA;
        float phi = acos(1.0f - 2.0f * randB); // uniform distribution https://mathworld.wolfram.com/SpherePointPicking.html
		//phi = PI * randB;						// bad non uniform
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        WorldDir.x = cosTheta * sinPhi;
        WorldDir.y = sinTheta * sinPhi;
        WorldDir.z = cosPhi;
        SingleScatteringResult result = IntegrateScatteredLuminance(pixPos, WorldPos, WorldDir, sunDir, Atmosphere, ground, SampleCountIni, DepthBufferValue, VariableSampleCount, MieRayPhase);

        MultiScatAs1SharedMem[ThreadId.z] = result.MultiScatAs1 * SphereSolidAngle / (sqrtSample * sqrtSample);
        LSharedMem[ThreadId.z] = result.L * SphereSolidAngle / (sqrtSample * sqrtSample);
    }
#undef SQRTSAMPLECOUNT

    GroupMemoryBarrierWithGroupSync();

	// 64 to 32
    if (ThreadId.z < 32)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 32];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 32];
    }
    GroupMemoryBarrierWithGroupSync();

	// 32 to 16
    if (ThreadId.z < 16)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 16];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 16];
    }
    GroupMemoryBarrierWithGroupSync();

	// 16 to 8 (16 is thread group min hardware size with intel, no sync required from there)
    if (ThreadId.z < 8)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 8];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 8];
    }
    GroupMemoryBarrierWithGroupSync();
    if (ThreadId.z < 4)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 4];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 4];
    }
    GroupMemoryBarrierWithGroupSync();
    if (ThreadId.z < 2)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 2];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 2];
    }
    GroupMemoryBarrierWithGroupSync();
    if (ThreadId.z < 1)
    {
        MultiScatAs1SharedMem[ThreadId.z] += MultiScatAs1SharedMem[ThreadId.z + 1];
        LSharedMem[ThreadId.z] += LSharedMem[ThreadId.z + 1];
    }
    GroupMemoryBarrierWithGroupSync();
    if (ThreadId.z > 0)
        return;

    float3 MultiScatAs1 = MultiScatAs1SharedMem[0] * IsotropicPhase; // Equation 7 f_ms
    float3 InScatteredLuminance = LSharedMem[0] * IsotropicPhase; // Equation 5 L_2ndOrder

	// MultiScatAs1 represents the amount of luminance scattered as if the integral of scattered luminance over the sphere would be 1.
	//  - 1st order of scattering: one can ray-march a straight path as usual over the sphere. That is InScatteredLuminance.
	//  - 2nd order of scattering: the inscattered luminance is InScatteredLuminance at each of samples of fist order integration. Assuming a uniform phase function that is represented by MultiScatAs1,
	//  - 3nd order of scattering: the inscattered luminance is (InScatteredLuminance * MultiScatAs1 * MultiScatAs1)
	//  - etc.
#if	MULTI_SCATTERING_POWER_SERIE==0
	float3 MultiScatAs1SQR = MultiScatAs1 * MultiScatAs1;
	float3 L = InScatteredLuminance * (1.0 + MultiScatAs1 + MultiScatAs1SQR + MultiScatAs1 * MultiScatAs1SQR + MultiScatAs1SQR * MultiScatAs1SQR);
#else
	// For a serie, sum_{n=0}^{n=+inf} = 1 + r + r^2 + r^3 + ... + r^n = 1 / (1.0 - r), see https://en.wikipedia.org/wiki/Geometric_series 
    const float3 r = MultiScatAs1;
    const float3 SumOfAllMultiScatteringEventsContribution = 1.0f / (1.0 - r);
    float3 L = InScatteredLuminance * SumOfAllMultiScatteringEventsContribution; // Equation 10 Psi_ms
#endif

    multiScatter[ThreadId.xy] = float4(MultipleScatteringFactor * L, 1.0f);
}