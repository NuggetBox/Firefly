#pragma once
#include "EditorWindow.h"

class RenderStatsWindow : public EditorWindow
{
public:
	RenderStatsWindow();

	static std::string GetFactoryName() { return "Render Stats"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<RenderStatsWindow>(); }

	//void OnEvent(Firefly::Event& aEvent) override;

protected:
	void OnImGui() override;

private:
	float myGetFPSTimer;
	float myPrevDelta;
};