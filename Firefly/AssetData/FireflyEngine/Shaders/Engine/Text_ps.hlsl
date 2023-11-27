#include <Includes/ConstStructs.hlsli>

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION;
    float4 Color : COLOR;
    float2 TexCoord : UV;
    uint TexIndex : TEXINDEX;
    uint is3D : IS3D;
};

sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);
Texture2D textures[32] : register(t0);

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 uv, float2 textureSize, float4 pxPos)
{
    float pixelRange = distance(cameraPosition, pxPos);
    float2 unitRange = float2(pixelRange, pixelRange) / textureSize;
    float2 screenTexSize = float2(1.0f, 1.0f) / fwidth(uv);
    return max(0.5 * dot(screenTexSize, unitRange), 1.0);
}

float4 main(VertexOutput Input) : SV_Target
{
    float4 texColor = 0;
    uint width = 0;
    uint height = 0;
    float4 newCol = 0;
    switch (Input.TexIndex)
    {
        case 0:
            texColor = textures[0].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[0].GetDimensions(width, height);
            break;
        case 1:
            texColor = textures[1].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[1].GetDimensions(width, height);
            break;
        case 2:
            texColor = textures[2].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[2].GetDimensions(width, height);
            break;
        case 3:
            texColor = textures[3].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[3].GetDimensions(width, height);
            break;
        case 4:
            texColor = textures[4].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[4].GetDimensions(width, height);
            break;
        case 5:
            texColor = textures[5].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[5].GetDimensions(width, height);
            break;
        case 6:
            texColor = textures[6].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[6].GetDimensions(width, height);
            break;
        case 7:
            texColor = textures[7].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[7].GetDimensions(width, height);
            break;
        case 8:
            texColor = textures[8].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[8].GetDimensions(width, height);
            break;
        case 9:
            texColor = textures[9].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[9].GetDimensions(width, height);
            break;
        case 10:
            texColor = textures[10].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[10].GetDimensions(width, height);
            break;
        case 11:
            texColor = textures[11].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[11].GetDimensions(width, height);
            break;
        case 12:
            texColor = textures[12].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[12].GetDimensions(width, height);
            break;
        case 13:
            texColor = textures[13].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[13].GetDimensions(width, height);
            break;
        case 14:
            texColor = textures[14].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[14].GetDimensions(width, height);
            break;
        case 15:
            texColor = textures[15].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[15].GetDimensions(width, height);
            break;
        case 16:
            texColor = textures[16].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[16].GetDimensions(width, height);
            break;
        case 17:
            texColor = textures[17].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[17].GetDimensions(width, height);
            break;
        case 18:
            texColor = textures[18].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[18].GetDimensions(width, height);
            break;
        case 19:
            texColor = textures[19].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[19].GetDimensions(width, height);
            break;
        case 20:
            texColor = textures[20].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[20].GetDimensions(width, height);
            break;
        case 21:
            texColor = textures[21].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[21].GetDimensions(width, height);
            break;
        case 22:
            texColor = textures[22].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[22].GetDimensions(width, height);
            break;
        case 23:
            texColor = textures[23].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[23].GetDimensions(width, height);
            break;
        case 24:
            texColor = textures[24].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[24].GetDimensions(width, height);
            break;
        case 25:
            texColor = textures[25].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[25].GetDimensions(width, height);
            break;
        case 26:
            texColor = textures[26].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[26].GetDimensions(width, height);
            break;
        case 27:
            texColor = textures[27].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[27].GetDimensions(width, height);
            break;
        case 28:
            texColor = textures[28].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[28].GetDimensions(width, height);
            break;
        case 29:
            texColor = textures[29].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[29].GetDimensions(width, height);
            break;
        case 30:
            texColor = textures[30].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[30].GetDimensions(width, height);
            break;
        case 31:
            texColor = textures[31].Sample(WrapSampler, Input.TexCoord).rgba;
            textures[31].GetDimensions(width, height);
            break;
    }
    float sd = median(texColor.r, texColor.g, texColor.b);
    float2 widthAndHeight = float2(float(width), float(height));
    float2 pxRange = Input.is3D > 0 ? screenPxRange(Input.TexCoord, widthAndHeight, Input.WorldPosition) : float2(1.5f, 1.5f);
    float screenPxDistance = pxRange * (sd - 0.5f);
    float opacity = clamp(screenPxDistance + 0.5f, 0.0, 1.0);
    
    float4 outputColor = lerp(float4(Input.Color.xyz, 0), Input.Color, opacity);
    return outputColor;
}