#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>

Texture2D outlineBuffer : register(t0);
Texture2D stencilBuffer : register(t1);

float4 main(VertextoPixel input) : SV_Target
{
    float outline = outlineBuffer.Sample(ClampSampler, input.uv).r;
    float stencil = stencilBuffer.Sample(ClampSampler, input.uv).r;
    return lerp(float4(outline, outline, outline, 1), float4(0, 0, 0, 1), stencil);
}