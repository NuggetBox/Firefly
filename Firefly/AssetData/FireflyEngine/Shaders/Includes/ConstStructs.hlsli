cbuffer CameraBuffer : register(b0)
{
    float4x4 toView;
    float4x4 toProjection;
    float4 cameraPosition;
    float nearPlane;
    float farPlane;
    float2 resolution;
    float4x4 oldViewProjection;
};

cbuffer ObjectBuffer : register(b1)
{
    uint matrixBufferOffset;
    uint boneStartOffset;
    uint materialDataOffset;
    uint ObjectBufferPadding;
};

cbuffer DirLightBuffer : register(b2)
{
    struct DirData
    {
        float4 colorAndIntensity;
        float4 direction;
        float4x4 viewMatrix;
        float4x4 projMatrix;
        uint4 dirLightInfo;
    } dirLight[6];
    int4 count;
};

cbuffer ParticleSystemBuffer : register(b3)
{
    float4x4 PS_ToWorld;
}

cbuffer PointLightBuffer : register(b4)
{
    struct PointData
    {
        float4 colorAndIntensity;
        float3 position;
        float radius;
       float4x4 transforms[6];
        float4 customValues;
    } pointLight[128];
    int4 pointCount;
}

cbuffer SpotLightBuffer : register(b5)
{
    struct SpotData
    {
        float4 colorAndIntensity;
        float4 position;
        float4 direction;
        float4 spotInfo;
        float4x4 transform;
    } spotData[64];
    int4 spotCount;
}

cbuffer RenderPassBuffer : register(b6)
{
    uint renderPassID;
    float EnvironmentIntensity;
    float2 renPadd;
    float4 EnvironmentFogColorIntensity;
    float4 godRaysColorIntensity;
    float4 FogSettings;
    float4 BloomThreshhold;
    int4 EnvironmentMip_padded3;
}

cbuffer TimeBuffer : register(b12)
{
    float entityLifeTime;
    float scaledTotalTime;
    float scaledDeltaTime;
    float unscaledDeltaTime;
}