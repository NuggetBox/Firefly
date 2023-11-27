cbuffer PostProcessData : register(b7)
{
    float4 fogColor;
    float fogthreshold;
    float fogDensity;
    float windSpeed;
    float fogWaveFrekvency;
    float fogWaveHeight;
    float3 padding;
    float4 outlineColor;
    float enableLUT;
    float3 pad;
    float4 Saturation;
    float4 Contrast;
    float4 Gamma;
    float4 Gain;
    float4 Intensities;
    float4 Enables;
    float4 LockFog;
    float4 SSAOSettings;
}