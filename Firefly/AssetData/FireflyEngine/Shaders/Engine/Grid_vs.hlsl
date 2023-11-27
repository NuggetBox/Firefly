#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>

struct Input
{
    unsigned int index : SV_VertexID;
};

struct Output
{
    float4 position : SV_POSITION;
    float3 near : NEARPOINT;
    float3 far : FARPOINT;
    float4x4 fragView : FRAGVIEW;
    float4x4 fragProj : FRAGPROJ;
};


float3 UnprojectPoint(float x, float y, float z, float4x4 view, float4x4 projection)
{
    float4x4 viewInv = inverse(view);
    float4x4 projInv = inverse(projection);
    float4 unprojectedPoint = mul(viewInv, mul(projInv, float4(x, y, z, 1.0)));
    return unprojectedPoint.xyz / unprojectedPoint.w;
}
Output main(Input input)
{
    Output output;
    float4 gridPlane[6] =
    {
        float4(1, 1, 0, 1), float4(-1, -1, 0, 1), float4(-1, 1, 0, 1),
    float4(-1, -1, 0, 1), float4(1, 1, 0, 1), float4(1, -1, 0, 1)
    };
    output.position = gridPlane[input.index];
    output.near = UnprojectPoint(output.position.x, output.position.y, 0, toView, toProjection);
    output.far = UnprojectPoint(output.position.x, output.position.y, 1.0, toView, toProjection);
    output.fragView = toView;
    output.fragProj = toProjection;
    return output;
}