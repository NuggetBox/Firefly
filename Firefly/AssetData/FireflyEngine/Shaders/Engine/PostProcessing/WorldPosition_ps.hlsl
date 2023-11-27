#include <Includes/PBRFunctions.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>
#include <Includes/ShadowFunctions.hlsli>

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float4 color0 : COLOR0;
    float4 color1 : COLOR1;
    float4 color2 : COLOR2;
    float4 color3 : COLOR3;
    float2 texcoord0 : UV0;
    float2 texcoord1 : UV1;
    float2 texcoord2 : UV2;
    float2 texcoord3 : UV3;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
};

sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);

Texture2D albedo : register(t0);

float4 main(PixelInput pInput) : SV_Target
{
    float alpha = albedo.Sample(WrapSampler, pInput.texcoord0.xy).a;
    
    if (alpha < 0.9f)
    {
        discard;
        return float4(0, 0, 0, 0);
    }
    return pInput.worldPosition;
}