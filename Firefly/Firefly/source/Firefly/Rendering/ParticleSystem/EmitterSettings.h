#pragma once
#include "Firefly/Rendering/RenderStateManager.h"
#include "Utils/Math/Vector.h"
#include "Utils/Math/Lerps.hpp"

#include "imgradient/ImGradientHDR.h"

enum class EmitterType
{
	Cone,
	Sphere,
	Rectangle,
	Mesh,
	AnimatedMesh
};

enum class ForceFieldType : int
{
	Point, Cuboid
};

inline const char* GlobalForceFieldTypes = "Point\0Cuboid";

struct ForceField
{
	ForceFieldType ForceFieldType = ForceFieldType::Point;
	Utils::Vector3f Direction = {1.0f, 0.0f, 0.0f}; //Direction for cuboid fields
	Utils::Vector3f Position;
	float Range = 50.0f;
	float Force = 1000.0f;
	bool Lerp;
	bool Global = false;
	Utils::LerpType LerpType = Utils::LerpType::COUNT;
	float LerpPower;
};

struct EmitterSettings
{
	EmitterType EmitterType = EmitterType::Cone;

	Firefly::BlendState BlendState = Firefly::BlendState::Additive;

	float SpawnRate = 50.0f;
	float MinLifeTime = 1.0f;
	float MaxLifeTime = 3.0f;

	Utils::Vector3f StartScale = { 10.0f, 10.0f, 10.0f };
	Utils::Vector3f EndScale = { 10.0f, 10.0f, 10.0f };
	float ScaleVariation = 1.0f;

	ImGui::ImGradientHDRState ColorGradient;

	float Speed = 100.0f;
	float SpeedVariation = 1.0f;

	bool UseAcceleration = false;
	Utils::Vector3f Acceleration = { 0.0f, -100.0f, 0.0f };
	float MaxSpeed = 250.0f;

	bool UseGlobalForceFields = true;
	std::vector<ForceField> LocalForceFields;

	float RotationSpeed = 0.0f;
	bool RandomSpawnRotation = false;
	bool Global = true; //Global or local particles
	bool Looping = true;
	float MaxEmitTime = 1.0f;
	bool ScaledDeltaTime = true;

	bool IsFlipbook = false;
	int Frames = 1;
	int Rows = 1;
	int Columns = 1;
	int FrameRate = 0;

	bool LoopInnerChangeSpeed = false;
	bool LoopOuterChangeSpeed = false;
	bool BounceRadiusChange = false;

	float MeshNormalOffset = 0.0f;

	float Height = 100.0f;

	//Cone
	float InnerRadius = 10.0f;
	float InnerRadiusChangeSpeed = 0.0f;
	float InnerRadiusMax = 20.0f;
	float OuterRadius = 50.0f;
	float OuterRadiusChangeSpeed = 0.0f;
	float OuterRadiusMax = 100.0f;
	bool SpawnOnEdge = true;
	bool AimForEdge = true;

	//Sphere
	float Radius = 50.0f;
	float RadiusChangeSpeed = 0.0f;
	float RadiusMax = 100.0f;
	bool SpawnOnSurface = true;

	//Rectangle
	float InnerRectangleWidth = 100.0f;
	float OuterRectangleWidth = 100.0f;
	float InnerRectangleHeight = 100.0f;
	float OuterRectangleHeight = 100.0f;
};