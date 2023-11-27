// HLSL version of glsl physical based bloom: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>

Texture2D srcTexture : register(t0);
float4 main(VertextoPixel input) : SV_Target
{
    float3 upsample;
     // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = 0.03;
    float y = 0.03;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-1, 1)).rgb;
    float3 b = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 0, 1)).rgb;
    float3 c = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 1, 1)).rgb;

    float3 d = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-1, 0)).rgb;
    float3 e = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 0, 0)).rgb;
    float3 f = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 1, 0)).rgb;

    float3 g = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2(-1,-1)).rgb;
    float3 h = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 0,-1)).rgb;
    float3 i = srcTexture.SampleLevel(ClampSampler, float2(input.uv.x, input.uv.y), 0, int2( 1,-1)).rgb;

    
    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;
    return float4(upsample, 1.0f);
}