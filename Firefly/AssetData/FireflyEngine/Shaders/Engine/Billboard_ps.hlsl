

struct BillboardPixelIn
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : UV;
    uint2 EntityID : ENTITYID;
    uint TextureID : TextureID;
};
struct Output
{
    uint2 EntityID : SV_Target0;
    float4 WorldPosition : SV_Target1;
};
Output main(BillboardPixelIn input)
{
    Output result = (Output)0;
    result.EntityID = input.EntityID;
    result.WorldPosition = float4(0.f, 0.f, 0.f, 0.f);
	

	return result;
}