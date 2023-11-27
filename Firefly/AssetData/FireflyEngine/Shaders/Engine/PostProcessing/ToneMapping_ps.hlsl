#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/PBRFunctions.hlsli>
#include <Includes/PostProcessStructs.hlsli>

// taken from UE5 
float Square(float x)
{
    return x * x;
}

float2 Square(float2 x)
{
    return x * x;
}

float3 Square(float3 x)
{
    return x * x;
}

float4 Square(float4 x)
{
    return x * x;
}

float2 PlanckianIsothermal(float Temp, float Tint)
{
    float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) / (1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
    float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) / (1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);

    float ud = (-1.13758118e9f - 1.91615621e6f * Temp - 1.53177f * Temp * Temp) / Square(1.41213984e6f + 1189.62f * Temp + Temp * Temp);
    float vd = (1.97471536e9f - 705674.0f * Temp - 308.607f * Temp * Temp) / Square(6.19363586e6f - 179.456f * Temp + Temp * Temp);

    float2 uvd = normalize(float2(u, v));

	// Correlated color temperature is meaningful within +/- 0.05
    u += -uvd.y * Tint * 0.05;
    v += uvd.x * Tint * 0.05;
	
    float x = 3 * u / (2 * u - 8 * v + 4);
    float y = 2 * v / (2 * u - 8 * v + 4);

    return float2(x, y);
}

float4 ContrastCalc(float4 color, float4 contrast)
{
    const float midPoint = 0.5f;
    float4 constrasted = lerp(float4(midPoint, midPoint, midPoint, color.a), color, float4(contrast.xyz, 1.f));
    
    return saturate(constrasted);

}

float4 SaturationCalc(float4 color, float4 saturation)
{
    float3 LuminanceWeights = float3(0.299, 0.587, 0.114);
    float luminance = dot(color.xyz, LuminanceWeights);
    float4 dstPixel = lerp(color, float4(luminance, luminance, luminance, color.a), saturation);
    return saturate(dstPixel);
}


float3 Tonemap_ACES(float3 x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

Texture2D framebuffer : register(t0);
Texture2D ColorCorrectionLUT : register(t1);

static float myLUTHeight = 15.0f;
static float myLUTWidth = 255.0f;

float3 levels(float3 color, float3 blackin, float3 whitein, float gamma, float3 outblack, float3 outwhite)
{
    float3 ret = saturate(color.xyz - blackin.xyz) / max(whitein.xyz - blackin.xyz, 0.000001f);
    ret.xyz = pow(ret.xyz, gamma);
    ret.xyz = ret.xyz * saturate(outwhite.xyz - outblack.xyz) + outblack.xyz;
    return ret;
}

float3 GammaCorrection(float3 color, float3 gamma)
{
    return saturate(pow(color, float3(max(1.0 / gamma.x, 0.0001f), max(1.0 / gamma.y, 0.0001f), max(1.0 / gamma.z, 0.0001f))));
}

float4 main(VertextoPixel input) : SV_Target
{
    float3 color = framebuffer.Sample(ClampSampler, input.uv).rgb;
    float4 result;
    if (enableLUT > 0.f)
    {
        color = LinearToGamma(ACESTonemap(color) * 1.8);
        float redOffset = (color.r * myLUTHeight);
        redOffset = trunc(redOffset);

        float v = color.b * myLUTWidth;
        v = v - (v % (myLUTHeight + 1));
        
        float uValue = ((v + redOffset));

        float yValue = (color.g * myLUTHeight);
        
        
        float3 blueValue = LinearToGamma(ColorCorrectionLUT.Load(uint3(uValue, yValue, 0)).rgb);

        result.a = 1.f;
        result.r = 0;
        result.xyz = LinearToSRGB((blueValue) * color);
        result.a = 1.f;
    }
    else
    {
        float4 col4 = float4(color, 1);
        if (Enables.x > 0)
        {
            col4 = SaturationCalc(col4, Saturation * (-Intensities.x +1));
        }
        if (Enables.y > 0)
        {
            col4 = ContrastCalc(col4, Contrast * (Intensities.y));
        }
        if (Enables.z > 0)
        {
            result.xyz = GammaCorrection(ACESTonemap(col4.xyz) * 1.8, Gamma.xyz * Intensities.z);
        }
        else
        {
            result.xyz = LinearToGamma(ACESTonemap(col4.xyz) * 1.8);
        }
        if (Enables.w > 0)
        {
            result.xyz += saturate(Gain.xyz * Intensities.w);

        }
        result.a = 1.f;
    }
    
    return result;
}