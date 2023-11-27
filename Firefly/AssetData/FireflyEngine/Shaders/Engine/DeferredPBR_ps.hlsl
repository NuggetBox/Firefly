#include <Includes/PBRFunctions.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>
#include <Includes/ShadowFunctions.hlsli>

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float2 texcoord0 : UV0;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
    uint PrimtiveID : ID;
};

sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);

Texture2D AlbedoTexture : register(t0);
Texture2D NormalMapTexture : register(t1);
Texture2D MaterialInfoTexture : register(t2);

struct DefferedOutput
{
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Material : SV_TARGET2;
    float4 VertexNormal : SV_TARGET3;
    float4 WorldPosition : SV_TARGET4;
    float AmbientOcclusion : SV_TARGET5;
    uint2 EntityId : SV_TARGET6;
};

cbuffer MaterialInfo : register(b10)
{
    float4 AlbedoColor_Color = float4(1, 1, 1, 1);
    float ColorStrength = 1.f;
    float RoughnessMultiplier_Slider = 1.f;
    float RoughnessIntensity_Slider = 1.f;
    float NormalMapIntensity_Slider = 1.0f;
    float Emissive_Slider = 1.0f;
    float AmbientOcclusionMapIntensity_Slider = 0.f;
    float Metalness = 0.f;
    float padd;
}

DefferedOutput main(PixelInput input)
{
    DefferedOutput result;
    result.Albedo = 0;
    result.Normal = 0;
    result.Material = 0;
    result.VertexNormal = 0;
    result.WorldPosition = 0;
    result.AmbientOcclusion = 0;
    result.EntityId = uint2(0, 0);
    float4 albedoSample = AlbedoTexture.Sample(ClampSampler, input.texcoord0).rgba;
    if (albedoSample.a <= 0.5f)
    {
        discard;
        return result;
    }

    float3 normalMap = NormalMapTexture.Sample(ClampSampler, input.texcoord0).agb;
    float4 material = MaterialInfoTexture.Sample(ClampSampler, input.texcoord0).rgba;

    
    material.g = pow(abs(material.g), RoughnessIntensity_Slider);
    material.g *= RoughnessMultiplier_Slider;
  
    material.a = Emissive_Slider;
    
    const float AO = normalMap.b;

    material.r += Metalness;
    
    normalMap.z = 0;
    normalMap = 2.0f * normalMap - 1.0f;
    normalMap.z = sqrt(1 - saturate(normalMap.x * normalMap.x + normalMap.y * normalMap.y));

    normalMap = normalize(normalMap);
    
    const float3 normalConstant = float3(0, 0, 1);
    float3 newNormalMap = lerp(normalConstant, normalMap, NormalMapIntensity_Slider);
    
    float3 PixelNormal = normalize(mul(newNormalMap, input.tangentBias));

    
    result.Albedo = albedoSample * AlbedoColor_Color * ColorStrength;
    result.Normal = float4(PixelNormal, 1.f);
    result.Material = material;
    result.VertexNormal = float4(input.normal, 1.0f);
    result.WorldPosition = input.worldPosition;
    result.AmbientOcclusion = lerp(1.f, AO, AmbientOcclusionMapIntensity_Slider);
    //result.EntityId = input.entityID.xy;
    return result;
}