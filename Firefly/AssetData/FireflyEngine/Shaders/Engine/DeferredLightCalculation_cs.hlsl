#include <Includes/FireflyShader.hlsli>


#ifndef BLOCK_SIZE
#define BLOCK_SIZE 8 // should be defined by the application.
#endif

uint2 ThreadGroupTilingX(
	const uint2 dipatchGridDim, // Arguments of the Dispatch call (typically from a ConstantBuffer)
	const uint2 ctaDim, // Already known in HLSL, eg:[numthreads(8, 8, 1)] -> uint2(8, 8)
	const uint maxTileWidth, // User parameter (N). Recommended values: 8, 16 or 32.
	const uint2 groupThreadID, // SV_GroupThreadID
	const uint2 groupId // SV_GroupID
)
{
	// A perfect tile is one with dimensions = [maxTileWidth, dipatchGridDim.y]
    const uint Number_of_CTAs_in_a_perfect_tile = maxTileWidth * dipatchGridDim.y;

	// Possible number of perfect tiles
    const uint Number_of_perfect_tiles = dipatchGridDim.x / maxTileWidth;

	// Total number of CTAs present in the perfect tiles
    const uint Total_CTAs_in_all_perfect_tiles = Number_of_perfect_tiles * maxTileWidth * dipatchGridDim.y;
    const uint vThreadGroupIDFlattened = dipatchGridDim.x * groupId.y + groupId.x;

	// Tile_ID_of_current_CTA : current CTA to TILE-ID mapping.
    const uint Tile_ID_of_current_CTA = vThreadGroupIDFlattened / Number_of_CTAs_in_a_perfect_tile;
    const uint Local_CTA_ID_within_current_tile = vThreadGroupIDFlattened % Number_of_CTAs_in_a_perfect_tile;
    uint Local_CTA_ID_y_within_current_tile;
    uint Local_CTA_ID_x_within_current_tile;

    if (Total_CTAs_in_all_perfect_tiles <= vThreadGroupIDFlattened)
    {
		// Path taken only if the last tile has imperfect dimensions and CTAs from the last tile are launched. 
        uint X_dimension_of_last_tile = dipatchGridDim.x % maxTileWidth;
#ifdef DXC_STATIC_DISPATCH_GRID_DIM
		X_dimension_of_last_tile = max(1, X_dimension_of_last_tile);
#endif
        Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / X_dimension_of_last_tile;
        Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % X_dimension_of_last_tile;
    }
    else
    {
        Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / maxTileWidth;
        Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % maxTileWidth;
    }

    const uint Swizzled_vThreadGroupIDFlattened =
		Tile_ID_of_current_CTA * maxTileWidth +
		Local_CTA_ID_y_within_current_tile * dipatchGridDim.x +
		Local_CTA_ID_x_within_current_tile;

    uint2 SwizzledvThreadGroupID;
    SwizzledvThreadGroupID.y = Swizzled_vThreadGroupIDFlattened / dipatchGridDim.x;
    SwizzledvThreadGroupID.x = Swizzled_vThreadGroupIDFlattened % dipatchGridDim.x;

    uint2 SwizzledvThreadID;
    SwizzledvThreadID.x = ctaDim.x * SwizzledvThreadGroupID.x + groupThreadID.x;
    SwizzledvThreadID.y = ctaDim.y * SwizzledvThreadGroupID.y + groupThreadID.y;

    return SwizzledvThreadID.xy;
}


struct ComputeShaderInput
{
    uint3 groupID : SV_GroupID; // 3D index of the thread group in the dispatch.
    uint3 groupThreadID : SV_GroupThreadID; // 3D index of local thread ID in a thread group.
    uint3 dispatchThreadID : SV_DispatchThreadID; // 3D index of global thread ID in the dispatch.
    uint groupIndex : SV_GroupIndex; // Flattened local index of the thread within a thread group.
};

RWTexture2D<float4> u_ResultTexture : register(u0);

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);
Texture2D VertexNormalTexture : register(t3);
Texture2D WorldPositionTexture : register(t4);
Texture2D AOTexture : register(t5);
Texture2D occlusionTexture : register(t6);
Texture2D forwardTexture : register(t7);

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput inputs)
{
    uint width = 0;
    uint height = 0;
    u_ResultTexture.GetDimensions(width, height);
    uint2 newThreadGroup = ThreadGroupTilingX(uint2(width / BLOCK_SIZE, height / BLOCK_SIZE) + uint2(1,1), uint2(BLOCK_SIZE, BLOCK_SIZE), 8, inputs.groupThreadID.xy, inputs.groupID.xy);
    float2 pixPos = float2(newThreadGroup) + 0.5f;
    float2 uv = pixPos / float2(width, height);
    float4 albedo = AlbedoTexture.SampleLevel(ClampSampler, uv, 0);
    float4 sceneColor = forwardTexture.SampleLevel(ClampSampler, uv, 0);

    if (albedo.a < 0.95f)
    {
        u_ResultTexture[pixPos] = float4(sceneColor);
        return;
    }

    
    float3 normal = NormalTexture.SampleLevel(ClampSampler, uv, 0).rgb;
    float4 material = MaterialTexture.SampleLevel(ClampSampler, uv, 0);
    float3 vertexNormal = VertexNormalTexture.SampleLevel(ClampSampler, uv, 0).rgb;
    float4 worldPosition = WorldPositionTexture.SampleLevel(ClampSampler, uv, 0);
    float hbaoResult = occlusionTexture.SampleLevel(ClampSampler, uv, 0).x;
    float textureAO = AOTexture.SampleLevel(ClampSampler, uv, 0).x;
    const float metalness = material.r;
    const float roughness = material.g;
    const float emissive = material.b;
    const float emissiveStr = material.a;
    
    float finalAO = min(hbaoResult, textureAO);
 
    {
        float3 final = CalculatePBR(albedo.rgb, normal, roughness, metalness, worldPosition, vertexNormal, finalAO, (emissive * emissiveStr) * 100);
        switch (renderPassID)
        {
            case 0:
                u_ResultTexture[pixPos] = float4(final, 1);
                break;
            case 1:
                u_ResultTexture[pixPos] = float4(albedo.xyz, 1);
                break;
            case 2:
                u_ResultTexture[pixPos] = float4(vertexNormal.xyz, 1);
                break;
            case 3:
                u_ResultTexture[pixPos] = float4(normal.xyz, 1);
                break;
            case 4:
                u_ResultTexture[pixPos] = float4(finalAO.xxx, 1);
                break;
            case 5:
                u_ResultTexture[pixPos] = float4(roughness, roughness, roughness, 1);
                break;
            case 6:
                u_ResultTexture[pixPos] = float4(metalness, metalness, metalness, 1);
                break;
        }
        
    }
}