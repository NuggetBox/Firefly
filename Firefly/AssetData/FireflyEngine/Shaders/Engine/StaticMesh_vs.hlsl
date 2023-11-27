#include <Includes/VertexCommon.hlsli>


struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float2 texcoord0 : UV0;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
    uint PrimtiveID : ID;
};

PixelInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    PixelInput pInput;
    
    InstanceData instanceData = FetchInstanceData(instanceID);
    
    float4x4 skinningMatrix = CreateSkinningMatrix(vInput.boneWeights, vInput.boneIds, instanceData, instanceID);
    
    pInput.position = ProjectPosition(vInput.position, skinningMatrix, instanceData);
    pInput.worldPosition = TransformPosition(vInput.position, skinningMatrix, instanceData);
    pInput.texcoord0 = vInput.texcoord0;
    pInput.normal = vInput.normal;
    
    const float3 newTangent = TransformVector(vInput.tangent, skinningMatrix, instanceData);
    const float3 newBiTangent = TransformVector(vInput.biTangent, skinningMatrix, instanceData);
    const float3 newNormal = TransformVector(vInput.normal, skinningMatrix, instanceData);

    pInput.RotatedNormal = newNormal;
    pInput.tangentBias = CreateTBN(newNormal, newBiTangent, newTangent);
    pInput.PrimtiveID = 0;
    
    return pInput;
}