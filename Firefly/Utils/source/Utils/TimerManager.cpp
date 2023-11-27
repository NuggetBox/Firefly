#include "TimerManager.h"

#include "Timer.h"

void Utils::TimerManager::Update()
{
	std::vector<int> timersToRemove;

	for (size_t i = 0; i < ourTimers.size(); ++i)
	{
		auto& timer = ourTimers[i];

		if (!timer)
		{
			continue;
		}

		if (timer->Paused)
		{
			continue;
		}

		if (timer->UseTimeScale)
		{
			timer->TimeElapsed += Timer::GetDeltaTime();
		}
		else
		{
			timer->TimeElapsed += Timer::GetUnscaledDeltaTime();
		}

		if (timer->IsUpdater)
		{
			timer->UpdateCallback(timer->TimeElapsed);
		}

		if (timer->TimeElapsed >= timer->Duration)
		{
			const int timerID = timer->TimerID;

			if (!timer->IsUpdater)
			{
				timer->Callback();
			}
			const auto it = std::find_if(ourTimers.begin(), ourTimers.end(), [timerID](const std::shared_ptr<TimerInfo>& aTimer) { return aTimer->TimerID == timerID; });

			if (it != ourTimers.end())
			{
				if (!(*it)->IsLooping)
				{
					(*it)->Callback = nullptr;
					timersToRemove.push_back((*it)->TimerID);

					/*ourTimers.erase(it);
					i--;*/
				}
				else
				{
					(*it)->TimeElapsed -= (*it)->Duration;
				}
			}
		}
		else if (timer->DoEveryFrame)
		{
			timer->Callback();
		}
	}

	for (int timerID : timersToRemove)
	{
		RemoveTimer(timerID);
	}
}

std::shared_ptr<const Utils::TimerManager::TimerInfo> Utils::TimerManager::AddTimer(const std::function<void()>& aCallback, float aDuration, bool aShouldLoop, bool aUseTimeScale, bool aDoEveryFrameTimer)
{
	TimerInfo timer;
	timer.TimerID = ourTimerIDCounter++;

	timer.Duration = aDuration;
	timer.TimeElapsed = 0.0f;

	timer.IsLooping = aShouldLoop;
	timer.UseTimeScale = aUseTimeScale;

	timer.Paused = false;

	timer.DoEveryFrame = aDoEveryFrameTimer;

	timer.Callback = aCallback;

	timer.IsUpdater = false;

	ourTimers.push_back(std::make_shared<TimerInfo>(timer));
	return ourTimers.back();
}

std::shared_ptr<const Utils::TimerManager::TimerInfo> Utils::TimerManager::AddUpdateTimer(const std::function<void(float)>& aCallback, float aDuration, bool aShouldLoop, bool aUseTimeScale, bool aDoEveryFrameTimer)
{
	TimerInfo timer;
	timer.TimerID = ourTimerIDCounter++;

	timer.Duration = aDuration;
	timer.TimeElapsed = 0.0f;

	timer.IsLooping = aShouldLoop;
	timer.UseTimeScale = aUseTimeScale;

	timer.Paused = false;

	timer.DoEveryFrame = aDoEveryFrameTimer;

	timer.UpdateCallback = aCallback;

	timer.IsUpdater = true;

	ourTimers.push_back(std::make_shared<TimerInfo>(timer));
	return ourTimers.back();
}

std::shared_ptr<const Utils::TimerManager::TimerInfo> Utils::TimerManager::GetTimer(int aTimerID)
{
	if (const auto it = std::find_if(ourTimers.begin(), ourTimers.end(), [aTimerID](const std::shared_ptr<TimerInfo>& aTimer) {return aTimer->TimerID == aTimerID; });
		it != ourTimers.end())
	{
		return *it;
	}

	return nullptr;
}

void Utils::TimerManager::ResumeTimer(int aTimerID)
{
	if (const auto it = std::find_if(ourTimers.begin(), ourTimers.end(), [aTimerID](const std::shared_ptr<TimerInfo>& aTimer) {return aTimer->TimerID == aTimerID; });
		it != ourTimers.end())
	{
		(*it)->Paused = false;
	}
}

void Utils::TimerManager::PauseTimer(int aTimerID)
{
	if (const auto it = std::find_if(ourTimers.begin(), ourTimers.end(), [aTimerID](const std::shared_ptr<TimerInfo>& aTimer) {return aTimer->TimerID == aTimerID; });
		it != ourTimers.end())
	{
		(*it)->Paused = true;
	}
}

void Utils::TimerManager::RemoveTimer(int aTimerID)
{
	if (const auto it = std::find_if(ourTimers.begin(), ourTimers.end(), [aTimerID](const std::shared_ptr<TimerInfo>& aTimer) {return aTimer->TimerID == aTimerID; });
		it != ourTimers.end())
	{
		(*it)->Callback = nullptr;
		ourTimers.erase(it);
	}
}

void Utils::TimerManager::ResumeAllTimers()
{
	for (size_t i = 0; i < ourTimers.size(); ++i)
	{
		ourTimers[i]->Paused = false;
	}
}

void Utils::TimerManager::PauseAllTimers()
{
	for (size_t i = 0; i < ourTimers.size(); ++i)
	{
		ourTimers[i]->Paused = true;
	}
}

void Utils::TimerManager::RemoveAllTimers()
{
	ourTimers.clear();
}