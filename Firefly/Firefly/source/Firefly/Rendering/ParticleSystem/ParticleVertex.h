#pragma once
#include "Utils/Math/Vector.h"

struct ParticleVertex
{
	Utils::Vector4f Position = { 0.0f, 0.0f, 0.0f, 1.0f };

	Utils::Vector4f Color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Utils::Vector3f Speed = { 0.0f, 0.0f, 0.0f };
	float SpeedVariation = 1.0f;

	Utils::Vector3f Scale = { 1.0f, 1.0f, 1.0f };
	float ScaleVariation = 1.0f;

	float Rotation = 0.0f;
	float TotalLifeTime = 0.0f;
	float LifeTime = 0.0f;
	bool Dead = true;

	float UVs[4][2]
	{
		{0, 0},
		{1, 0},
		{0, 1},
		{1, 1}
	};
};