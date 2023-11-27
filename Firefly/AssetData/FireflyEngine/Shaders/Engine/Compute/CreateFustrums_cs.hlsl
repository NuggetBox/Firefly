#include <Includes/IntersectionCommon.hlsli>
#include <Includes/ConstStructs.hlsli>

cbuffer DispatchParams : register(b11)
{
    uint3 numThreadGroups;
    uint padding;

    // Total number of threads dispatched. (Also not available as an HLSL system value!)
    // Note: This value may be less than the actual number of threads executed 
    // if the screen size is not evenly divisible by the block size.
    uint3 numThreads;
    uint pad; 
}

#ifndef BLOCK_SIZE
#pragma message( "BLOCK_SIZE undefined. Default to 16.")
#define BLOCK_SIZE 16 // should be defined by the application.
#endif

struct ComputeShaderInput
{
    uint3 groupID : SV_GroupID; // 3D index of the thread group in the dispatch.
    uint3 groupThreadID : SV_GroupThreadID; // 3D index of local thread ID in a thread group.
    uint3 dispatchThreadID : SV_DispatchThreadID; // 3D index of global thread ID in the dispatch.§
    uint groupIndex : SV_GroupIndex; // Flattened local index of the thread within a thread group.
};

RWStructuredBuffer<Frustum> outFrustums : register(u0);

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void ComputeFrustrums(ComputeShaderInput input)
{
    const float3 eyePosition = float3(0, 0, 0);
    
    float4 screenSpace[4];
    
    
    // Top left
    screenSpace[0] = float4(input.dispatchThreadID.xy * BLOCK_SIZE, -1.0f, 1.0f);
    // Top right
    screenSpace[1] = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y) * BLOCK_SIZE, -1.0f, 1.0f);
    // Bottom left
    screenSpace[2] = float4(float2(input.dispatchThreadID.x, input.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);
    // Bottom right
    screenSpace[3] = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);

    float3 viewSpace[4];
    
    for (int i = 0; i < 4; ++i)
    {
        viewSpace[i] = mul(toView, screenSpace[i]).xyz;
    }
    
    Frustum frustum;
    frustum.planes[0] = ComputePlane(eyePosition, viewSpace[2], viewSpace[0]);
    frustum.planes[1] = ComputePlane(eyePosition, viewSpace[1], viewSpace[3]);
    frustum.planes[0] = ComputePlane(eyePosition, viewSpace[0], viewSpace[1]);
    frustum.planes[0] = ComputePlane(eyePosition, viewSpace[3], viewSpace[2]);
    
    if(input.dispatchThreadID.x < numThreads.x && input.dispatchThreadID.y < numThreads.y)
    {
        uint index = input.dispatchThreadID.x + (input.dispatchThreadID.y * numThreads.x);
        outFrustums[index] = frustum;
    }
}