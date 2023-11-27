#include "EditorPch.h"
#include "ConsoleWindow.h"
#include "Editor/Windows/WindowRegistry.h"
#include "imnotify/fonts/font_awesome_5.h"
#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Utils/Timer.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "imgui/imgui_internal.h"

REGISTER_WINDOW(ConsoleWindow);

#include "Firefly/Core/Log/DebugLogger.h"
ConsoleWindow::ConsoleWindow() : EditorWindow("Console")
{
	DebugLogger::QueueDetailedLogFunc(std::bind(&ConsoleWindow::LogCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	myWindowFlags |= ImGuiWindowFlags_MenuBar;
	myShowErrors = true;
	myShowWarnings = true;
	myShowInfo = true;
	myCollapsed = true;
	myAutoScrollFlag = true;
	myShouldNotify = false;

	myErrorIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_error.dds", true);
	myWarningIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_warning.dds", true);
	myInfoIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_info.dds", true);
}

void ConsoleWindow::OnImGui()
{
	if (ImGui::BeginMenuBar())
	{
		//ERRORS
		bool showErrorsWasSelected = myShowErrors;
		if (myShowErrors)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::ImageButton(myErrorIcon->GetSRV().Get(), { 20,20 }))
		{
			myShowErrors = !myShowErrors;
		}
		if (showErrorsWasSelected)
		{
			ImGui::PopStyleColor();
		}
		//

		ImGui::SameLine();

		//WARNINGS
		bool showWarningsWasSelected = myShowWarnings;
		if (myShowWarnings)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::ImageButton(myWarningIcon->GetSRV().Get(), { 20,20 }))
		{
			myShowWarnings = !myShowWarnings;
		}
		if (showWarningsWasSelected)
		{
			ImGui::PopStyleColor();
		}
		//

		ImGui::SameLine();

		//INFO
		bool showInfoWasSelected = myShowInfo;
		if (myShowInfo)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::ImageButton(myInfoIcon->GetSRV().Get(), { 20,20 }))
		{
			myShowInfo = !myShowInfo;
		}
		if (showInfoWasSelected)
		{
			ImGui::PopStyleColor();
		}
		//

		//Collapse
		bool collapsedWasSelected = myCollapsed;
		if (myCollapsed)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::Button("Collapse"))
		{
			myCollapsed = !myCollapsed;
		}
		if (collapsedWasSelected)
		{
			ImGui::PopStyleColor();
		}
		//

		//Auto scroll
		bool autoScrollWasSelected = myAutoScrollFlag;
		if (myAutoScrollFlag)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::Button("Auto Scroll"))
		{
			myAutoScrollFlag = !myAutoScrollFlag;
		}
		if (autoScrollWasSelected)
		{
			ImGui::PopStyleColor();
		}
		//

		//Should Notify
		bool shouldNotifyWasSelected = myShouldNotify;
		if (myShouldNotify)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		}
		if (ImGui::Button("Notify"))
		{
			myShouldNotify = !myShouldNotify;
		}
		if (shouldNotifyWasSelected)
		{
			ImGui::PopStyleColor();
		}

		ImGui::EndMenuBar();
	}


	auto drawList = ImGui::GetWindowDrawList();

	if (myCollapsed)
	{
		for (auto messagePair : myMessagesCollapsed)
		{
			auto& messageData = messagePair.first;
			auto& messageCount = messagePair.second;
			if (!ShouldMessageTypeShow(messageData.myMessageType))
			{
				continue;
			}

			DrawMessageTab(messageData, messageCount);
		}
	}
	else
	{
		for (auto& message : myMessages)
		{
			if (!ShouldMessageTypeShow(message.myMessageType))
			{
				continue;
			}

			DrawMessageTab(message);
		}
	}
	if (myAutoScrollFlag)
	{
		if (ImGuiUtils::IsScrollBarBeingUsed())
		{
			myAutoScrollFlag = false;
		}
		else
		{
			ImGui::SetScrollY(ImGui::GetScrollMaxY());
		}
	}
}

const Utils::Vector4f& ConsoleWindow::GetTextColor(MessageType aMessageType)
{
	switch (aMessageType)
	{
		case MessageType::Error:
			return myErrorTextColor;
		case MessageType::Warning:
			return myWarningTextColor;
		case MessageType::Info:
			return myInfoTextColor;
		default:
			return myInfoTextColor;
	}
}

Ref<Firefly::Texture2D> ConsoleWindow::GetIcon(MessageType aMessageType)
{
	switch (aMessageType)
	{
		case MessageType::Error:
			return myErrorIcon;
		case MessageType::Warning:
			return myWarningIcon;
		case MessageType::Info:
			return myInfoIcon;
		default:
			return myInfoIcon;
	}
}

bool ConsoleWindow::ShouldMessageTypeShow(MessageType aMessageType)
{
	switch (aMessageType)
	{
		case MessageType::Error:
			return myShowErrors;
		case MessageType::Warning:
			return myShowWarnings;
		case MessageType::Info:
			return myShowInfo;
		default:
			return true;
	}
}

void ConsoleWindow::LogCallback(MessageType aMessageType, const std::string& aTimeString, const std::string& aFileName, const std::string& aFunctionName, const std::string& aCodeLine, const std::string& aMessage)
{
	MessageData messageData;
	messageData.myMessageType = aMessageType;
	messageData.myTimeString = aTimeString;
	messageData.myFileName = aFileName;
	messageData.myFunctionName = aFunctionName;
	messageData.myCodeLine = aCodeLine;
	messageData.myMessage = aMessage;
	messageData.myTime = Utils::Timer::GetTotalTime();
	auto messageID = aFileName + aCodeLine;


	if (myMessagesCollapsedIndices.contains(messageID))
	{
		myMessagesCollapsed[myMessagesCollapsedIndices[messageID]].first = messageData;
		myMessagesCollapsed[myMessagesCollapsedIndices[messageID]].second += 1;
	}
	else
	{
		myMessagesCollapsedIndices.insert({ messageID, myMessagesCollapsed.size() });
		myMessagesCollapsed.push_back({ messageData, 1 });
	}
	myMessages.push_back(messageData);

	if (myShouldNotify && messageData.myMessageType == MessageType::Error)
	{
		ImGuiUtils::NotifyError(("ERROR LOGGED:\n"+ messageData.myMessage + "\n\nYou can read more in the Error log (Console Window)").c_str());
	}
}

void ConsoleWindow::DrawMessageTab(MessageData& aShownMessage, int aMessageCount)
{
	auto drawList = ImGui::GetWindowDrawList();
	const float tabHeight = 50.0f;
	const float rectRounding = 10.f;

	auto color = GetTextColor(aShownMessage.myMessageType);
	ImGui::PushStyleColor(ImGuiCol_Text, UtilsVecToImGuiVec(color));

	const auto tabWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 min = ImGui::GetCursorScreenPos();
	ImVec2 max = ImGui::GetCursorScreenPos() + ImVec2(tabWidth, tabHeight);


	//background
	drawList->AddRectFilled(min, max, ImGui::GetColorU32({ 0.2f,0.2f,0.2f,1.f }), rectRounding);
	//

	//Icon
	const auto iconBackgroundWidth = tabHeight;
	const auto iconBackgroundMax = min + ImVec2(iconBackgroundWidth, tabHeight);
	drawList->AddRectFilled(min, iconBackgroundMax, ImGui::GetColorU32({ 0.25f ,0.25f ,0.25f ,1.f }), rectRounding, ImDrawFlags_RoundCornersLeft);

	auto icon = GetIcon(aShownMessage.myMessageType);

	const auto halfTabHeight = tabHeight / 2.f;
	const auto halfIconBackgroundWidth = iconBackgroundWidth / 2.f;

	const auto iconMin = min + ImVec2(halfIconBackgroundWidth / 2.f, halfTabHeight / 2.f);
	const auto iconMax = iconMin + ImVec2(halfIconBackgroundWidth, halfTabHeight);
	drawList->AddImage(icon->GetSRV().Get(), iconMin, iconMax);
	//

	//Message
	const auto padding = 2.f;
	const auto messageTextStartPos = min + ImVec2(padding + tabHeight, padding);
	const auto messageFormatted = ImGuiUtils::AddDotsIfMaxSize(aShownMessage.myMessage, tabWidth - iconBackgroundWidth);
	drawList->AddText(messageTextStartPos, ImGui::GetColorU32(ImGuiCol_Text), messageFormatted.c_str());
	//

	//Push info font
	const auto messageTextHeight = ImGui::CalcTextSize("").y;
	ImGuiUtils::PushFont(ImGuiUtilsFont_Roboto_10);
	const auto smallFontHeight = ImGui::CalcTextSize("").y;
	//

	//File
	const auto fileTextStartPos = messageTextStartPos + ImVec2(0, padding + messageTextHeight);
	drawList->AddText(fileTextStartPos, ImGui::GetColorU32(ImGuiCol_Text), aShownMessage.myFileName.c_str());
	//

	//Function and code line
	const auto functionTextStartPos = fileTextStartPos + ImVec2(0, padding + smallFontHeight);
	const auto text = aShownMessage.myFunctionName + " [Line: " + aShownMessage.myCodeLine + "]";
	drawList->AddText(functionTextStartPos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
	//

	//pop info font
	ImGuiUtils::PopFont();
	//

	//Time stamp
	auto timeStampText = "Time: " + aShownMessage.myTimeString;
	const auto timeStampStringSize = ImGui::CalcTextSize(timeStampText.c_str());
	const auto timeStampTextPos = min + ImVec2(tabWidth, tabHeight) - timeStampStringSize - ImVec2(rectRounding, padding);
	ImGui::PushClipRect(min + ImVec2(iconBackgroundWidth, 0), max, true);
	drawList->AddText(timeStampTextPos, ImGui::GetColorU32(ImGuiCol_Text), timeStampText.c_str());
	ImGui::PopClipRect();
	//

	if (aMessageCount != -1)
	{
		/*const auto circleRadius = 10.f;
		const auto circleCenterPos = ImVec2(max.x - circleRadius - rectRounding, min.y + circleRadius + rectRounding);
		drawList->AddCircle(circleCenterPos, circleRadius, ImGui::GetColorU32({ 1,1,1,1 }));*/
		const auto messageCountText = "Count: [" + std::to_string(aMessageCount) + "]";
		const auto messageCountTextSize = ImGui::CalcTextSize(messageCountText.c_str());

		const auto messageCountTextPos = ImVec2(max.x - rectRounding - messageCountTextSize.x, min.y + padding);

		const auto messageCountTextBackgroundMin = messageCountTextPos - ImVec2(padding + rectRounding, padding);
		const auto messageCountTextBackgroundMax = messageCountTextPos + messageCountTextSize + ImVec2(padding + rectRounding, padding);

		drawList->AddRectFilled(messageCountTextBackgroundMin, messageCountTextBackgroundMax, ImGui::GetColorU32({ 0,0,0,0.2f }), 10.f, ImDrawFlags_RoundCornersLeft);
		drawList->AddText(messageCountTextPos, ImGui::GetColorU32(ImGuiCol_Text), messageCountText.c_str());
	}

	//Border
	drawList->AddRect(min, max, ImGui::GetColorU32({ 0.1f,0.1f,0.1f,1.f }), rectRounding, 0, 3.f);
	//

	ImGui::SetCursorScreenPos(min + ImVec2(0, tabHeight));
	ImGui::PopStyleColor();
}
