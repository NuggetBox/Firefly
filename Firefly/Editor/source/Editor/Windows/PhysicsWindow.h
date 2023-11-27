#pragma once
#include "EditorWindow.h"
class PhysicsWindow : public EditorWindow
{
public:
	PhysicsWindow();
	static std::string GetFactoryName() { return "Physics Window"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<PhysicsWindow>(); }
	void OnImGui() override;
private:
	int32_t myInt;
	std::string myInputName;
	std::string myRenameBuffer;
	int32_t myRenameID;
};

