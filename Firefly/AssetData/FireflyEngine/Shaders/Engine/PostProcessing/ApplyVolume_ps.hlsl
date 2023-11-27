#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/VolumetricFogCommon.hlsli>
#include <Includes/Conststructs.hlsli>

Texture2D preDepth : register(t0);
Texture3D<float4> lightVolume : register(t1);
Texture2D framebuffer : register(t2);
static const float StepSize = 0.015625f;
float4 main(VertextoPixel input) : SV_Target
{
    const float depth = preDepth.Sample(ClampSampler, input.uv).r;
    const float4 color = framebuffer.Sample(ClampSampler, input.uv).rgba;
    const float linearizedDepth = LinearizeDepth(depth, nearPlane, 10000);
    
    uint mips = 0;
    uint width = 0;
    uint height = 0;
    uint slices = 0;
    lightVolume.GetDimensions(0, width, height, slices, mips);
    
    int2 texelCoord = int2(input.uv.x * width, input.uv.y * height);
    
    float3 VLI = 1.f;
    // sample light line til depth.
    for (float step = 0; step < (linearizedDepth * 64); step++)
    {
        float4 froxelVLI = lightVolume.SampleLevel(WrapSampler, float3(texelCoord.x / width, texelCoord.y / height, step / slices), 0).rgba;
        VLI += froxelVLI.rgb /** froxelVLI.a*/;
        
    }
    
    return float4(color.xyz * VLI, 1);
}