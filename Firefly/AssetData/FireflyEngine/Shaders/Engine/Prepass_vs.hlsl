#include <Includes/VertexCommon.hlsli>


struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    uint2 entityID : EntityID;
};

PixelInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    PixelInput pInput;
    
    InstanceData instanceData = FetchInstanceData(instanceID);

    float4x4 skinningMatrix = CreateSkinningMatrix(vInput.boneWeights, vInput.boneIds, instanceData, instanceID);
    
    const float4 vertexWorldPosition = TransformPosition(vInput.position, skinningMatrix, instanceData);
    
    pInput.position = ProjectPosition(vInput.position, skinningMatrix, instanceData);
    pInput.worldPosition = vertexWorldPosition;
    pInput.entityID.y = instanceData.LowerEntityID_HigherEntityID.y; // black magic, just trust
    pInput.entityID.x = instanceData.LowerEntityID_HigherEntityID.x;
    return pInput;
}