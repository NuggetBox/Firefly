#include <Includes/ConstStructs.hlsli>

struct DrawData
{
    float4 color : SV_TARGET;
    float depth : SV_Depth;
};
struct Output
{
    float4 position : SV_POSITION;
    float3 near : NEARPOINT;
    float3 far : FARPOINT;
    float4x4 fragView : FRAGVIEW;
    float4x4 fragProj : FRAGPROJ;
};

float4 grid(float3 fragPos3D, float scale)
{
    float2 coord = fragPos3D.xz * scale; // use the scale variable to set the distance between the lines
    float2 derivative = fwidth(coord);
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float liner = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    float4 color = float4(0.2, 0.2, 0.2, 1.0 - min(liner, 1.0));
    // z axis
    if (fragPos3D.x > -100 * minimumx && fragPos3D.x < 100 * minimumx)
        color.z = 1.0;
    // x axis
    if (fragPos3D.z > -100 * minimumz && fragPos3D.z < 100 * minimumz)
        color.x = 1.0;
    return color;
}
float computeDepth(float3 pos, float4x4 fragProj, float4x4 FragView)
{
    float4 clip_space_pos = mul(fragProj, mul(FragView, float4(pos.xyz, 1.0)));
    return (clip_space_pos.z / clip_space_pos.w);
}

float computeLinearDepth(float3 pos, float4x4 fragProj, float4x4 fragView)
{
    float far = 10000.f;
    float near = 0.1f;
    float4 clip_space_pos = mul(fragProj, mul(fragView, float4(pos.xyz, 1.0)));
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linearDepth / far;
}

DrawData main(Output input)
{
    DrawData drawData;
    float t = -input.near.y / (input.far.y - input.near.y);
    float3 fragPos3D = input.near + t * (input.far - input.near);
    float linearDepth = computeLinearDepth(fragPos3D, input.fragProj, input.fragView);
    float fading = max(0, (0.5 - linearDepth));
    drawData.color = grid(fragPos3D, 0.01) * float(t > 0);
    drawData.color.a *= fading;
    drawData.depth = computeDepth(fragPos3D, input.fragProj, input.fragView);
    return drawData;
}