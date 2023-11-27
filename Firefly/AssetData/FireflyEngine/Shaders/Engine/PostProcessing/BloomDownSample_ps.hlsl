// HLSL version of glsl physical based bloom: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>

float3 PowVec3(float3 v, float p)
{
    return float3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

static const float invGamma = 1.0 / 2.2;
float3 ToSRGB(float3 v)
{
    return PowVec3(v, invGamma);
}

float RGBToLuminance(float3 col)
{
    return dot(col, float3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(float3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

Texture2D srcTexture : register(t0);
float4 main(VertextoPixel input) : SV_Target
{
    float2 srcTexelSize = 1.0 / resolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    float3 downsample = float3(0, 0, 0);
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-2, 2)).rgb;
    float3 b = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(0, 2)).rgb;
    float3 c = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(2, 2)).rgb;

    float3 d = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-2, 0)).rgb;
    float3 e = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(0, 0)).rgb;
    float3 f = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(2, 0)).rgb;

    float3 g = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-2, -2)).rgb;
    float3 h = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(0, -2)).rgb;
    float3 i = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(2, -2)).rgb;

    float3 j = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x - x, input.uv.y + y), 0, int2(-1, 1)).rgb;
    float3 k = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x + x, input.uv.y + y), 0, int2(1, 1)).rgb;
    float3 l = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x - x, input.uv.y - y), 0, int2(-1, -1)).rgb;
    float3 m = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x + x, input.uv.y - y), 0, int2(1, -1)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    
    
    downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;
    return float4(downsample, 1.f);
}