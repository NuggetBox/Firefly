#include <Includes/FullscreenPassShader.hlsli>


Texture2D depth : register(t0);
float main(VertextoPixel input) : SV_Depth
{
    return depth.Sample(ClampSampler, input.uv).x;
}