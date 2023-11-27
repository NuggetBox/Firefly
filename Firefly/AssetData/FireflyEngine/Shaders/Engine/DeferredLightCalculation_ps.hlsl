#include <Includes/FireflyShader.hlsli>


struct DeferredVertextoPixel
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct Output
{
    float4 colorResult : SV_Target0;
};

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);
Texture2D VertexNormalTexture : register(t3);
Texture2D WorldPositionTexture : register(t4);
Texture2D AOTexture : register(t5);
Texture2D occlusionTexture : register(t6);


Output main(DeferredVertextoPixel input)
{
    float4 albedo = AlbedoTexture.Sample(ClampSampler, input.uv);
    //if (albedo.a == 0)
    //{
    //    discard;
    //    Output outer;
    //    outer.colorResult = float4(0, 0, 0, 0);
    //    return outer;
    //}

    float3 normal = NormalTexture.Sample(ClampSampler, input.uv).rgb;
    float4 material = MaterialTexture.Sample(ClampSampler, input.uv);
    float3 vertexNormal = VertexNormalTexture.Sample(ClampSampler, input.uv).rgb;
    float4 worldPosition = WorldPositionTexture.Sample(ClampSampler, input.uv);
    float hbaoResult = occlusionTexture.Sample(ClampSampler, input.uv).x;
    float textureAO = AOTexture.Sample(ClampSampler, input.uv).x;
    const float metalness = material.r;
    const float roughness = material.g;
    const float emissive = material.b;
    const float emissiveStr = material.a;
    
    float finalAO = min(hbaoResult, textureAO);
    
    Output outer = (Output) 0;
    float3 final = CalculatePBR(albedo.rgb, normal, roughness, metalness, worldPosition, vertexNormal, finalAO, (emissive * emissiveStr) * 100);
    //final = worldPosition.xyz - pointLight[].position;
    outer.colorResult = float4(0, 0, 0, 0);
    switch (renderPassID)
    {
        case 0:
            outer.colorResult = float4(final, 1);
            break;
        case 1:
            outer.colorResult = float4(albedo.xyz, 1);
            break;
        case 2:
            outer.colorResult = float4(vertexNormal.xyz, 1);
            break;
        case 3:
            outer.colorResult = float4(normal.xyz, 1);
            break;
        case 4:
            outer.colorResult = float4(finalAO.xxx, 1);
            break;
        case 5:
            outer.colorResult = float4(roughness, roughness, roughness, 1);
            break;
        case 6:
            outer.colorResult = float4(metalness, metalness, metalness, 1);
            break;
    }
    return outer;
}