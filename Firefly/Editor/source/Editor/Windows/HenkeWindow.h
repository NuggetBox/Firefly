#pragma once
#include "EditorWindow.h"
#include "WindowRegistry.h"
#include "imgui/imgui_internal.h"
#include "imgui_node_editor.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "Utils/Math/Vector3.hpp"


class HenkeEditor : public EditorWindow
{
public:
	HenkeEditor();

	static std::string GetFactoryName() { return "Henkes Window"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<HenkeEditor>(); }

protected:
	void OnImGui() override;
private:
	Utils::Vec3 pos;
	Utils::Vec3 pos2;
	int amount = 0;
	float tiden = 0;
	bool RigidBool;


	std::string filename;
};

