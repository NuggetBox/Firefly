struct DeferredVertexInput
{
    unsigned int index : SV_VertexID;
};

struct DeferredVertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

DeferredVertextoPixel main(DeferredVertexInput input)
{
    float4 pos[3] =
    {
        float4(-1.0f, -1.0f, 0.0f, 1.0f),
		float4(-1.0f, 3.0f, 0.0f, 1.0f),
		float4(3.0f, -1.0f, 0.0f, 1.0f),
    };

    float2 uvValue[3] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, -1.0f),
		float2(2.0f, 1.0f)
    };

    DeferredVertextoPixel returnValue;
    returnValue.position = pos[input.index];
    returnValue.uv = uvValue[input.index];
    return returnValue;
}