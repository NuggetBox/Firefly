#include <Includes/ConstStructs.hlsli>

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : UV;
    uint TexIndex : TEXINDEX;
};
sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);
Texture2D textures[32] : register(t0);



float4 main(VertexOutput Input) : SV_Target
{
    float4 texColor = Input.Color;

    Input.TexCoord.y *= -1;

    switch (Input.TexIndex)
    {
        case 0:
            //texColor *= 1;
            break;
        case 1:
            texColor *= textures[1].Sample(WrapSampler, Input.TexCoord);
            break;
        case 2:
            texColor *= textures[2].Sample(WrapSampler, Input.TexCoord);
            break;
        case 3:
            texColor *= textures[3].Sample(WrapSampler, Input.TexCoord);
            break;
        case 4:
            texColor *= textures[4].Sample(WrapSampler, Input.TexCoord);
            break;
        case 5:
            texColor *= textures[5].Sample(WrapSampler, Input.TexCoord);
            break;
        case 6:
            texColor *= textures[6].Sample(WrapSampler, Input.TexCoord);
            break;
        case 7:
            texColor *= textures[7].Sample(WrapSampler, Input.TexCoord);
            break;
        case 8:
            texColor *= textures[8].Sample(WrapSampler, Input.TexCoord);
            break;
        case 9:
            texColor *= textures[9].Sample(WrapSampler, Input.TexCoord);
            break;
        case 10:
            texColor *= textures[10].Sample(WrapSampler, Input.TexCoord);
            break;
        case 11:
            texColor *= textures[11].Sample(WrapSampler, Input.TexCoord);
            break;
        case 12:
            texColor *= textures[12].Sample(WrapSampler, Input.TexCoord);
            break;
        case 13:
            texColor *= textures[13].Sample(WrapSampler, Input.TexCoord);
            break;
        case 14:
            texColor *= textures[14].Sample(WrapSampler, Input.TexCoord);
            break;
        case 15:
            texColor *= textures[15].Sample(WrapSampler, Input.TexCoord);
            break;
        case 16:
            texColor *= textures[16].Sample(WrapSampler, Input.TexCoord);
            break;
        case 17:
            texColor *= textures[17].Sample(WrapSampler, Input.TexCoord);
            break;
        case 18:
            texColor *= textures[18].Sample(WrapSampler, Input.TexCoord);
            break;
        case 19:
            texColor *= textures[19].Sample(WrapSampler, Input.TexCoord);
            break;
        case 20:
            texColor *= textures[20].Sample(WrapSampler, Input.TexCoord);
            break;
        case 21:
            texColor *= textures[21].Sample(WrapSampler, Input.TexCoord);
            break;
        case 22:
            texColor *= textures[22].Sample(WrapSampler, Input.TexCoord);
            break;
        case 23:
            texColor *= textures[23].Sample(WrapSampler, Input.TexCoord);
            break;
        case 24:
            texColor *= textures[24].Sample(WrapSampler, Input.TexCoord);
            break;
        case 25:
            texColor *= textures[25].Sample(WrapSampler, Input.TexCoord);
            break;
        case 26:
            texColor *= textures[26].Sample(WrapSampler, Input.TexCoord);
            break;
        case 27:
            texColor *= textures[27].Sample(WrapSampler, Input.TexCoord);
            break;
        case 28:
            texColor *= textures[28].Sample(WrapSampler, Input.TexCoord);
            break;
        case 29:
            texColor *= textures[29].Sample(WrapSampler, Input.TexCoord);
            break;
        case 30:
            texColor *= textures[30].Sample(WrapSampler, Input.TexCoord);
            break;
        case 31:
            texColor *= textures[31].Sample(WrapSampler, Input.TexCoord);
            break;
    }
    
    return texColor;
}