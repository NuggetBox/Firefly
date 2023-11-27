#include "Gamepch.h"
#include "GameLayer.h"

#include <Utils/TimerManager.h>

#include "Firefly/Components/ParticleSystem/ForceFieldManager.h"
#include <Utils/InputHandler.h>

#include "Firefly/Application/Application.h"

#include "Game/Event/GameEvents.h"

#include "Utils/Timer.h"

namespace Firefly
{
	void GameLayer::OnAttach()
	{
#ifndef FF_INLAMNING
		myDebugWorld.Start();
#endif
	}

	void GameLayer::OnUpdate()
	{
		
	}

	void GameLayer::OnDetach()
	{
#ifndef FF_INLAMNING
		myDebugWorld.Exit();
#endif
	}

	void GameLayer::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent&)
		{
			Utils::TimerManager::Update();
			ForceFieldManager::Get().UpdateForceFields();

			if (Utils::InputHandler::WasFocusLost())
			{
				LOGINFO("Focus lost");

				if (Utils::IsAlmostEqual(Utils::Timer::GetTimeScale(), 1.0f, 0.001f))
				{
					TogglePauseEvent pause(true);
					Application::Get().OnEvent(pause);
				}
			}

			if (Utils::InputHandler::WasFocusGained())
			{
				LOGINFO("Focus gained");
			}

#ifndef FF_INLAMNING
			myDebugWorld.Update();
#endif

			return false;
		});
	}
}
