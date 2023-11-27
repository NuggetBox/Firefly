#include "EditorPch.h"
#include "ImGuiUtils.h"
#include "imgui/imgui_internal.h"

#include "Editor/EditorLayer.h"
#include "Editor/UndoSystem/UndoHandler.h"
#include "Editor/UndoSystem/Commands/EntityCommands/ComponentParameterChangedCommand.h"
#include "Editor/UndoSystem/Commands/EntityCommands/ComponentStringParameterChanged.h"
#include "Editor/Event/EditorOnlyEvents.h"

#include "Editor/UndoSystem/UndoHandler.h"

#include "Editor/Windows/ContentBrowser.h"

#include "Firefly/Application/Application.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/Component.h"

#include "Utils/StringUtils.hpp"

uint32_t ImGuiUtils::myIDStack = 0;
uint32_t ImGuiUtils::myContextID = 0;

void ImGuiUtils::Initialize()
{
	myFonts[ImGuiUtilsFont_Roboto_10] = ImGui::GetIO().Fonts->AddFontFromFileTTF("Editor/Fonts/Roboto.ttf", 10.0f);
	myFonts[ImGuiUtilsFont_Roboto_16] = ImGui::GetIO().Fonts->AddFontFromFileTTF("Editor/Fonts/Roboto.ttf", 16.0f);
	myFonts[ImGuiUtilsFont_RobotoBold_10] = ImGui::GetIO().Fonts->AddFontFromFileTTF("Editor/Fonts/Roboto-Bold.ttf", 10.0f);
	myFonts[ImGuiUtilsFont_RobotoBold_16] = ImGui::GetIO().Fonts->AddFontFromFileTTF("Editor/Fonts/Roboto-Bold.ttf", 16.0f);
}

const ImGuiPayload* ImGuiUtils::DragDropWindow(const char* aType, const std::string& aFilter, bool aAcceptPrimitives)
{
	const ImGuiPayload* payload = ImGui::GetDragDropPayload();
	if (payload)
	{
		//if type is FILE and we want to only accept with a certain file extension
		if (aType == "FILE" && aFilter != "")
		{
			std::filesystem::path path = (const char*)payload->Data;

			bool dontReturn = false;

			if (aAcceptPrimitives)
			{
				if (path.string() == "Cube" || path.string() == "Plane" || path.string() == "Pyramid"
					|| path.string() == "LDCube" || path.string() == "Triangle" || path.string() == "Cylinder")
				{
					dontReturn = true;
				}
			}

			if (!dontReturn)
			{
				if (path.extension() != aFilter)
				{
					return nullptr;
				}
			}
		}

		if (payload->IsDataType(aType))
		{
			auto windowPos = ImGui::GetWindowPos();
			ImRect windowRect = { windowPos.x, windowPos.y,
				windowPos.x + ImGui::GetWindowContentRegionMax().x, windowPos.y + ImGui::GetWindowContentRegionMax().y };

			ImGui::PushClipRect({ windowRect.Min.x - 10 ,windowRect.Min.y - 10 }, { windowRect.Max.x + 10 ,windowRect.Max.y + 10 }, false);
			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetID(("WindowDragDropTarget" + std::to_string(ImGui::GetCurrentWindow()->ID)).c_str())))
			{
				if (ImGui::AcceptDragDropPayload(aType))
				{
					return payload;
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopClipRect();
		}
	}
	return nullptr;
}

void ImGuiUtils::PushID()
{
	myContextID++;
	ImGui::PushID(myContextID);
	myIDStack = 0;
}
void ImGuiUtils::PopID()
{

	myContextID--;
	ImGui::PopID();
}
bool ImGuiUtils::BeginPopupRect(const std::string& aName, const Utils::Vector2f& aTLScreenPos, const Utils::Vector2f& aBRScreenPos, ImGuiPopupFlags somePopupFlags)
{
	ImGuiID id = ImGui::GetID(aName.c_str());
	ImRect itemRect = ImRect(ImVec2(aTLScreenPos.x, aTLScreenPos.y), ImVec2(aBRScreenPos.x, aBRScreenPos.y));
	int mouse_button = (somePopupFlags & ImGuiPopupFlags_MouseButtonMask_);
	if (ImGui::IsMouseReleased(mouse_button) && ImGui::IsMouseHoveringRect(ImVec2(aTLScreenPos.x, aTLScreenPos.y), ImVec2(aBRScreenPos.x, aBRScreenPos.y), false))
		ImGui::OpenPopupEx(id, somePopupFlags);
	return ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}
bool ImGuiUtils::BeginParameters(const std::string& aName, bool aPushId, Ptr<Firefly::Component> aComponent, UndoHandler* aUndoHandlerToUse)
{
	ourCurrentUndoHandler = aUndoHandlerToUse;
	if (aPushId)
	{
		PushID();
	}
	auto returnBool = ImGui::BeginTable(aName.c_str(), 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadInnerX);
	myCurrentComponent = aComponent;
	return returnBool;
}
void ImGuiUtils::EndParameters(bool aPopId)
{
	ourCurrentUndoHandler = nullptr;
	myCurrentComponent = Ptr<Firefly::Component>();
	ImGui::EndTable();
	if (aPopId)
	{
		PopID();
	}
}
void ImGuiUtils::DrawParameterNameText(const std::string& aName)
{
	if (myAlignNamesToLeft)
	{
		auto avail = ImGui::GetContentRegionAvail();
		ImGui::SetCursorPosX(avail.x - ImGui::CalcTextSize(aName.c_str()).x);
	}
	ImGui::TextUnformatted(aName.c_str());
}
bool ImGuiUtils::HeaderParameter(const std::string& aName)
{
	ImGui::TableNextColumn();
	bool open = ImGui::TreeNodeEx((aName + "##").c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Framed);
	ImGui::TreePush();
	ImGui::TableNextColumn();
	ImGui::Text("");
	return open;
}
void ImGuiUtils::EndHeaderParameter()
{
	ImGui::TreePop();
}
bool ImGuiUtils::Button(const std::string& aName)
{
	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool pressed = ImGui::Button((aName + id).c_str());
	return pressed;
}

bool ImGuiUtils::Parameter(const std::string& aName, int& aValue, float aIncrement, int aMin, int aMax, const char* aToolTip, bool aDragInt)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	ImGui::TableNextColumn();

	bool changed = false;

	if (aDragInt)
	{
		changed = DragInt(id, aValue, aIncrement, aMin, aMax);
	}
	else
	{
		changed = InputInt(id, aValue, aIncrement);
	}

	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, float& aValue, float aIncrement, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = DragFloat(id, aValue, aIncrement, aMin, aMax);

	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, std::string& aValue, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::InputText(id.c_str(), &aValue);
	//TODO: DO FOR STRING
	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, bool& aValue, const char* aToolTip)
{
	//checkbox
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::Checkbox(id.c_str(), &aValue);
	TrackValue(&aValue, sizeof(bool));
	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, Utils::Vector4f& aValue, float aIncrement, float aMin, float aMax, const char* aToolTip)
{

	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::DragFloat4(id.c_str(), &aValue.x, aIncrement, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector4f));

	return changed;


}

bool ImGuiUtils::Parameter(const std::string& aName, Utils::Vector3f& aValue, float aIncrement, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::DragFloat3(id.c_str(), &aValue.x, aIncrement, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector3f));
	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, Utils::Vector2f& aValue, float aIncrement, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::DragFloat2(id.c_str(), &aValue.x, aIncrement, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector2f));
	return changed;
}

bool ImGuiUtils::ColorParameter(const std::string& aName, Utils::Vector4f& aValue, bool aUseAlpha, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = false;
	if (aUseAlpha)
	{
		changed = ImGui::ColorEdit4(id.c_str(), &aValue.x);
	}
	else
	{
		changed = ImGui::ColorEdit3(id.c_str(), &aValue.x);
	}
	TrackValue(&aValue, sizeof(Utils::Vector4f));
	return changed;
}

bool ImGuiUtils::Parameter(const std::string& aName, std::function<void()> aFunctionToCall)
{
	bool pressed = ImGuiUtils::Button(aName.c_str());
	if (pressed)
	{
		aFunctionToCall();
	}
	return pressed;
}

bool ImGuiUtils::Parameter(const std::string& aName, Ptr<Firefly::Entity>& aValue)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);
	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = false;

	ImGui::BeginDisabled();
	std::string displayText = aValue.lock() ? aValue.lock()->GetName() : "Empty";

	ImGui::InputText(id.c_str(), &displayText);

	ImGui::EndDisabled();
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Ptr<Firefly::Entity> entity = *((Ptr<Firefly::Entity>*)payload->Data);
			BeginValueTracker(&aValue, sizeof(Ptr<Firefly::Entity>));
			aValue = entity;
			EndValueTracker(&aValue, sizeof(Ptr<Firefly::Entity>));

			changed = true;
		}
		ImGui::EndDragDropTarget();
	}
	return changed;
}

bool ImGuiUtils::FileParameter(const std::string& aName, std::string& aValue, const std::string& aFilter)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);
	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = false;

	std::string displayValue = "None";
	if (aValue != "")
	{
		displayValue = aValue;
	}
	auto it = displayValue.find_last_of('\\');
	if (it != std::string::npos)
	{
		displayValue = displayValue.substr(it + 1);
	}

	if (ImGui::Button((displayValue + id).c_str()))
	{
		if (!aValue.empty())
		{
			SelectInContentBrowserEvent ev(aValue);
			Firefly::Application::Get().OnEvent(ev);
		}
		else
		{
			SearchInContentBrowserEvent ev("t:" + aFilter.substr(1));
			Firefly::Application::Get().OnEvent(ev);
		}
	}

	std::vector<std::string> extensionsFilter;
	{
		//format is ".extension1;.extension2"
		std::string extensionFilter = aFilter;
		std::string extensionTemp;
		for (int i = 0; i < extensionFilter.size(); i++)
		{
			if (extensionFilter[i] == ';')
			{
				extensionsFilter.push_back(extensionTemp);
				extensionTemp = "";
			}
			else
			{
				extensionTemp += extensionFilter[i];
			}
		}
		extensionsFilter.push_back(extensionTemp);
	}


	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		auto contentBrowser = EditorLayer::GetWindow<ContentBrowser>();
		auto entry = contentBrowser->GetEntryFromPath(aValue);
		if (entry)
		{
			EditorLayer::GetWindow<ContentBrowser>()->OnOpenEntry(*entry);
		}
		else
		{
			ImGuiUtils::NotifyWarningLocal("Could not find Entry");
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		if (payload->IsDataType("FILE"))
		{
			std::filesystem::path file = (const char*)(payload->Data);

			bool acceptOverrule = false;

			//Accept primitives
			if (aFilter == ".fbx")
			{
				//TODO: Update primitives when creating new ones
				if (file == "Cube" || file == "Plane" || file == "Pyramid" || file == "LDCube" || file == "Triangle" || file == "Cylinder")
				{
					acceptOverrule = true;
				}
			}

			if (acceptOverrule || std::find(extensionsFilter.begin(), extensionsFilter.end(), file.extension()) != extensionsFilter.end() || aFilter == "")
			{
				if (ImGui::AcceptDragDropPayload("FILE"))
				{
					BeginValueTracker(aValue);
					aValue = file.string();
					EndValueTracker(aValue);
					changed = true;
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	static std::string searchString = "";
	if (ImGui::Button(("O##Select Asset Popup" + id).c_str()))
	{
		searchString = "";
	}
	if (ImGui::IsPopupOpen(ImGui::GetItemID(), 0))
	{
		ImGui::SetNextWindowSize({ ImGui::GetWindowWidth(), 300 });
	}
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
	{
		ImGui::InputTextWithHint("##PickAssetSearchBar", "Search...", &searchString);
		ImGui::BeginChild("##ImGuiUtilsPickAssetChild");
		//collect all files with the given filter
		auto lowerSearchString = Utils::ToLower(searchString);
		auto contentBrowser = EditorLayer::GetWindow<ContentBrowser>();
		std::vector<EntryType> entryTypeFilters;
		for (int i = 0; i < extensionsFilter.size(); i++)
		{
			entryTypeFilters.push_back(ContentBrowser::GetEntryTypeFromExtension(extensionsFilter[i]));
		}
		auto files = contentBrowser->GetAllEntriesOfTypes(entryTypeFilters);
		Entry none;
		none.Name = "None##NoneEntry";
		files.insert(files.begin(), &none);
		int fileCount = 0;
		for (auto& file : files)
		{
			if (Utils::ToLower(file->Name).find(lowerSearchString) == std::string::npos)
			{
				continue;
			}
			if (ImGui::MenuItem((file->Name + "##" + std::to_string(fileCount)).c_str()))
			{
				BeginValueTracker(aValue);
				if (file->Name == none.Name)
				{
					aValue = "";
				}
				else
				{
					aValue = file->Path.string();
				}
				EndValueTracker(aValue);
				changed = true;
				ImGui::CloseCurrentPopup();
			}
			ToolTip(file->Path.string().c_str());

			++fileCount;
		}
		if (fileCount == 0)
		{
			ImGui::Text("No Matches!");
		}
		ImGui::EndChild();
		ImGui::EndPopup();
	}

	return changed;
}

bool ImGuiUtils::EnumParameter(const std::string& aName, uint32_t& aValue, std::vector<std::string> aEnumNames)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);
	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = Combo(id, aValue, aEnumNames, 0);
	return changed;
}

bool ImGuiUtils::SliderParameter(const std::string& aName, int& aValue, int aMin, int aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = SliderInt(id, aValue, aMin, aMax);

	return changed;
}

bool ImGuiUtils::SliderParameter(const std::string& aName, float& aValue, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = SliderFloat(id, aValue, aMin, aMax);

	return changed;
}

bool ImGuiUtils::SliderParameter(const std::string& aName, Utils::Vector2f& aValue, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::SliderFloat2(id.c_str(), &aValue.x, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector2f));
	return changed;
}

bool ImGuiUtils::SliderParameter(const std::string& aName, Utils::Vector3f& aValue, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::SliderFloat3(id.c_str(), &aValue.x, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector3f));
	return changed;
}

bool ImGuiUtils::SliderParameter(const std::string& aName, Utils::Vector4f& aValue, float aMin, float aMax, const char* aToolTip)
{
	ImGui::TableNextColumn();
	DrawParameterNameText(aName);

	if (aToolTip != "")
	{
		ToolTip(aToolTip);
	}

	std::string id = "##" + std::to_string(myIDStack++);
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	bool changed = ImGui::SliderFloat4(id.c_str(), &aValue.x, aMin, aMax);
	TrackValue(&aValue, sizeof(Utils::Vector4f));
	return changed;
}

bool ImGuiUtils::Combo(const std::string& aName, uint32_t& aValue, std::vector<std::string> aComboOptions, int someFlags, bool aUseUndo)
{
	std::string id = aName;
	bool changed = false;
	if (ImGui::BeginCombo(id.c_str(), aComboOptions[aValue].c_str(), someFlags))
	{
		for (uint32_t i = 0; i < aComboOptions.size(); ++i)
		{
			bool isSelected = aValue == i;
			if (ImGui::Selectable(aComboOptions[i].c_str(), isSelected))
			{
				if (aUseUndo)
				{
					BeginValueTracker(&aValue, sizeof(uint32_t));
				}
				aValue = i;
				changed = true;
				if (aUseUndo)
				{
					EndValueTracker(&aValue, sizeof(uint32_t));
				}
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return changed;
}

bool ImGuiUtils::DragInt(const std::string& aName, int& aValue, float aIncrement, int aMin, int aMax, bool aUseUndo)
{
	bool changed = ImGui::DragInt(aName.c_str(), &aValue, aIncrement, aMin, aMax);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(int));
	}

	return changed;
}
bool ImGuiUtils::SliderFloat(const std::string& aName, float& aValue, float aMin, float aMax, bool aUseUndo)
{
	bool changed = ImGui::SliderFloat(aName.c_str(), &aValue, aMin, aMax);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(float));
	}
	return changed;
}
bool ImGuiUtils::SliderInt(const std::string& aName, int& aValue, int aMin, int aMax, bool aUseUndo)
{
	bool changed = ImGui::SliderInt(aName.c_str(), &aValue, aMin, aMax);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(int));
	}

	return changed;
}

bool ImGuiUtils::InputInt(const std::string& aName, int& aValue, int aStep, int aStepFast, bool aUseUndo)
{
	bool changed = ImGui::InputInt(aName.c_str(), &aValue, aStep, aStepFast);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(int));
	}

	return changed;
}

bool ImGuiUtils::DragFloat(const std::string& aName, float& aValue, float aIncrement, float aMin, float aMax, bool aUseUndo)
{
	bool changed = ImGui::DragFloat(aName.c_str(), &aValue, aIncrement, aMin, aMax);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(float));
	}
	return changed;
}


bool ImGuiUtils::Checkbox(const std::string& aName, bool& aValue, bool aUseUndo)
{
	bool changed = ImGui::Checkbox(aName.c_str(), &aValue);
	if (aUseUndo)
	{
		TrackValue(&aValue, sizeof(bool));
	}
	return changed;
}

void ImGuiUtils::SetAlignNamesToLeft(bool aFlag)
{
	myAlignNamesToLeft = aFlag;
}

void ImGuiUtils::PushFont(ImGuiUtilsFont_ aFont)
{
	ImGui::PushFont(myFonts[aFont]);
}

void ImGuiUtils::PopFont()
{
	ImGui::PopFont();
}

void ImGuiUtils::DrawlistAddLineArrow(ImDrawList* aDrawList, const Utils::Vector2f& aStart, const Utils::Vector2f& aEnd, ImU32 aColor, float aLineThickness, float aArrowSize)
{
	const Utils::Vector2f dir = (aEnd - aStart).GetNormalized();
	const Utils::Vector2f perpClockwise = { dir.y, -dir.x };
	const Utils::Vector2f perpCounterClockwise = { -dir.y, dir.x };
	const float arrowAngleRad = -PI / 4.0f;

	const Utils::Vector2f arrowDir1 =
	{ std::cos(arrowAngleRad) * perpClockwise.x - std::sin(arrowAngleRad) * perpClockwise.y,
		std::sin(arrowAngleRad) * perpClockwise.x + std::cos(arrowAngleRad) * perpClockwise.y };

	const Utils::Vector2f arrowDir2 =
	{ std::cos(-arrowAngleRad) * perpCounterClockwise.x - std::sin(-arrowAngleRad) * perpCounterClockwise.y,
	std::sin(-arrowAngleRad) * perpCounterClockwise.x + std::cos(-arrowAngleRad) * perpCounterClockwise.y };

	const ImVec2 arrowEnd1 = { aEnd.x + arrowDir1.x * aArrowSize, aEnd.y + arrowDir1.y * aArrowSize };
	const ImVec2 arrowEnd2 = { aEnd.x + arrowDir2.x * aArrowSize, aEnd.y + arrowDir2.y * aArrowSize };

	aDrawList->AddLine({ aStart.x, aStart.y }, { aEnd.x, aEnd.y }, aColor, aLineThickness);
	aDrawList->AddLine({ aEnd.x, aEnd.y }, arrowEnd1, aColor, aLineThickness);
	aDrawList->AddLine({ aEnd.x, aEnd.y }, arrowEnd2, aColor, aLineThickness);
}

void ImGuiUtils::OpenYesNoPopupModal(const std::string& aName, const std::string& aMessage, std::function<void()> aYesFunction, std::function<void()> aNoFunction)
{
	myModals.emplace_back(aYesFunction,
		aNoFunction,
		aMessage,
		aName);
}

void ImGuiUtils::UpdateYesNoPopupModal()
{
	if (myModals.size() > 0)
	{
		ImGui::OpenPopup((myModals.begin()->myYesNoPopupName + "##YesNoPopupModal").c_str());
		if (ImGui::BeginPopupModal((myModals.begin()->myYesNoPopupName + "##YesNoPopupModal").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::PushTextWrapPos(500);
			ImGui::TextWrapped(myModals.begin()->myYesNoPopupMessage.c_str());
			ImGui::PopTextWrapPos();
			ImGui::Separator();
			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				if (myModals.begin()->myYesFunction)
				{
					myModals.begin()->myYesFunction();
					myModals.erase(myModals.begin());
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				if (myModals.begin()->myNoFunction)
				{
					myModals.begin()->myNoFunction();
					myModals.erase(myModals.begin());

				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}

void ImGuiUtils::ToolTip(const char* aMessage)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(aMessage);
		ImGui::EndTooltip();
	}
}

void ImGuiUtils::ResizeWidget(const std::string& aLabel, float& aWidthToModify, float aMaxWidth, bool aLeftIsBiggerFlag)
{
	if (aMaxWidth == 0)
	{
		aMaxWidth = ImGui::GetContentRegionMax().x - 20;
	}
	ImGui::Button(("##" + aLabel).c_str(), ImVec2(5, ImGui::GetContentRegionAvail().y));
	if (aLabel == myResizeWidgetValueToModify || myResizeWidgetValueToModify == "")
	{

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			myUsingResizeWidget = true;
			myResizeWidgetStartValue = aWidthToModify;
			myResizeWidgetValueToModify = aLabel;
		}
		if (myUsingResizeWidget)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				float mul = 1;
				if (aLeftIsBiggerFlag)
				{
					mul = -1;
				}
				aWidthToModify = myResizeWidgetStartValue + ImGui::GetMouseDragDelta().x * mul;
			}
			else
			{
				myUsingResizeWidget = false;
				myResizeWidgetValueToModify = "";
			}
		}
	}
	if (aWidthToModify < 10)
	{
		aWidthToModify = 10;
	}
	else if (aWidthToModify > aMaxWidth)
	{
		aWidthToModify = aMaxWidth;
	}

}

std::string ImGuiUtils::AddDotsIfMaxSize(const std::string& aText, float myMaxWidth)
{
	auto displayText = aText;
	auto textSize = ImGui::CalcTextSize(displayText.c_str());
	bool isTooLong = textSize.x > myMaxWidth;
	while (textSize.x > myMaxWidth)
	{
		displayText = displayText.substr(0, displayText.size() - 4);
		displayText += "...";
		textSize = ImGui::CalcTextSize(displayText.c_str());
		if (displayText == "...")
		{
			break;
		}
	}
	return displayText;
}

bool ImGuiUtils::IsScrollBarBeingUsed()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiID active_id = ImGui::GetActiveID();

	if (active_id && (active_id == ImGui::GetWindowScrollbarID(window, ImGuiAxis_X) || active_id == ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y)))
	{
		return true;
	}
	else if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0.0f)
	{
		return true;
	}
	return false;
}

void ImGuiUtils::TrackValue(void* someData, int aSize)
{
	if (!myIsTrackingValue && ImGui::IsItemActivated())
	{
		BeginValueTracker(someData, aSize);
	}
	if (myIsTrackingValue && ImGui::IsItemDeactivatedAfterEdit())
	{
		EndValueTracker(someData, aSize);
	}
}

void ImGuiUtils::BeginValueTracker(void* someData, int aSize)
{
	myTrackedValue = new char[aSize];
	memcpy_s(myTrackedValue, aSize, someData, aSize);
	myTrackedValueSize = aSize;
	myIsTrackingValue = true;
}

void ImGuiUtils::EndValueTracker(void* someData, int aSize)
{
	if (ourCurrentUndoHandler != nullptr)
	{
		//create undo command and pass in tracked value pointer
		ourCurrentUndoHandler->AddUndo(std::make_shared<ComponentParameterChangedCommand>(myTrackedValue, someData, aSize, myCurrentComponent));

		myTrackedValue = nullptr;
		myIsTrackingValue = false;
	}
	else
	{
		LOGWARNING("ImGuiUtils::EndValueTracker: No Undo Handler given");
	}
}

void ImGuiUtils::TrackValue(std::string& someData)
{
	if (!myIsTrackingValue && ImGui::IsItemActivated())
	{
		BeginValueTracker(someData);
	}
	if (myIsTrackingValue && ImGui::IsItemDeactivatedAfterEdit())
	{
		EndValueTracker(someData);
	}
}

void ImGuiUtils::BeginValueTracker(std::string& someData)
{
	myTrackedValueString = someData;
	myIsTrackingValue = true;
}

void ImGuiUtils::EndValueTracker(std::string& someData)
{
	if (ourCurrentUndoHandler != nullptr)
	{
		//create undo command and pass in tracked value pointer
		ourCurrentUndoHandler->AddUndo(std::make_shared<ComponentStringParameterChangedCommand>(someData, myTrackedValueString, myCurrentComponent));
		myTrackedValueString = "";
		myIsTrackingValue = false;
	}
	else
	{
		LOGWARNING("ImGuiUtils::EndValueTracker(std::string): No Undo Handler given");
	}
}