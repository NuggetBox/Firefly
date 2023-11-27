#include <Includes/ConstStructs.hlsli>

struct BillboardGeometryIn
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    uint2 EntityID : ENTITYID;
    uint TextureID : TEXID;
};

struct BillboardPixelIn
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : UV;
    uint2 EntityID : ENTITYID;
    uint TextureID : TextureID;
};

[maxvertexcount(4)]
void main(point BillboardGeometryIn input[1], inout TriangleStream<BillboardPixelIn> output)
{
	const float2 offsets[4] =
	{
		{-1.0f, 1.0f},
		{1.0f, 1.0f},
		{-1.0f, -1.0f},
		{1.0f, -1.0f}
	};

	const float2 uvs[4] =
	{
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 1.0f}
	};

    const BillboardGeometryIn inputBillboard = input[0];

	for (unsigned int i = 0; i < 4; ++i)
	{
        BillboardPixelIn result;

        result.Position = mul(toView, inputBillboard.Position);
        result.Position.xy += offsets[i] * 10.f;
		result.Position = mul(toProjection, result.Position);
        result.TextureID = inputBillboard.TextureID;
		result.UV = uvs[i];
        result.Color = inputBillboard.Color;
		result.EntityID = inputBillboard.EntityID;

		output.Append(result);
	}
}