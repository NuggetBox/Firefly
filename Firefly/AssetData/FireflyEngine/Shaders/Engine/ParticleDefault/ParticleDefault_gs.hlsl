#include <Includes/ConstStructs.hlsli>
#include <Includes/ParticleStructs.hlsli>

[maxvertexcount(4)]
void main(point ParticleVertexData input[1], inout TriangleStream<ParticleGeometryToPixel> output)
{
	const float2 offsets[4] =
	{
		{-1.0f, 1.0f},
		{1.0f, 1.0f},
		{-1.0f, -1.0f},
		{1.0f, -1.0f}
	};

	const ParticleVertexData inputParticle = input[0];

	for (unsigned int i = 0; i < 4; ++i)
	{
		ParticleGeometryToPixel result;

		float2 rotatedOffset;
		rotatedOffset.x = cos(inputParticle.Rotation) * offsets[i].x - sin(inputParticle.Rotation) * offsets[i].y;
		rotatedOffset.y = sin(inputParticle.Rotation) * offsets[i].x + cos(inputParticle.Rotation) * offsets[i].y;

		result.Position = mul(toView, inputParticle.Position);
		result.Position.xy += rotatedOffset * inputParticle.Scale.xy;
		result.Position = mul(toProjection, result.Position);

		result.Color = inputParticle.Color;

		result.Speed = inputParticle.Speed;
		result.LifeTime = inputParticle.TotalLifeTime - inputParticle.LifeTime;

		result.UV = inputParticle.UV[i];

		output.Append(result);
	}
}