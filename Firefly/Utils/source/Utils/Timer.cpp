#include "Timer.h"
#include "Math/Random.hpp"

namespace Utils
{
	void Timer::Start()
	{
		ourStartTime = std::chrono::high_resolution_clock::now();
		ourTotalTime = std::chrono::seconds(0);
		ourDeltaTime = std::chrono::seconds(0);
		ourScaledTotalTime = 0;
		ResetTimeScale();

		SeedRandom();
	}

	void Timer::Update()
	{
		const std::chrono::duration<double> totalTimeLastFrame = ourTotalTime;
		ourTotalTime = std::chrono::high_resolution_clock::now() - ourStartTime;
		ourDeltaTime = ourTotalTime - totalTimeLastFrame;
		ourScaledTotalTime += GetDeltaTime();
	}

	float Timer::GetDeltaTime()
	{
		return ourDeltaTime.count() * ourTimeScale;
	}

	float Timer::GetFixedDeltaTime()
	{
		return (1.f / 60.f);
	}

	float Timer::GetUnscaledDeltaTime()
	{
		return ourDeltaTime.count();
	}

	double Timer::GetTotalTime()
	{
		return ourTotalTime.count();
	}

	float Timer::GetScaledTotalTime()
	{
		return ourScaledTotalTime;
	}

	//yyyy.mm.dd HH:MM:SS.fff
	std::string Timer::GetTimeStamp()
	{
		const std::chrono::time_point curTime = std::chrono::system_clock::now();
		const __time64_t timeT = std::chrono::system_clock::to_time_t(curTime);
		const std::chrono::time_point curTimeSec = std::chrono::system_clock::from_time_t(timeT);
		const std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - curTimeSec);

		const std::tm* timeStruct = localtime(&timeT);

		const char dateFormat[] = "%Y.%m.%d %H:%M:%S";

		char timeStr[] = "yyyy.mm.dd HH:MM:SS.fff";
		strftime(timeStr, strlen(timeStr), dateFormat, timeStruct);

		std::string result(timeStr);
		result.append(".");

		//Append extra 0's to fill millisecond gap
		if (milliseconds.count() < 10)
		{
			result.append("00");
		}
		else if (milliseconds.count() < 100)
		{
			result.append("0");
		}

		result.append(std::to_string(milliseconds.count()));
		return result;
	}

	std::string Timer::GetDate()
	{
		std::string timeStamp = GetTimeStamp();
		timeStamp.erase(10, 13);
		return timeStamp;
	}

	std::string Timer::GetTime()
	{
		std::string timeStamp = GetTimeStamp();
		timeStamp.erase(0, 11);
		return timeStamp;
	}

	void Timer::SetTimeScale(float aTimeScale)
	{
		ourTimeScale = aTimeScale;
	}

	float Timer::GetTimeScale()
	{
		return ourTimeScale;
	}

	void Timer::ResetTimeScale()
	{
		ourTimeScale = 1.0f;
	}
}
