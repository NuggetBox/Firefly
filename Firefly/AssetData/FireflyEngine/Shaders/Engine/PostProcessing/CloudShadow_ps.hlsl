#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/PostProcessStructs.hlsli>

Texture2D colorBuffer : register(t0);
Texture2D worldPositions : register(t1);
Texture2D noise : register(t2);


float4 main(VertextoPixel input) : SV_Target
{
    float3 worldPos = colorBuffer.Sample(ClampSampler, input.uv).xyz;
   
    return float4(worldPos, 1);
}