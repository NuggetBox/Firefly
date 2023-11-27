#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/PostProcessStructs.hlsli>


Texture2D framebuffer : register(t0);
Texture2D outlinebuffer : register(t1);
float4 main(VertextoPixel input) : SV_Target
{
    float4 color = framebuffer.Sample(ClampSampler, input.uv);
    float outline = outlinebuffer.Sample(ClampSampler, input.uv).r;
    return lerp(color, float4(1, 0.5f,0.f, 1.f), outline);
}