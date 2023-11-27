#include <Includes/ConstStructs.hlsli>

struct VertexInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};

struct PixelOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

PixelOutput main( VertexInput vinput )
{
    PixelOutput pOut;
    const float4 newPos = vinput.Position;
    pOut.Position = newPos;
    pOut.Color = vinput.Color;
	return pOut;
}