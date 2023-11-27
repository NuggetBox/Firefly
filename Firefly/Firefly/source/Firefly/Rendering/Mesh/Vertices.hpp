#pragma once
#include "Utils/Math/Vector.h"

struct Vertex
{
	Vertex() = default;

	//Pos, UV0, Normal, Tangent, Binormal
	Vertex(
		float aX, float aY, float aZ, 
		float aU, float aV, 
		float aNX, float aNY, float aNZ,
		float aTX, float aTY, float aTZ, 
		float aBX, float aBY, float aBZ)
	{
		Position[0] = aX;
		Position[1] = aY;
		Position[2] = aZ;
		//Position[3] = 1;

		UVs[0][0] = aU;
		UVs[0][1] = aV;

		Normal[0] = aNX;
		Normal[1] = aNY;
		Normal[2] = aNZ;

		Tangent[0] = aTX;
		Tangent[1] = aTY;
		Tangent[2] = aTZ;

		Binormal[0] = aBX;
		Binormal[1] = aBY;
		Binormal[2] = aBZ;
	}

	Vertex(float aPosition[4], float someVertexColors[4][4], float someUVs[4][2], float aNormal[3], float aTangent[3], float aBinormal[3], unsigned int someBoneIDs[4], float someBoneWeights[4])
	{
		memcpy(Position, aPosition, sizeof(float) * 4);
		memcpy(VertexColors, someVertexColors, sizeof(float) * 4 * 4);
		memcpy(UVs, someUVs, sizeof(float) * 4 * 2);
		memcpy(Normal, aNormal, sizeof(float) * 3);
		memcpy(Tangent, aTangent, sizeof(float) * 3);
		memcpy(Binormal, aBinormal, sizeof(float) * 3);
		memcpy(BoneIDs, someBoneIDs, sizeof(unsigned int) * 4);
		memcpy(BoneWeights, someBoneWeights, sizeof(float) * 4);
	}

	float Position[4] = { 0, 0, 0, 1 };
	float VertexColors[4][4]
	{
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};

	float UVs[4][2]
	{
		{0, 0},
		{0, 0},
		{0, 0},
		{0, 0}
	};

	float Normal[3] = { 0, 0, 0 };
	float Tangent[3] = { 0, 0, 0 };
	float Binormal[3] = { 0, 0, 0 };

	unsigned int BoneIDs[4] = { 0, 0, 0, 0 };
	float BoneWeights[4] = { 0, 0, 0, 0 };

};

struct SpriteVertex
{
	Utils::Vector3f Position;
	Utils::Vector4f Color;
	Utils::Vector2f UV;
	uint32_t TexId;
	uint32_t Is3D;
};

struct BillBoardVertex
{
	Utils::Vector4f Position;
	uint32_t TexId;
	Utils::Vector2<uint32_t> EntityID;
	Utils::Vec4 Color;
};

struct LineVertex
{
	Utils::Vector4f Position;
	Utils::Vector4f Color;
};
