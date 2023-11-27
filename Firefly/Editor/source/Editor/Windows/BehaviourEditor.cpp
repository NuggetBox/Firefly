#include "EditorPch.h"
#include "BehaviourEditor.h"
#include "imgui_node_editor.h"

REGISTER_WINDOW(BehaviourEditor);

namespace ed = ax::NodeEditor;

void ImGuiEx_BeginColumn()
{
	ImGui::BeginGroup();
}

void ImGuiEx_NextColumn()
{
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
}

void ImGuiEx_EndColumn()
{
	ImGui::EndGroup();
}

BehaviourEditor::BehaviourEditor() : EditorWindow("Behaviour")
{
	ed::Config config;
	config.SettingsFile = "Behaviour.json";
	myContext = ed::CreateEditor(&config);
}

void BehaviourEditor::OnImGui()
{
	
}
