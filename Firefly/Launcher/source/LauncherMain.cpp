#include <Firefly/Rendering/Renderer.h>

#include "Firefly.h"

#ifndef FF_SHIPIT
#include "Editor/EditorLayer.h"
#endif // !FF_SHIPIT


#include "Firefly/EntryPoint.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/Event/ApplicationEvents.h"

#include "FmodWrapper/AudioManager.h"

#include "Game/GameLayer/GameLayer.h"

#include "Utils/InputHandler.h"


class LauncherApp : public Firefly::Application
{
public:
	LauncherApp()
	{
		// :)

#ifndef FF_SHIPIT
		myLayerStack.Push(new Firefly::ImGuiLayer());
		myLayerStack.Push(new EditorLayer());
#else
		SetIsInPlayMode(true);
		Utils::InputHandler::SetWindowSize({ (float)Firefly::Application::Get().GetWindow()->GetWidth(), (float)Firefly::Application::Get().GetWindow()->GetHeight() });
		Firefly::SceneManager::Initialize();
		Firefly::SceneManager::Get().CreatePhysicsScene();
		Firefly::SceneManager::Get().LoadScene("Assets/Scenes/SplashScreen.scene");
		Firefly::Renderer::SetDebugLinesActive(false);
		Firefly::Renderer::SetGridActive(false);
		AudioManager::StartStop(true);

		Utils::InputHandler::CaptureMouse();
		Utils::InputHandler::HideMouseCursor();
#endif
		myLayerStack.Push(new Firefly::GameLayer());
	}

	void OnEventSub(Firefly::Event& aEvent) override
	{
#ifdef FF_SHIPIT
		if (Firefly::SceneManager::GetCurrentScenes().empty())
			return;

		const auto scenes = Firefly::SceneManager::GetCurrentScenes();
		for (const auto & i : scenes)
		{
			if (!i.expired())
			{
				const auto& scene = i.lock();

				if (scene->IsLoaded())
				{
					scene->OnEvent(aEvent);
				}
			}
		}

		Firefly::EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& aEvent) -> bool
			{
				Utils::InputHandler::SetWindowSize({ (float)Firefly::Application::Get().GetWindow()->GetWidth(), (float)Firefly::Application::Get().GetWindow()->GetHeight() });
				Utils::InputHandler::SetMouseRelativePos(Utils::InputHandler::GetMousePosition().x, Utils::InputHandler::GetMousePosition().y);
				return false;
			});
#endif
	}

protected:
	LRESULT WindowsMessages(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam) override
	{
		return 0;
	}
};

Firefly::Application* Firefly::CreateApplication()
{
	return new LauncherApp();
}