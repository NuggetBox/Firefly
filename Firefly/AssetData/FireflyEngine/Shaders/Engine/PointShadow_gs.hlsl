#include <Includes/Conststructs.hlsli>

struct GSOutput
{
    float4 pos : SV_POSITION;
    float3 worldPosition : POSITION;
    uint layer : SV_RenderTargetArrayIndex;
};
struct GSInput
{
    float4 WorldPosition : POSITION;
};
[maxvertexcount(18)]
void main(
	triangle GSInput input[3],
	inout TriangleStream<GSOutput> output)
{
    for (int h = 0; h < 6; h++)
    {
        GSOutput element = (GSOutput)0;
        element.layer = h;
        
        for (uint strip = 0; strip < 3; strip++)
        {
            element.pos = mul(pointLight[nearPlane].transforms[h], input[strip].WorldPosition);
            element.worldPosition = input[strip].WorldPosition;
            output.Append(element);
            
        }
        output.RestartStrip();
    }
}