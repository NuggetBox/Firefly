#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>


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

Texture2D u_DepthTexture : register(t0);
Texture2D u_NoiseTexture : register(t1);

float3 ReconstructNormalFromDepth(in float2 uv, in float4x4 inverseViewProj)
{
        // improved normal reconstructed from Wicked engine.
    // https://wickedengine.net/2019/09/22/improved-normal-reconstruction-from-depth/
    const float2 uv0 = uv; // center
    const float2 uv1 = uv + float2(1, 0) / resolution; // right 
    const float2 uv2 = uv + float2(0, 1) / resolution; // top

    const float depth0 = u_DepthTexture.SampleLevel(ClampSampler, uv0, 0).r;
    const float depth1 = u_DepthTexture.SampleLevel(ClampSampler, uv1, 0).r;
    const float depth2 = u_DepthTexture.SampleLevel(ClampSampler, uv2, 0).r;

    const float3 P0 = ReconstructPositionFromDepth(uv0, depth0, inverseViewProj);
    const float3 P1 = ReconstructPositionFromDepth(uv1, depth1, inverseViewProj);
    const float3 P2 = ReconstructPositionFromDepth(uv2, depth2, inverseViewProj);

    const float3 normal = normalize(cross(P2 - P0, P1 - P0));
    return normal;
}

float3 GetPerpendicularVector(in float3 vec)
{
    const float3 randomVec = float3(0, 0, 1);
    return cross(vec, randomVec);
}

float3 GetCosHemisphereSample(float rand1, float rand2, float3 hitNormal)
{
    const float2 randValue = float2(rand1, rand2);
    
    const float3 biTangent = GetPerpendicularVector(hitNormal);
    const float3 tangent = cross(biTangent, hitNormal);
    float r = sqrt(randValue.x);
    const float phi = 2.0f * 3.1415f * randValue.y;
    
    return tangent * (r * cos(phi).x) + biTangent * (r * sin(phi)) + hitNormal.xyz * sqrt(max(0.0f, 1.0f - randValue.x));
}


float4 main(VertextoPixel input) : SV_Target
{
    const float2 uv = input.uv;
    const int2 position = uv * resolution.xy;
    
    const float depth = u_DepthTexture.Sample(ClampSampler, uv, 0).r;
    if (depth < 1.f)
    {
        
        float depth = u_DepthTexture.SampleLevel(ClampSampler, uv, 0).r;
        float3 P = ReconstructPositionFromDepth(uv, depth, mul(inverse(toView), inverse(toProjection)));
        float3 normal = ReconstructNormalFromDepth(uv, mul(inverse(toView), inverse(toProjection)));

        float3 normalLength = length(normal);
    
        const float3 noise = u_NoiseTexture.SampleLevel(WrapSampler, (float2) position, 0).xyz;
    
        float3 stochasticNormal = GetCosHemisphereSample(noise.x, noise.y, normal);
    
        return (float4(normalize(stochasticNormal) * -1.f, 1.f));
    }
    return float4(0, 0, 0, 1);
}