#pragma once
#include "Editor/Windows/EditorWindow.h"
#include "Firefly/Core/Log/DebugLogger.h"
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector4.hpp"
#include <map>

namespace Firefly
{
	class Texture2D;
}

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow();
	virtual ~ConsoleWindow() = default;
	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<ConsoleWindow>(); }
	static std::string GetFactoryName() { return "Console"; }
	std::string GetName() const override { return GetFactoryName(); }
private:
	const Utils::Vector4f& GetTextColor(MessageType aMessageType);
	Ref<Firefly::Texture2D> GetIcon(MessageType aMessageType);
	bool ShouldMessageTypeShow(MessageType aMessageType);
	void LogCallback(MessageType aMessageType, const std::string& aTimeString, const std::string& aFileName, const std::string& aFunctionName, const std::string& aCodeLine, const std::string& aMessage);

	struct MessageData
	{
		MessageType myMessageType;
		std::string myTimeString;
		std::string myFileName;
		std::string myFunctionName;
		std::string myCodeLine;
		std::string myMessage;
		double myTime;
	};
	void DrawMessageTab(MessageData& aShownMessage, int aMessageCount = -1);

	Ref<Firefly::Texture2D> myErrorIcon;
	Ref<Firefly::Texture2D> myWarningIcon;
	Ref<Firefly::Texture2D> myInfoIcon;

	std::unordered_map<std::string, uint32_t> myMessagesCollapsedIndices;
	std::vector < std::pair<MessageData, uint32_t>>myMessagesCollapsed;
	std::vector<MessageData> myMessages;

	const Utils::Vector4f myErrorTextColor = { 222.f / 255.f, 57.f / 255.f, 45.f / 255.f, 1.0f };
	const Utils::Vector4f myWarningTextColor = { 221.f / 255.f, 185.f / 255.f, 53.f / 255.f, 1.0f };
	const Utils::Vector4f myInfoTextColor = { 127.f / 255.f, 178.f / 255.f, 255.f / 255.f, 1.0f };

	bool myShowErrors;
	bool myShowWarnings;
	bool myShowInfo;
	bool myCollapsed;
	bool myAutoScrollFlag;
	bool myShouldNotify;
};

