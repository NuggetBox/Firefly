#pragma once
#include "Firefly/Event/Event.h"
#include "Utils/Math/Vector2.hpp"

class EditorWindow
{
public:
	EditorWindow(const std::string& aDisplayName);
	virtual ~EditorWindow() = default;

	void OnImGuiUpdate();
	virtual void OnEvent(Firefly::Event& aEvent);

	virtual std::string GetName() const = 0;
	std::string GetIDString();
	const std::string& GetDisplayName() const { return myDisplayName; }
	void SetFocused();
	bool IsFocused() const { return myIsFocused; }
	inline bool IsOpen() const { return myIsOpen; }
	inline void SetOpen(bool aOpenFlag) { myIsOpen = aOpenFlag; }

	virtual void WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam) {};

protected:
	virtual void OnImGui();
	static int myCurrentWindowID;

	const int myId;

	std::string myDisplayName;

	bool myIsOpen;
	bool myIsFocused;
	bool myUnsavedChangesFlag;
	bool myNoWindowPadding;
	bool myManualFocusFlag;

	int myWindowFlags = 0;

	Utils::Vector2f myBaseWindowPos;
	Utils::Vector2f myBaseWindowSize;
};
