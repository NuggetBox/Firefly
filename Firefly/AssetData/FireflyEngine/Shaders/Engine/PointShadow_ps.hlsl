#include <Includes/ConstStructs.hlsli>

struct GSOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    uint layer : SV_RenderTargetArrayIndex;
};

float computeDepth(float3 pos, float4x4 viewProj)
{
    float4 clip_space_pos = mul(viewProj, float4(pos.xyz, 1.0));
    return (clip_space_pos.z / clip_space_pos.w);
}

float main(GSOutput gInput) : SV_Depth
{
    float lightDistance = length(gInput.worldPos.xyz - pointLight[nearPlane].position.xyz);
    
    lightDistance = lightDistance / pointLight[nearPlane].radius;
    
    return min(lightDistance, 1);
}