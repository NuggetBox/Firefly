#pragma once
#include "EditorWindow.h"
#include "WindowRegistry.h"
#include "imgui/imgui_internal.h"
#include "imgui_node_editor.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

class BehaviourEditor : public EditorWindow
{
public:
	BehaviourEditor();

	static std::string GetFactoryName() { return "Behaviour"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<BehaviourEditor>(); }

protected:
	void OnImGui() override;
private:
	ax::NodeEditor::EditorContext* myContext = nullptr;

};

