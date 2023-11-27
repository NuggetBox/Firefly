#pragma once
#include "imnotify/imgui_notify.h"
#include "Utils/Math/Vector2.hpp"
#include "Utils/Math/Vector4.hpp"
#include "Firefly/Core/Core.h"

class UndoHandler;

namespace Firefly
{
	class Entity;
	class Component;
}

enum ImGuiUtilsFont_
{
	ImGuiUtilsFont_Roboto_10,
	ImGuiUtilsFont_Roboto_16,
	ImGuiUtilsFont_RobotoBold_10,
	ImGuiUtilsFont_RobotoBold_16,
	//ImGuiUtilsFont_RobotoItalic
	ImGuiUtilsFont_Count
};

struct ImGuiPayload;
class ImGuiUtils
{
public:
	//Called in editor layer Initialize
	static void Initialize();
	static const ImGuiPayload* DragDropWindow(const char* aType, const std::string& aFilter = "", bool aAcceptPrimitives = false);
	static void PushID();
	static void PopID();
	static bool BeginPopupRect(const std::string& aName, const Utils::Vector2f& aTLScreenPos, const Utils::Vector2f& aBRScreenPos, ImGuiPopupFlags somePopupFlags = ImGuiPopupFlags_MouseButtonRight);

	static bool BeginParameters(const std::string& aName = "", bool aPushId = true, Ptr<Firefly::Component> aComponent = Ptr<Firefly::Component>(), UndoHandler* aUndoHandlerToUse = nullptr);
	static void EndParameters(bool aPopId = true);

	static void DrawParameterNameText(const std::string& aName);

	static bool HeaderParameter(const std::string& aName);
	static void EndHeaderParameter();

	static bool Button(const std::string& aName);
	//int
	static bool Parameter(const std::string& aName, int& aValue, float aIncrement = 1, int aMin = 0, int aMax = 0, const char* aToolTip = "", bool aDragInt = true);
	//float
	static bool Parameter(const std::string& aName, float& aValue, float aIncrement = 1.0f, float aMin = 0.0f, float aMax = 0.0f, const char* aToolTip = "");
	//string 
	static bool Parameter(const std::string& aName, std::string& aValue, const char* aToolTip = "");
	//bool
	static bool Parameter(const std::string& aName, bool& aValue, const char* aToolTip = "");
	//vec2
	static bool Parameter(const std::string& aName, Utils::Vector2f& aValue, float aIncrement = 1.0f, float aMin = 0.0f, float aMax = 0.0f, const char* aToolTip = "");
	//vec3
	static bool Parameter(const std::string& aName, Utils::Vector3f& aValue, float aIncrement = 1.0f, float aMin = 0.0f, float aMax = 0.0f, const char* aToolTip = "");
	//vec4
	static bool Parameter(const std::string& aName, Utils::Vector4f& aValue, float aIncrement = 1.0f, float aMin = 0.0f, float aMax = 0.0f, const char* aToolTip = "");
	//color
	static bool ColorParameter(const std::string& aName, Utils::Vector4f& aValue, bool aUseAlpha, const char* aToolTip = "");
	//button 
	static bool Parameter(const std::string& aName, std::function<void()> aFunctionToCall);
	//entity
	static bool Parameter(const std::string& aName, Ptr<Firefly::Entity>& aValue);
	//File
	static bool FileParameter(const std::string& aName, std::string& aValue, const std::string& aFilter);
	//enum
	static bool EnumParameter(const std::string& aName, uint32_t& aValue, std::vector<std::string> aEnumNames);
	//slider int
	static bool SliderParameter(const std::string& aName, int& aValue, int aMin, int aMax, const char* aToolTip = "");
	//slider float
	static bool SliderParameter(const std::string& aName, float& aValue, float aMin, float aMax, const char* aToolTip = "");
	//slider vec2
	static bool SliderParameter(const std::string& aName, Utils::Vector2f& aValue, float aMin, float aMax, const char* aToolTip = "");
	//slider vec3
	static bool SliderParameter(const std::string& aName, Utils::Vector3f& aValue, float aMin, float aMax, const char* aToolTip = "");
	//slider vec4
	static bool SliderParameter(const std::string& aName, Utils::Vector4f& aValue, float aMin, float aMax, const char* aToolTip = "");

	static bool Combo(const std::string& aName, uint32_t& aValue, std::vector<std::string> aComboOptions, int someFlags, bool aUseUndo = true);
	static bool DragFloat(const std::string& aName, float& aValue, float aIncrement = 1.0f, float aMin = 0.0f, float aMax = 0.0f, bool aUseUndo = true);
	static bool DragInt(const std::string& aName, int& aValue, float aIncrement = 1, int aMin = 0, int aMax = 0, bool aUseUndo = true);
	static bool SliderFloat(const std::string& aName, float& aValue, float aMin = 0.0f, float aMax = 1.0f, bool aUseUndo = true);
	static bool SliderInt(const std::string& aName, int& aValue, int aMin = 0, int aMax = 10, bool aUseUndo = true);
	static bool InputInt(const std::string& aName, int& aValue, int aStep = 1, int aStepFast = 10, bool aUseUndo = true);


	static bool Checkbox(const std::string& aName, bool& aValue, bool aUseUndo = true);

	static void SetAlignNamesToLeft(bool aFlag);

	static void PushFont(ImGuiUtilsFont_ aFont);
	static void PopFont();

	static void DrawlistAddLineArrow(ImDrawList* aDrawList, const Utils::Vector2f& aStart, const Utils::Vector2f& aEnd, ImU32 aColor, float aLineThickness = 1.f, float aArrowSize = 5.0f);

	static void OpenYesNoPopupModal(const std::string& aName, const std::string& aMessage, std::function<void()> aYesFunction, std::function<void()> aNoFunction);
	/// <summary>
	/// for use in EditorLayer Do not use in other places
	/// </summary>
	/// <returns></returns>
	static void UpdateYesNoPopupModal();

	static void ToolTip(const char* aMessage);

	static void ResizeWidget(const std::string& aLabel, float& aWidthToModify, float aMaxWidth = 0, bool aLeftIsBiggerFlag = false);

	template<typename... Args>
	static void NotifyInfo(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifySuccess(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyWarning(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyError(const char* aFormattedMessage, Args&&... someArgs);

	template<typename... Args>
	static void NotifyInfoLocal(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifySuccessLocal(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyWarningLocal(const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyErrorLocal(const char* aFormattedMessage, Args&&... someArgs);

	//custom
	template<typename... Args>
	static void NotifyInfoCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifySuccessCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyWarningCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args&&... someArgs);
	template<typename... Args>
	static void NotifyErrorCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args&&... someArgs);

	static std::string AddDotsIfMaxSize(const std::string& aEntry, float myMaxWidth);

	static bool IsScrollBarBeingUsed();


private:
	static void TrackValue(void* someData, int aSize);
	static void BeginValueTracker(void* someData, int aSize);
	static void EndValueTracker(void* someData, int aSize);
	static void TrackValue(std::string& someData);
	static void BeginValueTracker(std::string& someData);
	static void EndValueTracker(std::string& someData);

	static uint32_t myIDStack;
	static uint32_t myContextID;

	static inline void* myTrackedValue;
	static inline size_t myTrackedValueSize = 0;

	static inline std::string myTrackedValueString;

	static inline bool myIsTrackingValue = false;
	static inline bool myAlignNamesToLeft = false;

	static inline Ptr<Firefly::Component> myCurrentComponent = Ptr<Firefly::Component>();

	static inline bool myUsingResizeWidget = false;
	static inline float myResizeWidgetStartValue = 0.0f;
	static inline std::string myResizeWidgetValueToModify;

	struct ModalInfo
	{
		std::function<void()> myYesFunction;
		std::function<void()> myNoFunction;
		std::string myYesNoPopupMessage;
		std::string myYesNoPopupName;
	};
	static inline std::vector<ModalInfo> myModals;

	static inline std::array<ImFont*, ImGuiUtilsFont_Count> myFonts;

	static inline UndoHandler* ourCurrentUndoHandler = nullptr;
};


template<typename ... Args>
void ImGuiUtils::NotifyInfo(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyInfo(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifySuccess(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifySuccess(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifyWarning(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyWarning(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifyError(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyError(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifyInfoLocal(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyInfoLocal(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifySuccessLocal(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifySuccessLocal(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifyWarningLocal(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyWarningLocal(message.c_str());
}

template<typename ... Args>
void ImGuiUtils::NotifyErrorLocal(const char* aFormattedMessage, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));
	ImGui::NotifyErrorLocal(message.c_str());
}

template<typename ...Args>
inline void ImGuiUtils::NotifyInfoCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args && ...someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));

	ImGuiToast notification(ImGuiToastType_Info, 3000, message.c_str());
	notification.currentWindow = true;
	notification.windowPos = { aWindowPos.x, aWindowPos.y };
	notification.windowSize = { aWindowSize.x, aWindowSize.y };
	ImGui::InsertNotification(notification);
}

template<typename ...Args>
inline void ImGuiUtils::NotifySuccessCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args && ...someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));

	ImGuiToast notification(ImGuiToastType_Success, 3000, message.c_str());
	notification.currentWindow = true;
	notification.windowPos = { aWindowPos.x, aWindowPos.y };
	notification.windowSize = { aWindowSize.x, aWindowSize.y };
	ImGui::InsertNotification(notification);
}

template<typename ...Args>
inline void ImGuiUtils::NotifyWarningCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args && ...someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));

	ImGuiToast notification(ImGuiToastType_Warning, 3000, message.c_str());
	notification.currentWindow = true;
	notification.windowPos = { aWindowPos.x, aWindowPos.y };
	notification.windowSize = { aWindowSize.x, aWindowSize.y };
	ImGui::InsertNotification(notification);
}

template<typename ...Args>
inline void ImGuiUtils::NotifyErrorCustom(const Utils::Vector2f& aWindowPos, const Utils::Vector2f& aWindowSize, const char* aFormattedMessage, Args && ...someArgs)
{
	const std::string message = std::vformat(aFormattedMessage, std::make_format_args(someArgs...));

	ImGuiToast notification(ImGuiToastType_Error, 3000, message.c_str());
	notification.currentWindow = true;
	notification.windowPos = { aWindowPos.x, aWindowPos.y };
	notification.windowSize = { aWindowSize.x, aWindowSize.y };
	ImGui::InsertNotification(notification);
}

#ifndef IMGUIUTILS_NO_IMGUI_OPERATORS

inline ImVec2 operator+(const ImVec2& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x + aVec2.x, aVec1.y + aVec2.y };
}

inline ImVec2 operator-(const ImVec2& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x - aVec2.x, aVec1.y - aVec2.y };
}

inline ImVec2 operator*(const ImVec2& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x * aVec2.x, aVec1.y * aVec2.y };
}

inline ImVec2 operator/(const ImVec2& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x / aVec2.x, aVec1.y / aVec2.y };
}

inline ImVec2 operator*(const ImVec2& aVec1, const float aScalar)
{
	return { aVec1.x * aScalar, aVec1.y * aScalar };
}

inline ImVec2 operator/(const ImVec2& aVec1, const float aScalar)
{
	return { aVec1.x / aScalar, aVec1.y / aScalar };
}

inline ImVec2 operator*(const float aScalar, const ImVec2& aVec1)
{
	return { aVec1.x * aScalar, aVec1.y * aScalar };
}

inline ImVec2 operator/(const float aScalar, const ImVec2& aVec1)
{
	return { aVec1.x / aScalar, aVec1.y / aScalar };
}

inline ImVec2 operator+ (const ImVec2& aVec1, const Utils::Vector2f& aVec2)
{
	return { aVec1.x + aVec2.x, aVec1.y + aVec2.y };
}

inline Utils::Vector2f operator+ (const Utils::Vector2f& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x + aVec2.x, aVec1.y + aVec2.y };
}

inline ImVec2 operator- (const ImVec2& aVec1, const Utils::Vector2f& aVec2)
{
	return { aVec1.x - aVec2.x, aVec1.y - aVec2.y };
}

inline Utils::Vector2f operator- (const Utils::Vector2f& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x - aVec2.x, aVec1.y - aVec2.y };
}

inline ImVec2 operator* (const ImVec2& aVec1, const Utils::Vector2f& aVec2)
{
	return { aVec1.x * aVec2.x, aVec1.y * aVec2.y };
}

inline Utils::Vector2f operator* (const Utils::Vector2f& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x * aVec2.x, aVec1.y * aVec2.y };
}

inline ImVec2 operator/ (const ImVec2& aVec1, const Utils::Vector2f& aVec2)
{
	return { aVec1.x / aVec2.x, aVec1.y / aVec2.y };
}

inline Utils::Vector2f operator/ (const Utils::Vector2f& aVec1, const ImVec2& aVec2)
{
	return { aVec1.x / aVec2.x, aVec1.y / aVec2.y };
}
#endif ////IMGUIUTILS_NO_IMGUI_OPERATORS


inline ImVec2 UtilsVecToImGuiVec(const Utils::Vector2f& aVec)
{
	return { aVec.x, aVec.y };
}

inline Utils::Vector2f ImGuiVecToUtilsVec(const ImVec2& aVec)
{
	return { aVec.x, aVec.y };
}

inline ImVec4 UtilsVecToImGuiVec(const Utils::Vector4f& aVec)
{
	return { aVec.x, aVec.y, aVec.z, aVec.w };
}

inline Utils::Vector4f ImGuiVecToUtilsVec(const ImVec4& aVec)
{
	return { aVec.x, aVec.y, aVec.z, aVec.w };
}


