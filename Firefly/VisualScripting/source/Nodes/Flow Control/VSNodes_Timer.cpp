#include "VSNodes_Timer.h"
#include "Utils/TimerManager.h"

void VSNode_StartTimer::Init()
{
	CreateExecPin("Start Timer", PinDirection::Input);
	CreateDataPin<float>("Duration", PinDirection::Input);
	CreateDataPin<bool>("Loop", PinDirection::Input);
	CreateDataPin<bool>("Use Time Scale", PinDirection::Input);

	CreateExecPin("Out", PinDirection::Output, true);
	CreateExecPin("On Elapsed", PinDirection::Output);
	CreateDataPin<int>("Timer ID", PinDirection::Output);

	SetPinData<float>("Duration", 3.0f);
	SetPinData<bool>("Use Time Scale", true);
}

size_t VSNode_StartTimer::DoOperation()
{
	float duration;
	bool loop, useTimeScale;

	if (GetPinData("Duration", duration) && GetPinData("Loop", loop) && GetPinData("Use Time Scale", useTimeScale))
	{
		const auto& timer = Utils::TimerManager::AddTimer([this]() { OnTimer(); }, duration, loop, useTimeScale);
		SetPinData<int>("Timer ID", static_cast<int>(timer->TimerID));
		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_StartTimer::OnTimer()
{
	ExitViaPin("On Elapsed");
}

void VSNode_GetTimerInfo::Init()
{
	CreateDataPin<int>("Timer ID", PinDirection::Input);

	CreateDataPin<bool>("Is Valid Timer", PinDirection::Output);

	CreateDataPin<float>("Duration", PinDirection::Output);
	CreateDataPin<float>("Time Elapsed", PinDirection::Output);
	CreateDataPin<float>("Remaining Time", PinDirection::Output);
	CreateDataPin<float>("Complete Percentage", PinDirection::Output);

	CreateDataPin<bool>("Is Looping", PinDirection::Output);
	CreateDataPin<bool>("Uses Time Scale", PinDirection::Output);
	CreateDataPin<bool>("Paused", PinDirection::Output);
}

size_t VSNode_GetTimerInfo::DoOperation()
{
	int timerID;

	if (GetPinData("Timer ID", timerID))
	{
		const auto& timer = Utils::TimerManager::GetTimer(timerID);

		if (timer)
		{
			SetPinData("Is Valid Timer", true);

			SetPinData("Duration", timer->Duration);
			SetPinData("Time Elapsed", timer->TimeElapsed);
			SetPinData("Remaining Time", timer->GetRemainingTime());
			SetPinData("Complete Percentage", timer->GetCompletePercentage());

			SetPinData("Is Looping", timer->IsLooping);
			SetPinData("Uses Time Scale", timer->UseTimeScale);
			SetPinData("Paused", timer->Paused);
		}
		else
		{
			SetPinData("Is Valid Timer", false);
		}
	}

	return 0;
}

void VSNode_ResumeTimer::Init()
{
	CreateExecPin("In", PinDirection::Input);
	CreateExecPin("Out", PinDirection::Output);
	CreateDataPin<int>("Timer ID", PinDirection::Input);
}

size_t VSNode_ResumeTimer::DoOperation()
{
	int timerID;

	if (GetPinData("Timer ID", timerID))
	{
		Utils::TimerManager::ResumeTimer(timerID);
		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_PauseTimer::Init()
{
	CreateExecPin("In", PinDirection::Input);
	CreateExecPin("Out", PinDirection::Output);
	CreateDataPin<int>("Timer ID", PinDirection::Input);
}

size_t VSNode_PauseTimer::DoOperation()
{
	int timerID;

	if (GetPinData("Timer ID", timerID))
	{
		Utils::TimerManager::PauseTimer(timerID);
		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_RemoveTimer::Init()
{
	CreateExecPin("In", PinDirection::Input);
	CreateExecPin("Out", PinDirection::Output);
	CreateDataPin<int>("Timer ID", PinDirection::Input);
}

size_t VSNode_RemoveTimer::DoOperation()
{
	int timerID;

	if (GetPinData("Timer ID", timerID))
	{
		Utils::TimerManager::RemoveTimer(timerID);
		return ExitViaPin("Out");
	}

	return 0;
}