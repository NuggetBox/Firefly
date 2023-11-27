
sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);

struct VertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};