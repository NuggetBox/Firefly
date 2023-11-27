#pragma once
#include <chrono>

namespace Utils
{
	class Timer
	{
	public:
		Timer() = delete;
		~Timer() = delete;
		Timer(const Timer& aTimer) = delete;
		Timer& operator=(const Timer& aTimer) = delete;

		static void Start();
		static void Update();

		static float GetDeltaTime();
		static float GetFixedDeltaTime();
		static float GetUnscaledDeltaTime();
		static double GetTotalTime();
		static float GetScaledTotalTime();

		static std::string GetTimeStamp();
		static std::string GetDate();
		static std::string GetTime();

		static void SetTimeScale(float aTimeScale);
		static float GetTimeScale();
		static void ResetTimeScale();

	private:
		inline static std::chrono::time_point<std::chrono::high_resolution_clock> ourStartTime;
		inline static std::chrono::duration<double> ourTotalTime;
		inline static std::chrono::duration<float> ourDeltaTime;

		inline static float ourScaledTotalTime;
		inline static float ourTimeScale;
	};
}