#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/PostProcessStructs.hlsli>
#include <Includes/Math.hlsli>

Texture2D WorldPosition : register(t0);
Texture2D normals : register(t1);
Texture2D noiseTex : register(t2);
Texture2D depthTexture : register(t3);

StructuredBuffer<float4> kernels : register(t100);

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


static int kernelSize = 32;
static float bias = 0.01f;
static float radius = 100.f;
float4 main(VertextoPixel pInput) : SV_TARGET
{    
    float4x4 inverseVP = mul(inverse(toView), inverse(toProjection));
    
    float depth = depthTexture.SampleLevel(ClampSampler, pInput.uv, 0).r;
    float3 position = ReconstructPositionFromDepth(pInput.uv, depth, inverseVP);
    float2 noiseScale = float2(resolution.x / 4.f, resolution.y / 4.f);
    float3 normal = normals.Sample(ClampSampler, pInput.uv).rgb;
    float3 randomVec = (noiseTex.SampleLevel(WrapSampler, pInput.uv * noiseScale, 0.f).rgb);
    
    float3 tangent = (randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(tangent, normal);
    
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    TBN = transpose(TBN);
    
    float occlusion = 1.f;
    float positionDepth = mul(toView, float4(position, 1.f)).z;

    [loop]
    for (int i = 0; i < kernelSize; i++)
    {
        float4 samplePos = mul(toView, float4(position + mul(TBN, kernels[i].xyz) * radius, 1.f));

        float4 offset = mul(toProjection, samplePos);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5f;
        depth = depthTexture.SampleLevel(ClampSampler, float2(offset.x, (1.f - offset.y)), 0).r;
        float sampleDepth = mul(toView, float4(ReconstructPositionFromDepth(float2(offset.x, (1.f - offset.y)), depth, inverseVP), 1.f)).z;

        float rangeCheck = smoothstep(0.f, 1.f, radius * abs(positionDepth - sampleDepth));
        occlusion -= samplePos.z + bias < sampleDepth ? rangeCheck / kernelSize : 0.f;
    }
    
    occlusion = 1 - pow(clamp((occlusion), 0.f, 1.f), 1.f);
    return float4(occlusion.xxx, 1.f);
}