#include <Includes/VolumetricFogCommon.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <includes/ShadowFunctions.hlsli>
#include <Includes/Math.hlsli>

struct ComputeInputs
{
    uint3 ThreadId : SV_DispatchThreadID;
};


Texture3D<float4> u_FroxelVolume : register(t50);
sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);
RWTexture3D<float4> u_LightTexture : register(u0);

float SliceDistance(int z)
{
    float n = nearPlane;
    float f = 500000;

    return n * pow(max(f / n, 0), (float(z) + 0.5f) / float(FROXEL_GRID_SIZE_Z));
}

// ------------------------------------------------------------------

float SliceThickness(int z)
{
    return abs(SliceDistance(z + 1) - SliceDistance(z));
}


// https://github.com/Unity-Technologies/VolumetricLighting/blob/master/Assets/VolumetricFog/Shaders/Scatter.compute
float4 Accumulate(int z, float3 accum_scattering, float accum_transmittance, float3 slice_scattering, float slice_density)
{
    const float thickness = SliceThickness(z);
    const float slice_transmittance = exp(-slice_density * thickness * 0.01f);

    float3 slice_scattering_integral = slice_scattering * (1.0 - slice_transmittance) / slice_density;

    accum_scattering += slice_scattering_integral * accum_transmittance;
    accum_transmittance *= slice_transmittance;

    return float4(accum_scattering, accum_transmittance);
}

[numthreads(8, 8, 1)]
void main(ComputeInputs input)
{
    float4 accumilationScatteringTransmittance = float4(0, 0, 0, 1);
    
    for (uint z = 0; z < FROXEL_GRID_SIZE_Z; z += 2)
    {
        int3 coord = int3(input.ThreadId.xy, z);
        int3 coord2 = int3(input.ThreadId.xy, z + 1);
        float4 sliceScatterDensity = u_FroxelVolume.Load(int4(coord, 0));
        float4 sliceScatterDensity2 = u_FroxelVolume.Load(int4(coord2, 0));
        accumilationScatteringTransmittance = Accumulate(z, accumilationScatteringTransmittance.rgb, accumilationScatteringTransmittance.a, sliceScatterDensity.rgb, sliceScatterDensity.a);
        u_LightTexture[coord] = accumilationScatteringTransmittance;
        
        accumilationScatteringTransmittance = Accumulate(z + 1, accumilationScatteringTransmittance.rgb, accumilationScatteringTransmittance.a, sliceScatterDensity2.rgb, sliceScatterDensity2.a);
        u_LightTexture[coord2] = accumilationScatteringTransmittance;
    }
    
}