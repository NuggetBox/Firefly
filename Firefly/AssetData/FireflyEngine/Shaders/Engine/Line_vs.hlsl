#include <Includes/ConstStructs.hlsli>

struct VertexInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    unsigned int instanceID : SV_InstanceID;
};

struct PixelOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

PixelOutput main( VertexInput vinput )
{
    PixelOutput pOut;
    float4 newPos = vinput.Position;
    pOut.Position = mul(toProjection, mul(toView, newPos));
    pOut.Color = vinput.Color;
	return pOut;
}