#include <Includes/FireflyShader.hlsli>

struct PixelInputF
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float2 texcoord0 : UV0;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
    uint PrimtiveID : ID;
};

Texture2D tex : register(t0);
Texture2D norm : register(t3);
Texture2D materialInfo : register(t10);

cbuffer MaterialInfo : register(b10)
{
    float4 AlbedoColor_Color = float4(1, 1, 1, 1);
    float ColorStrength = 1.f;
    float RoughnessMultiplier_Slider = 1.f;
    float RoughnessIntensity_Slider = 1.f;
    float NormalMapIntensity_Slider = 1.0f;
    float Emissive_Slider = 1.0f;
    float Metalness = 0.f;
    float2 padd;
    
}


struct Output
{
    float4 colorResult : SV_Target0;
};

Output main(PixelInputF pInput)
{
    float alpha = tex.Sample(WrapSampler, pInput.texcoord0.xy).a * AlbedoColor_Color.a;
    float3 color = tex.Sample(WrapSampler, pInput.texcoord0.xy).xyz * AlbedoColor_Color.xyz * ColorStrength;
    float3 nomal = norm.Sample(WrapSampler, pInput.texcoord0.xy).agb;
    float4 matInfo = materialInfo.Sample(WrapSampler, pInput.texcoord0.xy);
    if (alpha == 0)
    {
        discard;
        Output outer = (Output)0;
        outer.colorResult = float4(0, 0, 0, 0);
        return outer;
    }
    
    matInfo.g = pow(abs(matInfo.g), RoughnessIntensity_Slider);
    matInfo.g *= RoughnessMultiplier_Slider;
    
    matInfo.r += Metalness;
    
    const float AO = nomal.b;
    const float metalness = matInfo.r;
    const float roughness = matInfo.g;
    const float emissive = matInfo.b;
    const float emissiveStr = matInfo.a;
    nomal.z = 0;
    nomal = 2.0f * nomal - 1.0f;
    nomal.z = sqrt(1 - saturate(nomal.x * nomal.x + nomal.y * nomal.y));

    nomal = normalize(nomal);
    
    
    const float3 normalConstant = float3(0, 0, 1);
    float3 newNormalMap = lerp(normalConstant, nomal, NormalMapIntensity_Slider);
    
    float3 PixelNomal = normalize(mul(newNormalMap, pInput.tangentBias));

    float3 final = CalculatePBR(color, PixelNomal, roughness, metalness, pInput.worldPosition, pInput.normal, AO, ((emissive * emissiveStr) * 100) * Emissive_Slider);
    
    Output outer = (Output) 0;

    outer.colorResult = float4(0, 0, 0, 0);
    switch (renderPassID)
    {
        case 0:
            outer.colorResult = float4(final, alpha);
            break;
        case 1:
            outer.colorResult = float4(color, alpha);
            break;
        case 2:
            outer.colorResult = float4(nomal.xyz, 1);
            break;
        case 3:
            outer.colorResult = float4(PixelNomal.xyz, 1);
            break;
        case 4:
            outer.colorResult = float4(AO, AO, AO, 1);
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