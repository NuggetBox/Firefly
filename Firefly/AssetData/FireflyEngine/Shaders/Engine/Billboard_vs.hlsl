#include <Includes/ConstStructs.hlsli>

struct BillboardVertexIn
{
    float4 Position : POSITION;
    uint TextureID : TEXID;
    uint2 EntityID : ENTITYID;
    float4 Color : COLOR;
};

struct BillboardGeometryIn
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    uint2 EntityID : ENTITYID;
    uint TextureID : TEXID;
};

BillboardGeometryIn main(BillboardVertexIn input)
{
    BillboardGeometryIn result;


	result.Position = input.Position;
	result.TextureID = input.TextureID;
	result.EntityID = input.EntityID;
    result.Color = input.Color;

	return result;
}