#include <Includes/VertexCommon.hlsli>

struct GSInput
{
    float4 Position : POSITION;
};

GSInput main(VertexInput vInput, uint instanceID : SV_InstanceID)
{
    //float4x4 mvp = mul(toProjection, mul(toView, instanceData[instanceID].transform));
    
    InstanceData instanceData = FetchInstanceData(instanceID);
    
    float4x4 skinningMatrix = CreateSkinningMatrix(vInput.boneWeights, vInput.boneIds, instanceData, instanceID);
    
    const float4 vertexWorldPosition = TransformPosition(vInput.position, skinningMatrix, instanceData);
    
    
    GSInput output = (GSInput)0;
    
    output.Position = vertexWorldPosition;
    return output;
}