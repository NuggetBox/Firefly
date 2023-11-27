#pragma once
#include "Utils/Math/Matrix4x4.hpp"
#include "Firefly/Core/Core.h"
#include <Firefly/Rendering/Buffer/ConstantBuffers.h>
#include <Firefly/Rendering/Camera/Camera.h>

namespace Firefly
{
	struct Cascade
	{
		Utils::Mat4 View;
		Utils::Mat4 Projection;
		float Levels = 0;
	};

	class CascadeBuilder
	{
	public:
		CascadeBuilder(DirLightData& aDirLight);


		// This should be handled by the Engine automaticly but this will work for now. // Niklas
		CascadeBuilder& SetCascadeCount(uint32_t aCascadeCount);
		CascadeBuilder& SetActiveCamera(Ptr<Camera> aCamera);
		std::vector<Cascade> Build();

	private:
		Cascade CreateCascade(float aNearPlane, float aFarPlane);

		uint32_t myAmountOfCascades;
		Ptr<Camera> myCam;
		std::vector<float> myShadowLevels;
		DirLightData& myDirlight;
	};
}
