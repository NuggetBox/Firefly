#include <Includes/ConstStructs.hlsli>
#include <Includes/ParticleStructs.hlsli>

ParticleVertexData main(ParticleVertexData input)
{
	ParticleVertexData result;

	const float4 particleLocalPosition = input.Position;
	const float4 particleWorldPosition = mul(PS_ToWorld, particleLocalPosition);

	result.Position = particleWorldPosition;
	result.Rotation = input.Rotation;

	result.Color = input.Color;

	result.SpeedMult = input.SpeedMult;
	result.Speed = input.Speed;

	result.ScaleMult = input.ScaleMult;
	result.Scale = input.Scale;

	result.TotalLifeTime = input.TotalLifeTime;
	result.LifeTime = input.LifeTime;

	result.Dead = input.Dead;

	result.UV[0][0] = input.UV[0][0];
	result.UV[0][1] = input.UV[0][1];
	result.UV[1][0] = input.UV[1][0];
	result.UV[1][1] = input.UV[1][1];
	result.UV[2][0] = input.UV[2][0];
	result.UV[2][1] = input.UV[2][1];
	result.UV[3][0] = input.UV[3][0];
	result.UV[3][1] = input.UV[3][1];

	return result;
}