#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>

Texture2D OcclusionMap : register(t0);
Texture2D normals : register(t1);

float4 dssdo_blur(float2 tex, float2 dir)
{
    float weights[9] =
    {
        0.013519569015984728,
		0.047662179108871855,
		0.11723004402070096,
		0.20116755999375591,
		0.240841295721373,
		0.20116755999375591,
		0.11723004402070096,
		0.047662179108871855,
		0.013519569015984728
    };

    float indices[9] = { -4, -3, -2, -1, 0, +1, +2, +3, +4 };

    float2 step = dir / resolution.xy;

    float3 normal[9];

    normal[0] = normals.Sample(WrapSampler, tex + indices[0] * step).xyz;
    normal[1] = normals.Sample(WrapSampler, tex + indices[1] * step).xyz;
    normal[2] = normals.Sample(WrapSampler, tex + indices[2] * step).xyz;
    normal[3] = normals.Sample(WrapSampler, tex + indices[3] * step).xyz;
    normal[4] = normals.Sample(WrapSampler, tex + indices[4] * step).xyz;
    normal[5] = normals.Sample(WrapSampler, tex + indices[5] * step).xyz;
    normal[6] = normals.Sample(WrapSampler, tex + indices[6] * step).xyz;
    normal[7] = normals.Sample(WrapSampler, tex + indices[7] * step).xyz;
    normal[8] = normals.Sample(WrapSampler, tex + indices[8] * step).xyz;

    float total_weight = 1.0;
    float discard_threshold = 0.85;

    int i;

    for (i = 0; i < 9; ++i)
    {
        if (dot(normal[i], normal[4]) < discard_threshold)
        {
            total_weight -= weights[i];
            weights[i] = 0;
        }
    }

	//

    float4 res = 0;

    for (i = 0; i < 9; ++i)
    {
        res += OcclusionMap.Sample(WrapSampler, tex + indices[i] * step) * weights[i];
    }

    res /= total_weight;

    return res;
}


float4 main(VertextoPixel input) : SV_Target
{
    float4 blur = dssdo_blur(input.uv, float2(0, 1));
    return blur;
}