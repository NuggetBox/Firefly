#pragma once
#include <Firefly/ComponentSystem/Entity.h>
#include <Utils/Math/Lerps.hpp>

#include "EditorWindow.h"

class LerpVisualizerWindow : public EditorWindow
{
public:
	LerpVisualizerWindow();

	static std::string GetFactoryName() { return "Lerp Visualizer"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<LerpVisualizerWindow>(); }

	static inline const char* LerpsCharCombo = "Lerp\0EaseIn\0EaseOut\0EaseInOut\0Bounce\0Logerp\0Parabola\0BounceCustom";

protected:
	void OnImGui() override;

private:
	Utils::LerpType mySelectedLerp = Utils::LerpType::Lerp;

	float myPower = 2.0f;
	float mySquish = 2.0f;

	int myBounceCount = 3;
	float myHeightLoss = 0.5f;

	int myLerpVisualRes = 50;
};