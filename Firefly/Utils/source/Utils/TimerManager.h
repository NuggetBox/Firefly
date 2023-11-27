#pragma once
#include <chrono>
#include <functional>

namespace Utils
{
	class TimerManager
	{
	public:
		struct TimerInfo
		{
			friend class TimerManager;

			float GetRemainingTime() const { return Duration - TimeElapsed; }
			float GetCompletePercentage() const { return TimeElapsed / Duration; }

			int TimerID;

			float Duration;
			float TimeElapsed;

			bool IsLooping;
			bool UseTimeScale;

			bool DoEveryFrame;

			bool Paused;

		private:
			bool IsUpdater = false;

			std::function<void()> Callback;
			std::function<void(float)> UpdateCallback;
		};

		static void Update();

		static std::shared_ptr<const TimerInfo> AddTimer(const std::function<void()>& aCallback, float aDuration, bool aShouldLoop = false, bool aUseTimeScale = true, bool aDoEveryFrameTimer = false);
		static std::shared_ptr<const TimerInfo> AddUpdateTimer(const std::function<void(float)>& aCallback, float aDuration, bool aShouldLoop = false, bool aUseTimeScale = true, bool aDoEveryFrameTimer = false);
		static std::shared_ptr<const TimerInfo> GetTimer(int aTimerID);

		static void ResumeTimer(int aTimerID);
		static void PauseTimer(int aTimerID);
		static void RemoveTimer(int aTimerID);

		static void ResumeAllTimers();
		static void PauseAllTimers();
		static void RemoveAllTimers();

	private:
		static inline std::vector<std::shared_ptr<TimerInfo>> ourTimers;
		static inline int ourTimerIDCounter = 0;
	};
}