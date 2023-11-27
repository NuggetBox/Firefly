#pragma once
#include "Firefly/Application/Window.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Core/Layer/LayerStack.h"
#include "Utils/Math/Vector2.hpp"

namespace Firefly
{
	class Event;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		static Application& Get() { return *myInstance; }
		static Scope<Window>& GetWindow() { return myWindow; }

		void OnEvent(Event& aEvent);
		virtual void OnEventSub( Event& aEvent){}

		void Fly();

		void SetIsInPlayMode(bool aIsInPlayMode = true);
		bool GetIsInPlayMode() const { return myIsInPlayMode; }

		void SetResolutionSize(Utils::Vector2<int> aRes);
		Utils::Vector2<int> GetResolution() const;

		void SetFullscreen(bool aBool);
		bool GetIsFullscreen() const;

		void CloseApplication();

		void LockPhysXSimulationMutex();
		void UnlockPhysXSimulationMutex();

	protected:
		virtual LRESULT WindowsMessages(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam) { return 0; }

		LayerStack myLayerStack;

	private:
		LRESULT WndProc(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam);

		bool myFlying = true;
		bool myIsInPlayMode = false;
		
		static Application* myInstance;

		inline static Scope<Window> myWindow;

		float myAccumulatedTime = 0.0f;

		std::mutex myPhysXSimulateLock;
	};

	static Application* CreateApplication();
}
