#include <Includes/VertexCommon.hlsli>


struct PixelInput
{
    float4 position : SV_POSITION;
};

PixelInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    PixelInput pInput;
    
    InstanceData instanceData = FetchInstanceData(instanceID);
    
    float4x4 skinningMatrix = CreateSkinningMatrix(vInput.boneWeights, vInput.boneIds, instanceData, instanceID);
    
    pInput.position = ProjectPosition(vInput.position, skinningMatrix, instanceData);
    
    return pInput;
}