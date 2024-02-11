#include "../../Shaders/includes/ConstStructs.hlsli"
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
struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float4 color0 : COLOR0;
    float4 color1 : COLOR1;
    float4 color2 : COLOR2;
    float4 color3 : COLOR3;
    float2 texcoord0 : UV0;
    float2 texcoord1 : UV1;
    float2 texcoord2 : UV2;
    float2 texcoord3 : UV3;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
    float entityTime : EntityTime;
};

struct InstanceData
{
    float4x4 transform;
    float4 entityTime;
};
struct InstanceBoneData
{
    float4x4 bone;
};

cbuffer MaterialInfo : register(b10)
{
    float4 DEMOCol_Color = float4(1,1,1,1); // With _Color after the name it will be a color variable in the editor.
    float DEMOSlider_Slider = 0.5f; // With _Slider it will have a slider from 0 -> 1 in the editor.
    float DEMO = 1.f; // Nothing special just a value.
}

StructuredBuffer<InstanceData> instanceData : register(t9);
StructuredBuffer<InstanceBoneData> instanceBoneData : register(t10);

PixelInput PrepVertexForPixelShader(VertexInput vInput, uint instanceID)
{
    PixelInput pInput;
    float4x4 mvp = mul(toProjection, mul(toView, instanceData[instanceID].transform));
    
    float4x4 skinningMatrix = 0;
    if (instanceData[instanceID].entityTime.z > 0)
    {
        skinningMatrix += mul(vInput.boneWeights.x, instanceBoneData[(instanceID * 128) + vInput.boneIds.x].bone);
        skinningMatrix += mul(vInput.boneWeights.y, instanceBoneData[(instanceID * 128) + vInput.boneIds.y].bone);
        skinningMatrix += mul(vInput.boneWeights.z, instanceBoneData[(instanceID * 128) + vInput.boneIds.z].bone);
        skinningMatrix += mul(vInput.boneWeights.w, instanceBoneData[(instanceID * 128) + vInput.boneIds.w].bone);
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
    
    const float4 vertexWorldPosition = mul(instanceData[instanceID].transform, mul(vInput.position, skinningMatrix));
    
    const float4 vertexViewPosition = mul(toView, vertexWorldPosition);
    
    const float4 vertexProjectionPosition = mul(toProjection, vertexViewPosition);
    
    pInput.position = vertexProjectionPosition;
    pInput.worldPosition = vertexWorldPosition;
    pInput.color0 = vInput.color0;
    pInput.color1 = vInput.color1;
    pInput.color2 = vInput.color2;
    pInput.color3 = vInput.color3;
    pInput.texcoord0 = vInput.texcoord0;
    pInput.texcoord1 = vInput.texcoord1;
    pInput.texcoord2 = vInput.texcoord2;
    pInput.texcoord3 = vInput.texcoord3;
    pInput.normal = vInput.normal;
    pInput.entityTime = instanceData[instanceID].entityTime.y - instanceData[instanceID].entityTime.x;
    
    float3 newTangent = mul((float3x3) instanceData[instanceID].transform, vInput.tangent);
    float3 newBiTangent = mul((float3x3) instanceData[instanceID].transform, vInput.biTangent);
    float3 newNormal = mul((float3x3) instanceData[instanceID].transform, vInput.normal);

    pInput.RotatedNormal = newNormal;
    float3x3 tbn = float3x3(normalize(newTangent), normalize(-newBiTangent), normalize(newNormal));
    pInput.tangentBias = tbn;
    return pInput;
}

PixelInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    PixelInput pInput = PrepVertexForPixelShader(vInput, instanceID);
    return pInput;
}