#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>

struct InstanceData
{
    float4x4 transform;
    float2 entityTime_IsAnimation;
    uint2 LowerEntityID_HigherEntityID;
};
struct InstanceBoneData
{
    float4x4 bone;
};

StructuredBuffer<InstanceData> instanceData : register(t70);
StructuredBuffer<InstanceBoneData> instanceBoneData : register(t71);

InstanceData FetchInstanceData(uint offset)
{
    return instanceData[matrixBufferOffset + offset];
}

float4x4 FetchBone(uint offset)
{
    return instanceBoneData[offset].bone;
}


struct VertexInput
{
    float4 position : POSITION;
    float4 color0 : COLOR0;
    float4 color1 : COLOR1;
    float4 color2 : COLOR2;
    float4 color3 : COLOR3;
  
    float2 texcoord0 : UV0;
    float2 texcoord1 : UV1;
    float2 texcoord2 : UV2;
    float2 texcoord3 : UV3;
    
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;

    uint4 boneIds : BONEIDS;
    float4 boneWeights : BONEWEIGHTS;
};

float4 ProjectPosition(float4 worldPosition, float4x4 skinningMatrix, InstanceData instanceData)
{
    const float4 vertexWorldPosition = mul(instanceData.transform, mul(worldPosition, skinningMatrix));
    const float4 vertexViewPosition = mul(toView, vertexWorldPosition);
    
    const float4 vertexProjectionPosition = mul(toProjection, vertexViewPosition);
    return vertexProjectionPosition;
}

float4 TransformPosition(float4 worldPosition, float4x4 skinningMatrix, InstanceData instanceData)
{
    return mul(instanceData.transform, mul(worldPosition, skinningMatrix));
}

float3 TransformVector(float3 normalizedVector, float4x4 skinningMatrix, InstanceData instanceData)
{
    return mul((float3x3) instanceData.transform, mul(normalizedVector, (float3x3) skinningMatrix));
}

float3 InverseTransformVector(in float3 normalizedVector, InstanceData instancedata)
{
    return mul((float3x3) inverse(instancedata.transform), normalizedVector);

}

float FetchEntityTime(InstanceData instanceData)
{
    return instanceData.entityTime_IsAnimation.x;
}

float3x3 CreateTBN(float3 normal, float3 bitangent, float3 tangent)
{
    return float3x3(normalize(tangent), normalize(-bitangent), normalize(normal));
}

float4x4 CreateSkinningMatrix(float4 boneweights, uint4 boneIds, InstanceData instanceData, uint offset)
{
    float4x4 skinningMatrix = 0;
    
    const uint instanceID = boneStartOffset + offset * 128;
    
    if (instanceData.entityTime_IsAnimation.y > 0)
    {
        skinningMatrix += mul(boneweights.x, instanceBoneData[(instanceID ) + boneIds.x].bone);
        skinningMatrix += mul(boneweights.y, instanceBoneData[(instanceID) + boneIds.y].bone);
        skinningMatrix += mul(boneweights.z, instanceBoneData[(instanceID) + boneIds.z].bone);
        skinningMatrix += mul(boneweights.w, instanceBoneData[(instanceID) + boneIds.w].bone);
    }
    else
    {
         skinningMatrix = float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    return skinningMatrix;
}