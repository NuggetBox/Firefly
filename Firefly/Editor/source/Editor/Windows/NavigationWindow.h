#pragma once
#include "EditorWindow.h"
#include "WindowRegistry.h"

class NavigationWindow : public EditorWindow
{
public:
	NavigationWindow();

	static std::string GetFactoryName() { return "Navigation"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<NavigationWindow>(); }

protected:
	void OnImGui() override;
	int myBenneIsCringe = true;
};
