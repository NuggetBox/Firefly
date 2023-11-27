#include "DiscordAPI.h"
#include "discord.h"
#include <windows.h>
#include <iostream>
#include <chrono>

discord::Core* core{};

namespace Firefly::DiscordAPI
{
	discord::Activity activity{};
	discord::User userPtr;

	discord::Event eventHand;

	void Initialize()
	{
		auto result = discord::Core::Create(1106185554594381895, DiscordCreateFlags_NoRequireDiscord, &core);

		if (!IsValid())
		{
			return;
		}

		//core->SetLogHook(
		//	discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
		//		std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
		//	});

		core->UserManager().OnCurrentUserUpdate.Connect([]() 
			{
				core->UserManager().GetCurrentUser(&userPtr);
			});

		activity = {};
		activity.GetAssets().SetLargeImage("image");
		activity.GetAssets().SetLargeText("Firefly Engine");

		UpdatePressence();
	}

	void Update()
	{
		if (IsValid())
		{
			core->RunCallbacks();
		}
	}

	bool IsValid()
	{
		return core != nullptr;
	}

	void SetStartTimestampToNow()
	{
		auto now = std::chrono::system_clock::now();
		auto legacyStart = std::chrono::system_clock::to_time_t(now);

		activity.GetTimestamps().SetStart(legacyStart);
		UpdatePressence();
	}

	void SetStartTimestamp(float aTime)
	{
		activity.GetTimestamps().SetStart(aTime);
		UpdatePressence();
	}

	void SetEndTimestamp(float aTime)
	{
		activity.GetTimestamps().SetEnd(aTime);
		UpdatePressence();
	}

	void ResetStartTimestamp()
	{
		activity.GetTimestamps().SetStart(0);
		UpdatePressence();
	}

	void ResetEndTimestamp()
	{
		activity.GetTimestamps().SetEnd(0);
		UpdatePressence();
	}

	std::string_view GetDiscordUserName()
	{
		return userPtr.GetUsername();
	}

	void SetActivity(std::string_view aActivity)
	{
		activity.SetDetails(aActivity.data());
		UpdatePressence();
	}

	void SetStatus(std::string_view aStatus)
	{
		activity.SetState(aStatus.data());
		UpdatePressence();
	}

	void UpdatePressence()
	{
		if (IsValid())
		{
			core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
		}
	}
}
