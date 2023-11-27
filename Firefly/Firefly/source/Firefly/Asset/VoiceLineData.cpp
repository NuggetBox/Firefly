#include "FFpch.h"
#include "VoiceLineData.h"

void Firefly::VoiceLineData::Init(std::unordered_map<std::string, std::vector<VoiceLine>> someSoundGroups)
{
	mySoundGroups = someSoundGroups;
}
