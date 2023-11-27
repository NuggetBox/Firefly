#include <Includes/VertexCommon.hlsli>

struct GSInput
{
    float4 Position : SV_POSITION;
};

GSInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    //float4x4 mvp = mul(toProjection, mul(toView, instanceData[instanceID].transform));
    
    InstanceData instanceData = FetchInstanceData(instanceID);
    
    float4x4 skinningMatrix = CreateSkinningMatrix(vInput.boneWeights, vInput.boneIds, instanceData, instanceID);
    
    const float4 vertexWorldPosition = mul(instanceData.transform, mul(vInput.position, skinningMatrix));
    
    const float4 vertexProjectionPosition = mul(toView, vertexWorldPosition);
    
    
    GSInput output = (GSInput)0;
    
    output.Position = vertexProjectionPosition;
    return output;
}