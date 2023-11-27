#include <Includes/Conststructs.hlsli>

struct GSOutput
{
    float4 pos : SV_POSITION;
    uint layer : SV_RenderTargetArrayIndex;
};
struct GSInput
{
    float4 WorldPosition : POSITION;
};
[maxvertexcount(15)]
void main(
	triangle GSInput input[3],
	inout TriangleStream<GSOutput> output)
{
    for (int h = 0; h < 5; h++)
    {
        GSOutput element;
        element.layer = h;
        
        for (uint strip = 0; strip < 3; strip++)
        {
            element.pos = mul(dirLight[h].projMatrix, mul(dirLight[h].viewMatrix, input[strip].WorldPosition));
            output.Append(element);
            
        }
        output.RestartStrip();
    }
}