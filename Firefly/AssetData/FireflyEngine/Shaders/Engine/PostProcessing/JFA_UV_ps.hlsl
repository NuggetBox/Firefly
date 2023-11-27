#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>

Texture2D colorBuffer : register(t0);

float4 main(VertextoPixel input) : SV_Target
{
    float2 srcTexelSize = 1.0 / resolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    
     // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float value[9];
    value[0] = colorBuffer.Sample(ClampSampler, float2(input.uv.x - 2 * x, input.uv.y + 2 * y)).r;
    value[1] = colorBuffer.Sample(ClampSampler, float2(input.uv.x, input.uv.y + 2 * y)).r;
    value[2] = colorBuffer.Sample(ClampSampler, float2(input.uv.x + 2 * x, input.uv.y + 2 * y)).r;

    value[3] = colorBuffer.Sample(ClampSampler, float2(input.uv.x - 2 * x, input.uv.y)).r;
    value[4] = colorBuffer.Sample(ClampSampler, float2(input.uv.x + 2 * x, input.uv.y)).r;
    value[8] = colorBuffer.Sample(ClampSampler, input.uv).r;
    value[5] = colorBuffer.Sample(ClampSampler, float2(input.uv.x - 2 * x, input.uv.y - 2 * y)).r;
    value[6] = colorBuffer.Sample(ClampSampler, float2(input.uv.x, input.uv.y - 2 * y)).r;
    value[7] = colorBuffer.Sample(ClampSampler, float2(input.uv.x + 2 * x, input.uv.y - 2 * y)).r;
    
    int blackCOunt = 0;
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        if (value[i] < 0.5f)
        {
            blackCOunt++;
        }
    }
    
    if(blackCOunt < 8 && blackCOunt > 1)
    {
        return float4(1, 1, 1, 1);
    }
    return float4(0, 0, 0, 1);
}