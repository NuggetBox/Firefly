#include <Includes/ConstStructs.hlsli>

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    uint2 entityID : EntityID;
};

struct Output
{
    uint2 EntityID : SV_Target0;
    float4 WorldPosition : SV_Target1;
};


Output main(PixelInput input)
{
    Output output = (Output)0;
    output.EntityID = input.entityID;
    output.WorldPosition = input.worldPosition.xyzw;
    return output;
}