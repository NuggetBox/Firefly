#include <Includes/ConstStructs.hlsli>

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 Uv : UV;
    uint TexIndex : TEXINDEX;
    uint is3D : IS3D;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : UV;
    uint TexIndex : TEXINDEX;
};

VertexOutput main(VertexInput vinput)
{
    VertexOutput pOut;
    if(vinput.is3D > 0)
    {
        pOut.Position = mul(toProjection, mul(toView, float4(vinput.Position, 1)));
    }
    else
    {
        pOut.Position = float4(vinput.Position, 1);
    }
    pOut.Color = vinput.Color;
    pOut.TexCoord = vinput.Uv;
    pOut.TexIndex = vinput.TexIndex;
    return pOut;
}