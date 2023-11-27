
SamplerState WrapSampler : register(s0);
Texture2D textures[32] : register(t0);

struct BillboardPixelIn
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : UV;
    uint2 EntityID : ENTITYID;
    uint TextureID : TextureID;
};
struct Output
{
    float4 colorResult : SV_Target0;
};
Output main(BillboardPixelIn input)
{
    Output result = (Output)0;
    float4 texColor = 1.f;
    switch (input.TextureID)
    {
        case 0:
            break;
        case 1:
            texColor *= textures[1].Sample(WrapSampler, input.UV);
            break;
        case 2:
            texColor *= textures[2].Sample(WrapSampler, input.UV);
            break;
        case 3:
            texColor *= textures[3].Sample(WrapSampler, input.UV);
            break;
        case 4:
            texColor *= textures[4].Sample(WrapSampler, input.UV);
            break;
        case 5:
            texColor *= textures[5].Sample(WrapSampler, input.UV);
            break;
        case 6:
            texColor *= textures[6].Sample(WrapSampler, input.UV);
            break;
        case 7:
            texColor *= textures[7].Sample(WrapSampler, input.UV);
            break;
        case 8:
            texColor *= textures[8].Sample(WrapSampler, input.UV);
            break;
        case 9:
            texColor *= textures[9].Sample(WrapSampler, input.UV);
            break;
        case 10:
            texColor *= textures[10].Sample(WrapSampler, input.UV);
            break;
        case 11:
            texColor *= textures[11].Sample(WrapSampler, input.UV);
            break;
        case 12:
            texColor *= textures[12].Sample(WrapSampler, input.UV);
            break;
        case 13:
            texColor *= textures[13].Sample(WrapSampler, input.UV);
            break;
        case 14:
            texColor *= textures[14].Sample(WrapSampler, input.UV);
            break;
        case 15:
            texColor *= textures[15].Sample(WrapSampler, input.UV);
            break;
        case 16:
            texColor *= textures[16].Sample(WrapSampler, input.UV);
            break;
        case 17:
            texColor *= textures[17].Sample(WrapSampler, input.UV);
            break;
        case 18:
            texColor *= textures[18].Sample(WrapSampler, input.UV);
            break;
        case 19:
            texColor *= textures[19].Sample(WrapSampler, input.UV);
            break;
        case 20:
            texColor *= textures[20].Sample(WrapSampler, input.UV);
            break;
        case 21:
            texColor *= textures[21].Sample(WrapSampler, input.UV);
            break;
        case 22:
            texColor *= textures[22].Sample(WrapSampler, input.UV);
            break;
        case 23:
            texColor *= textures[23].Sample(WrapSampler, input.UV);
            break;
        case 24:
            texColor *= textures[24].Sample(WrapSampler, input.UV);
            break;
        case 25:
            texColor *= textures[25].Sample(WrapSampler, input.UV);
            break;
        case 26:
            texColor *= textures[26].Sample(WrapSampler, input.UV);
            break;
        case 27:
            texColor *= textures[27].Sample(WrapSampler, input.UV);
            break;
        case 28:
            texColor *= textures[28].Sample(WrapSampler, input.UV);
            break;
        case 29:
            texColor *= textures[29].Sample(WrapSampler, input.UV);
            break;
        case 30:
            texColor *= textures[30].Sample(WrapSampler, input.UV);
            break;
        case 31:
            texColor *= textures[31].Sample(WrapSampler, input.UV);
            break;
    }
    if(texColor.a < 0.1)
    {
        clip(texColor.a - 0.05f);
    }
    result.colorResult = texColor * input.Color;

	return result;
}