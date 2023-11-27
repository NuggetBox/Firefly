#pragma once
#include <string>
namespace Firefly
{
	namespace DiscordAPI
	{
		void Initialize();
		void Update();

		bool IsValid();

		void SetStartTimestampToNow();

		void SetStartTimestamp(float aTime);
		void SetEndTimestamp(float aTime);

		void ResetStartTimestamp();
		void ResetEndTimestamp();

		std::string_view GetDiscordUserName();

		void SetActivity(std::string_view aActivity);
		void SetStatus(std::string_view aStatus);
		void UpdatePressence();
	};
}

