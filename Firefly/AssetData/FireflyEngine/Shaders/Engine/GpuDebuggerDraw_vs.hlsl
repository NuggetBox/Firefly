#include "../Includes/ConstStructs.hlsli"

struct VertexOutput
{
    float4 u_Position : SV_Position;
    float4 u_Color : Color;
};

struct VertexInput
{
    uint i_VertexId : SV_VertexID;
    uint i_InstanceId : SV_InstanceID;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    output.u_Color = 
}