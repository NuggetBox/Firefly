struct PixelOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

float4 main(PixelOutput pInput) : SV_TARGET
{
	return pInput.Color;
}