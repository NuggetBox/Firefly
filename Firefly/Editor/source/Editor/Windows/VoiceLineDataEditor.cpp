#include "EditorPch.h"
#include "VoiceLineDataEditor.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/VoiceLineData.h"
#include "imgui/imgui_internal.h"
#include "FmodWrapper/AudioManager.h"

REGISTER_WINDOW(VoiceLineDataEditor);


VoiceLineDataEditor::VoiceLineDataEditor()
	: EditorWindow("VoiceLineDataEditor")
{
}

void VoiceLineDataEditor::OnImGui()
{
	if (!myVoiceLineDataRef || !myVoiceLineDataCopy)
	{
		ImGui::Text("No VoiceLineData loaded, load one by double clicking it in the ContentBrowser or drag and drop it here");
		if (auto* payload = ImGuiUtils::DragDropWindow("FILE", ".vld"))
		{
			const char* file = static_cast<const char*>(payload->Data);

			Load(file);
		}
		return;
	}

	auto& voiceLineGroups = myVoiceLineDataCopy->mySoundGroups;

	std::string removeKey = "";
	for (auto& voiceLineGroup : voiceLineGroups)
	{
		//collapsing header per group'
		bool voiceLineGroupOpen = ImGui::CollapsingHeader(voiceLineGroup.first.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);
		ImGui::SameLine();
		auto style = ImGui::GetStyle();
		auto rightOfGroupHeaderXPos = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
		auto removeGroupButtonTextSize = ImGui::CalcTextSize("X");
		ImVec2 removeGroupButtonSize = ImGui::CalcItemSize({ 0,0 }, removeGroupButtonTextSize.x + style.FramePadding.x * 2.0f, removeGroupButtonTextSize.y + style.FramePadding.y * 2.0f);
		ImGui::SetCursorPosX(rightOfGroupHeaderXPos - removeGroupButtonSize.x);
		if (ImGui::Button(("X##RemoveGroup" + voiceLineGroup.first).c_str()))
		{
			removeKey = voiceLineGroup.first;
		}
		if (voiceLineGroupOpen)
		{
			ImGui::Indent();
			int indexToRemoveAt = -1;
			int index = 0;
			//voice lines with collapsing header
			for (auto& voiceLine : voiceLineGroup.second)
			{
				bool voiceLineOpen = ImGui::CollapsingHeader(voiceLine.EventName.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);
				ImGui::SameLine();

				auto removeButtonTextSize = ImGui::CalcTextSize("X");
				ImVec2 removeButtonSize = ImGui::CalcItemSize({ 0,0 }, removeButtonTextSize.x + style.FramePadding.x * 2.0f, removeButtonTextSize.y + style.FramePadding.y * 2.0f);

				auto playSoundButtonTextSize = ImGui::CalcTextSize("Play");
				ImVec2 playSoundButtonSize = ImGui::CalcItemSize({ 0,0 }, playSoundButtonTextSize.x + style.FramePadding.x * 2.0f, playSoundButtonTextSize.y + style.FramePadding.y * 2.0f);

				auto rightOfHeaderXPos = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
				ImGui::SetCursorPosX(rightOfHeaderXPos - removeButtonSize.x);
				if (ImGui::Button(("X##" + voiceLine.EventName).c_str()))
				{
					indexToRemoveAt = index;
				}
				ImGui::SameLine();
				ImGui::SetCursorPosX(rightOfHeaderXPos - removeButtonSize.x - playSoundButtonSize.x);
				if (ImGui::Button(("Play##" + voiceLine.EventName).c_str()))
				{
					AudioManager::PlayEventOneShot(voiceLine.EventName);
				}

				if (voiceLineOpen)
				{
					ImGui::Indent();
					for (int i = 0; i < voiceLine.Subtitle.size(); i++)
					{
						if (ImGui::Button("X", { 20.f,0 }))
						{
							voiceLine.Subtitle.erase(voiceLine.Subtitle.begin() + i);
							voiceLine.SubtitleTime.erase(voiceLine.SubtitleTime.begin() + i);
							i--;
							continue;
						}
						ImGui::SameLine();
						ImGui::InputText(("Subtitle##" + voiceLine.EventName + std::to_string(i)).c_str(), &voiceLine.Subtitle[i]);

						ImGui::Indent();
						if (i < voiceLine.SubtitleTime.size())
						{
							float min = 0;
							if (i > 0)
							{
								min = voiceLine.SubtitleTime[i - 1];
							}
							if (voiceLine.SubtitleTime[i] < min)
							{
								voiceLine.SubtitleTime[i] = min;
							}
							ImGui::DragFloat(("Time: ##" + voiceLine.EventName + std::to_string(i)).c_str(), &voiceLine.SubtitleTime[i]);
						}
						else
						{
							voiceLine.SubtitleTime.push_back(0);
						}
						ImGui::Unindent();
						ImGui::Separator();
					}

					if (ImGui::Button(("Add Sub ##" + voiceLine.EventName).c_str()))
					{
						voiceLine.Subtitle.push_back("");
						voiceLine.SubtitleTime.push_back(0);
					}
					ImGui::Unindent();
				}
				index++;
			}

			if (indexToRemoveAt != -1)
			{
				voiceLineGroup.second.erase(voiceLineGroup.second.begin() + indexToRemoveAt);
			}

			//add buttonImGui
			ImGui::InputText(("##AddEventInputText" + voiceLineGroup.first).c_str(), &myInputTexts[voiceLineGroup.first]);
			ImGui::SameLine();
			if (ImGui::Button(("Add Voiceline##" + voiceLineGroup.first).c_str()))
			{
				//check if the event already exists
				bool eventExists = false;
				for (auto& voiceLine : voiceLineGroup.second)
				{
					if (voiceLine.EventName == myInputTexts[voiceLineGroup.first])
					{
						eventExists = true;
						break;
					}
				}

				if (!eventExists)
				{
					if (!myInputTexts[voiceLineGroup.first].empty())
					{
						if (myInputTexts[voiceLineGroup.first].starts_with("event:"))
						{
							voiceLineGroup.second.emplace_back();
							voiceLineGroup.second.back().EventName = myInputTexts[voiceLineGroup.first];
							myInputTexts[voiceLineGroup.first].clear();
						}
						else
						{
							ImGuiUtils::NotifyErrorLocal("You have to have event: in front of the event name. Use the copy path button in FMOD!");

						}
					}
					else
					{
						ImGuiUtils::NotifyErrorLocal("You cannot add a voiceline with no name!");
					}
				}
				else
				{
					ImGuiUtils::NotifyErrorLocal("You cannot add a event that already exists in the group!");
				}
			}

			ImGui::Unindent();

		}
	}

	ImGui::InputText("##VOiceLineDataEditorAddgroupInputText", &myAddGroupInputText);
	ImGui::SameLine();
	if (ImGui::Button("Add Group##VOiceLineDataEditorAddgroup"))
	{
		if (!voiceLineGroups.contains(myAddGroupInputText))
		{
			if (!myAddGroupInputText.empty())
			{
				voiceLineGroups.insert({ myAddGroupInputText, {} });
				myAddGroupInputText.clear();
			}
			else
			{
				ImGuiUtils::NotifyErrorLocal("You cannot add a group with no name!");
			}
		}
		else
		{
			ImGuiUtils::NotifyErrorLocal("You cannot add a group with the same name as another group!");
		}
	}

	if (!removeKey.empty())
	{
		voiceLineGroups.erase(removeKey);
	}



	if (ImGui::Button("Save"))
	{
		Save();
	}


}

void VoiceLineDataEditor::Save()
{
	std::ofstream file(myVoiceLineDataRef->GetPath());
	if (!file.is_open())
	{
		ImGuiUtils::NotifyErrorLocal("Could not save VoiceLineData to path \"{}\"\n File could not be opened for write, make sure it is checked out in perforce!", myVoiceLineDataRef->GetPath().string());
		return;
	}
	nlohmann::json json;

	auto& voiceLineGrupsJson = json["VoiceLineGroups"];
	int voiceLineGroupIndex = 0;
	for (auto& voiceLineGroup : myVoiceLineDataCopy->GetSoundGroupsMutable())
	{
		voiceLineGrupsJson[voiceLineGroupIndex]["Name"] = voiceLineGroup.first;
		auto& eventsJson = voiceLineGrupsJson[voiceLineGroupIndex]["Events"];
		int eventIndex = 0;
		for (auto& ev : voiceLineGroup.second)
		{
			eventsJson[eventIndex]["Name"] = ev.EventName;
			for (int i = 0; i < ev.Subtitle.size(); i++)
			{
				eventsJson[eventIndex]["Subtitles"][i]["Sub"] = ev.Subtitle[i];
				if (i >= ev.SubtitleTime.size())
				{
					ev.SubtitleTime.push_back(i == 0 ? 0 : ev.SubtitleTime[i - 1]);
				}
				eventsJson[eventIndex]["Subtitles"][i]["Time"] = ev.SubtitleTime[i];

			}
			eventIndex++;
		}
		voiceLineGroupIndex++;
	}

	file << std::setw(4) << json;

	if (myVoiceLineDataRef)
	{
		*myVoiceLineDataRef = *myVoiceLineDataCopy;
	}

	ImGuiUtils::NotifySuccessLocal("Successfully saved voice line data to path \"{}\"\n!", myVoiceLineDataRef->GetPath().string());
}

void VoiceLineDataEditor::Load(const std::filesystem::path& aPath)
{
	myVoiceLineDataRef = Firefly::ResourceCache::GetAsset<Firefly::VoiceLineData>(aPath, true);
	myVoiceLineDataCopy = std::make_shared<Firefly::VoiceLineData>(*myVoiceLineDataRef);
}
