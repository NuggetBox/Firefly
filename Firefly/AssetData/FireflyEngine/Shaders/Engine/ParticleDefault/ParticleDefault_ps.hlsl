#include <Includes/ParticleStructs.hlsli>
#include <Includes/FireflyShader.hlsli>

SamplerState defaultSampler : register(s0);
Texture2D albedoTexture : register(t0);

struct Output
{
    float4 colorResult : SV_Target0;
};

Output main(ParticleGeometryToPixel input)
{
    Output result;
    float4 textureColor = albedoTexture.Sample(defaultSampler, input.UV);

	//Helps avoid extra alpha testing
	if (textureColor.a <= 0.05f)
	{
		result.colorResult.rgba = float4(0, 0, 0, 0);
		discard;
		return result;
	}

	//const float3 pbr = CalculatePBR(textureColor.rgb * input.Color.rgb, (normalize(cameraPosition - input.WorldPosition)).rgb, 1, 0, input.WorldPosition, (normalize(cameraPosition - input.WorldPosition)).rgb, 1.0f, float3(0, 0, 0));
	//result.colorResult.rgba = float4(pbr, textureColor.a * input.Color.a);

	result.colorResult.rgba = textureColor.rgba * input.Color.rgba;

	return result;
}