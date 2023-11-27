#include <Includes/FullscreenPassShader.hlsli>


Texture2D framebuffer : register(t0);
Texture2D bloombuffer : register(t1);

float RGBToLuminance(float3 col)
{
    float lumanance = dot(col, float3(0.2126f, 0.7152f, 0.0722f));
    return lumanance;
}
cbuffer PostProcessData : register(b7)
{
    float4 fogColor;
    float fogthreshold;
    float fogDensity;
    float windSpeed;
    float fogWaveFrekvency;
    float fogWaveHeight;
    float3 padding;
    float4 vignetteColor;
    float enableLUT;
    float3 pad;
}
float4 main(VertextoPixel input) : SV_Target
{
    const float3 color = framebuffer.Sample(ClampSampler, input.uv).rgb;
    const float3 bloom = bloombuffer.Sample(ClampSampler, input.uv).rgb;
  
    const float luminace = RGBToLuminance(color);
    
    const float3 scaledResource = bloom * (padding.z);
    
    return float4(color + scaledResource, 1.f);
}