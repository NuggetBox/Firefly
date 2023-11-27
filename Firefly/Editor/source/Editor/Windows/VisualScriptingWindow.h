#pragma once
#include "EditorWindow.h"
#include "Editor/VisualScripting/ScriptGraphEditor.h"

class VisualScriptingWindow : public EditorWindow
{
public:
	VisualScriptingWindow();

	static std::string GetFactoryName() { return "VisualScriptingWindow"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() 
	{ 
		auto window = std::make_shared<VisualScriptingWindow>(); 
		window->InitializeEditor(); 
		return window;
	}

	void InitializeEditor();

	void OpenVisualScript(const std::filesystem::path& aVisualScriptFile);

	//void OnEvent(Firefly::Event& aEvent) override;

protected:
	void OnImGui() override;
	void AcceptVisualScriptDrop();

	ScriptGraphEditor myScriptGraphEditor;
};