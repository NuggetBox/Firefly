#include "EditorPch.h"
#include "VisualScriptingWindow.h"

#include <Editor/EditorLayer.h>
#include <Editor/Utilities/ImGuiUtils.h>

#include "WindowRegistry.h"

REGISTER_WINDOW(VisualScriptingWindow);

VisualScriptingWindow::VisualScriptingWindow()
	: EditorWindow("Visual Scripting")
{
}

void VisualScriptingWindow::InitializeEditor()
{
	myScriptGraphEditor.Init();
}

void VisualScriptingWindow::OpenVisualScript(const std::filesystem::path& aVisualScriptFile)
{
	myScriptGraphEditor.OpenScriptGraph(aVisualScriptFile);
}

//void VisualScriptingWindow::OnEvent(Firefly::Event& aEvent)
//{
//}

void VisualScriptingWindow::OnImGui()
{
	AcceptVisualScriptDrop();
	myScriptGraphEditor.Render();
}

void VisualScriptingWindow::AcceptVisualScriptDrop()
{
	if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("FILE", ".flow"))
	{
		SetFocused();
		const char* file = static_cast<const char*>(payload->Data);
		OpenVisualScript(file);
	}
}