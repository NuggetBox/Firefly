#include "FFpch.h"
#include "VoiceLineDataImporter.h"
#include "Firefly/Asset/VoiceLineData.h"
#include "nlohmann/json.hpp"

bool Firefly::VoiceLineDataImporter::ImportVoiceLineData(Ref<VoiceLineData> aVoiceLineData)
{
	std::ifstream file(aVoiceLineData->GetPath());
	if (!file.is_open())
	{
		LOGERROR("File could not be opened! Path: {}", aVoiceLineData->GetPath().string());
		return false;
	}
	nlohmann::json json;
	file >> json;
	file.close();

	std::unordered_map<std::string, std::vector<VoiceLine>> voiceLineGroups;

	auto& voiceLinesGroupJsonArr = json["VoiceLineGroups"];
	for (auto& voiceLinesGroupJson : voiceLinesGroupJsonArr)
	{
		std::string groupName = voiceLinesGroupJson["Name"].get<std::string>();
		std::vector<VoiceLine> voiceLines;
		auto& eventsJsonArr = voiceLinesGroupJson["Events"];
		for (auto& eventJson : eventsJsonArr)
		{
			VoiceLine voiceLine;
			voiceLine.EventName = eventJson["Name"];
			if (eventJson.contains("Subtitle"))
			{
				voiceLine.Subtitle.push_back(eventJson["Subtitle"]);
				voiceLine.SubtitleTime.push_back(0);

			}
			else if (eventJson.contains("Subtitles"))
			{
				auto subsArr = eventJson["Subtitles"];
				for (int i = 0; i < subsArr.size(); i++)
				{
					if (subsArr[i]["Sub"].is_array())
					{
						voiceLine.Subtitle.push_back(subsArr[i]["Sub"][0]);
					}
					else
					{
						voiceLine.Subtitle.push_back(subsArr[i]["Sub"]);
					}
					voiceLine.SubtitleTime.push_back(subsArr[i]["Time"]);
				}
			}
			voiceLines.push_back(voiceLine);
		}

		voiceLineGroups.emplace(groupName, voiceLines);
	}
	aVoiceLineData->Init(voiceLineGroups);

	return true;
}
