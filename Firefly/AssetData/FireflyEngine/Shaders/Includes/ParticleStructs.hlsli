struct ParticleVertexData
{
	float4 Position : POSITION;

	float4 Color : COLOR;

	float3 Speed : SPEED;
	float SpeedMult : SPEEDMULT;

	float3 Scale : SCALE;
	float ScaleMult : SCALEMULT;

	float Rotation : ROTATION;
	float TotalLifeTime : TOTALLIFETIME;
	float LifeTime : LIFETIME;
	bool Dead : DEAD;

	float2 UV[4] : UV;
};

struct ParticleGeometryToPixel
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float3 Speed : SPEED;
	float LifeTime : LIFETIME;
	float2 UV : TEXCOORD;
};

struct ParticlePixelOutput
{
	float4 Color : SV_TARGET;
};