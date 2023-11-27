#include "EditorPch.h"
#include "EditorWindow.h"

int EditorWindow::myCurrentWindowID = 0;

EditorWindow::EditorWindow(const std::string& aDisplayName)
	: myId(myCurrentWindowID++)
{
	myDisplayName = aDisplayName;
	myIsFocused = false;
	myIsOpen = true;
	myUnsavedChangesFlag = false;
	myNoWindowPadding = false;
	myManualFocusFlag = false;
}

void EditorWindow::OnImGui()
{

}

void EditorWindow::OnImGuiUpdate()
{
	FF_PROFILESCOPE(myDisplayName.c_str());

	if (myIsOpen)
	{
		auto idString = GetIDString();
		ImGuiWindowFlags localFlags = 0;

		if (myUnsavedChangesFlag)
		{
			localFlags |= ImGuiWindowFlags_UnsavedDocument;
		}

		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
		if (myNoWindowPadding)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		}
		if (ImGui::Begin(idString.c_str(), &myIsOpen, myWindowFlags | localFlags))
		{
			if (!myManualFocusFlag)
			{
				myIsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
			}

			myBaseWindowPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			myBaseWindowSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
			OnImGui();
		}
		if (myNoWindowPadding)
		{
			ImGui::PopStyleVar();
		}

		ImGui::End();
	}
}

void EditorWindow::OnEvent(Firefly::Event& aEvent)
{

}

std::string EditorWindow::GetIDString()
{
	return myDisplayName + "##" + std::to_string(myId);
}

void EditorWindow::SetFocused()
{
	ImGui::SetWindowFocus(GetIDString().c_str());
	myIsFocused = true;
}