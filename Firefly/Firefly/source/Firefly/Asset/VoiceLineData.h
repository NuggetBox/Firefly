#pragma once
#include "Firefly/Asset/Asset.h"

class VoiceLineDataEditor;

namespace Firefly
{
	struct VoiceLine
	{
		std::string EventName;
		std::vector<std::string> Subtitle;
		std::vector<float> SubtitleTime;
	};

	class VoiceLineData : public Asset
	{
	public:
		VoiceLineData() = default;
		~VoiceLineData() = default;
		static AssetType GetStaticType() { return AssetType::VoiceLineData; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		void Init(std::unordered_map<std::string, std::vector<VoiceLine>> someSoundGroups);

		const std::vector<VoiceLine>& GetVoiceLines(const std::string& aSoundGroup) const { return mySoundGroups.at(aSoundGroup); }

		const std::unordered_map<std::string, std::vector<VoiceLine>>& GetSoundGroups() const { return mySoundGroups; }
		 std::unordered_map<std::string, std::vector<VoiceLine>>& GetSoundGroupsMutable()  { return mySoundGroups; }
	private:
		friend class ::VoiceLineDataEditor;
		std::unordered_map<std::string, std::vector<VoiceLine>> mySoundGroups;

	};
}