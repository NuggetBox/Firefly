#include <Includes/ConstStructs.hlsli>

struct PixelShaderInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

PixelShaderInput main(float4 pos : POSITION)
{
    PixelShaderInput output;
    output.localPosition = pos;

    float4x4 rotView = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.5f, 0.5f, 0.5f, 1.0f);
    
    float4x4 viewProjection = mul(toProjection, toView);
    
    float3x3 view = (float3x3) toView;
    rotView[0].xyz = view[0];
    rotView[1].xyz = view[1];
    rotView[2].xyz = view[2];


    output.pixelPosition = mul(toProjection, mul(rotView, pos));
    return output;
}