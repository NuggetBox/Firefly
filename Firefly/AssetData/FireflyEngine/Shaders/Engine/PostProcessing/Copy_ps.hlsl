#include <Includes/FullscreenPassShader.hlsli>


Texture2D framebuffer : register(t0);
float4 main(VertextoPixel input) : SV_Target
{
    return framebuffer.Sample(ClampSampler, input.uv);
}