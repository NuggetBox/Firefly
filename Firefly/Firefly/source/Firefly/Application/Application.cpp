#include "FFpch.h"
#include "Application.h"

#ifndef FF_SHIPIT
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <imnotify/imgui_notify.h>

#include "imgui/ImGuizmo.h"
#endif

#include <FmodWrapper/AudioManager.h>

#include "Firefly/Asset/Importers/PipelineImporter.h"
#include "Firefly/Asset/Importers/ShaderImporter.h"
#include "Firefly/Asset/ResourceCache.h"

#include "Firefly/ComponentSystem/SceneManager.h"

#include "Firefly/Core/Layer/ImGuiLayer.h"

#include "Firefly/Event/ApplicationEvents.h"

#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Rendering/Renderer.h"

#include "Optick/optick.h"

#include "Utils/InputHandler.h"
#include "Utils/Timer.h"

#include "Firefly/Utilities/Filewatcher.h"

#include "Firefly/Physics/PhysicsScene.h"
#include "Firefly/Physics/PhysicsLayerHandler.h"
#include "Firefly/Physics/PhysicsImplementation.h"

#include "Utils/InputHandler.h"

#include "Utils/InputMap.h"
#include "DiscordAPI/DiscordAPI.h"

#include "FBXExporter/Exporter.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Firefly
{
	Application* Application::myInstance = nullptr;

	Application::Application()
	{
		myInstance = this;

		WindowProperties windowProps;

		const wchar_t* title;

#ifdef FF_SHIPIT
		title = L"Can Strike";
		windowProps.WindowMode = WindowMode::Windowed;
		const int sizeX = GetSystemMetrics(SM_CXSCREEN);
		const int sizeY = GetSystemMetrics(SM_CYSCREEN);
		windowProps.X = 0;
		windowProps.Y = 0;
#else
		title = L"Firefly";
		windowProps.WindowMode = WindowMode::Windowed;
		constexpr int sizeX = 1280;
		constexpr int sizeY = 720;
		windowProps.X = (GetSystemMetrics(SM_CXSCREEN) - sizeX) / 2;
		windowProps.Y = (GetSystemMetrics(SM_CYSCREEN) - sizeY) / 2;
#endif

		windowProps.Width = sizeX;
		windowProps.Height = sizeY;
		windowProps.Title = title;

		myWindow = CreateScope<Window>(windowProps);

		myWindow->SetWndFunctionToCall([this](HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam) -> LRESULT
			{
				return WndProc(aHwnd, aMessage, aWParam, aLParam);
			});

		GraphicsContextInfo info;

#ifdef FF_DEBUG
		info.EnableDebug = true;
#else
		info.EnableDebug = false;
#endif

		info.Fullscreen = false;
		info.Vsync = false;
		info.Width = sizeX;
		info.Height = sizeY;

		GraphicsContext::Initialize(info);

		PhysicsLayerHandler::Initialize();
		PhysicsImplementation::Initialize();

		FileWatcherInfo filewatcherInfo{};
		filewatcherInfo.Folders.emplace_back("FireflyEngine/Shaders");
		filewatcherInfo.Folders.emplace_back("FireflyEngine/Pipelines/Custom");
		FileWatcher::Initialize(filewatcherInfo);

		ShaderImporter::Initialize();
		PipelineImporter::Initialize();
		ResourceCache::Initialize();
		Renderer::Initialize();
		AudioManager::Initialization();
		Utils::InputHandler::ResetAllInput();
		Utils::Timer::Start();
		InputMap::Init();
		DiscordAPI::Initialize();
#ifndef FF_SHIPIT
		//ShaderLibrary::ReCompileAllShaders();
#endif

#ifdef FF_SHIPIT
		myWindow->SetFullScreen(true);
#endif // FF_SHIPIT

		Utils::InputHandler::Initialize(myWindow->GetHandle());
		Utils::Timer::Update();
	}

	LRESULT Application::WndProc(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam)
	{
		{
			//FF_PROFILESCOPE("Layers");
			for (const auto& layer : myLayerStack)
			{
				layer->WindowsMessages(aMessage, aWParam, aLParam);
			}
		}

#ifndef FF_SHIPIT
		ImGui_ImplWin32_WndProcHandler(aHwnd, aMessage, aWParam, aLParam);
#endif

		{
			FF_PROFILESCOPE("Input");
			if (Utils::InputHandler::UpdateEvents(aMessage, aWParam, aLParam))
			{
				return false;
			}
		}

		WindowsMessages(aHwnd, aMessage, aWParam, aLParam);

		{
			FF_PROFILESCOPE("Other");
			switch (aMessage)
			{
			case WM_CLOSE:
				//PhysicsImplementation::ShutDown();
				myFlying = false;
				break;
			case WM_SIZE:
				UINT x = LOWORD(aLParam);
				UINT y = HIWORD(aLParam);
				GraphicsContext::Resize(x, y);
				Renderer::GetFrameBuffer()->Resize({ x, y });
				break;
			}
		}
		return 0;
	}

	Application::~Application()
	{
		PhysicsImplementation::ShutDown();
		ResourceCache::Shutdown();
		myWindow->UnsetWindowsFunction();
	}

	void Application::OnEvent(Event& aEvent)
	{
		OnEventSub(aEvent);
		for (auto& layer : myLayerStack)
		{
			layer->OnEvent(aEvent);
		}
	}

	void Application::Fly()
	{
#ifdef FF_SHIPIT
		myIsInPlayMode = true;
#endif
		while (myFlying)
		{
			FF_PROFILEFRAME("Main");

#ifndef FF_SHIPIT
			{
				FF_PROFILESCOPE("ImGui::NewFrame");
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				ImGuizmo::BeginFrame();
			}
#endif

			auto& events = FileWatcher::GetEvents();
			for (auto& ev : events)
			{
				OnEvent(ev);
			}
			events.clear();

			FileWatcher::Watch();

			for (const auto& layer : myLayerStack)
			{
				layer->OnUpdate();
			}

			{
				FF_PROFILESCOPE("Timer Update");
				Utils::Timer::Update();
			}

			//Delete queued entities after all updates have run
			SceneManager::Get().DeleteQueuedEntities();

			myWindow->ProcessMessages();
			Renderer::Begin();

			{
				FF_PROFILESCOPE("Graphics Context Clear");
				GraphicsContext::Clear({0.0f, 0.0f, 0.05f, 1.0f});
			}

			{
				FF_PROFILESCOPE("Update Audio Manager");
				AudioManager::Update();
			}

			auto physScene = SceneManager::Get().GetPhysicsScene();
			if (physScene)
			{
				myAccumulatedTime += Utils::Timer::GetDeltaTime();
				physScene->UpdateControllerManager();
				bool hasUpdated = false;
				while (myAccumulatedTime >= Utils::Timer::GetFixedDeltaTime())
				{
					myAccumulatedTime -= Utils::Timer::GetFixedDeltaTime();
					//myAccumulatedTime = 0;

					LockPhysXSimulationMutex();
					physScene->Simulate(Utils::Timer::GetFixedDeltaTime());
					UnlockPhysXSimulationMutex();
					hasUpdated = true;

					{
						FF_PROFILESCOPE("App Fixed Update Event");
						AppFixedUpdateEvent fixedUpdateEvent(myIsInPlayMode);
						OnEvent(fixedUpdateEvent);
						AppLateFixedUpdateEvent lateFixedUpdateEvent(myIsInPlayMode);
						OnEvent(lateFixedUpdateEvent);
					}
				}

				if (hasUpdated)
				{
					physScene->UpdateQuery();
				}


				PhysicsImplementation::FlushActorCallbacks();
			}
			else
			{
				PhysicsImplementation::ClearActorCallbacks();
			}

#ifndef FF_SHIPIT
			{
				FF_PROFILESCOPE("Editor App Update Event");
				EditorAppUpdateEvent editorUpdateEvent(myIsInPlayMode);
				OnEvent(editorUpdateEvent);
			}
#endif

			{
				FF_PROFILESCOPE("App Update Event");
				AppUpdateEvent updateEvent(myIsInPlayMode);
				OnEvent(updateEvent);
			}
			{
				FF_PROFILESCOPE("App Late Update Event");
				AppLateUpdateEvent updateEvent(myIsInPlayMode);
				OnEvent(updateEvent);
			}
			{
				FF_PROFILESCOPE("App Render Event");
				AppRenderEvent renderEvent;
				OnEvent(renderEvent);
			}

			static uint32_t value = 0;
			if (Utils::InputHandler::GetKeyDown(VK_F6))
			{
				value++;
				value %= 7;

			}

			DiscordAPI::Update();

			globalRendererStats.VisablePass = value;
			Renderer::Sync();

			{
				FF_PROFILESCOPE("Bind swapchain");
				GraphicsContext::BindSwapchainRTV();
			}

#ifdef FF_SHIPIT
			{
				FF_PROFILESCOPE("Transfer swapchain");
				GraphicsContext::TransferFBtoSwapchain(Renderer::GetFrameBuffer().get());
			}
#endif


#ifndef FF_SHIPIT
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
			ImGui::RenderNotifications();
			ImGui::PopStyleVar(1);
			ImGui::PopStyleColor(1);

			// MAGIC CODE DO NOT TOUCH!!!!!!!
			{
				FF_PROFILESCOPE("ImGui Draw");
				{
					FF_PROFILESCOPE("ImGui Render");
					GraphicsContext::BeginEvent("ImGui Draws");
					ImGui::Render();
				}
				{
					FF_PROFILESCOPE("ImGui Render draw data");
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				}
				GraphicsContext::EndEvent();
			}
#endif

			{
				FF_PROFILESCOPE("Present");
				GraphicsContext::Present();
			}

#ifndef FF_SHIPIT

			{
				FF_PROFILESCOPE("ImGui update windows")
					auto& io = ImGui::GetIO();
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}
			}
#endif

			Utils::InputHandler::UpdatePreviousState();
			{
				FF_PROFILESCOPE("Resize all Framebuffers");
				Framebuffer::FlushResizes();
			}
		}
	}

	void Application::SetIsInPlayMode(bool aIsInPlayMode)
	{
		myIsInPlayMode = aIsInPlayMode;
	}

	void Application::SetResolutionSize(Utils::Vector2<int> aRes)
	{
		Renderer::GetFrameBuffer()->Resize({ static_cast<uint32_t>(aRes.x), static_cast<uint32_t>(aRes.y) });
		GraphicsContext::Resize(static_cast<uint32_t>(aRes.x), static_cast<uint32_t>(aRes.y));
	}

	void Application::SetFullscreen(bool aBool)
	{
		myWindow->SetFullScreen(aBool);
	}

	Utils::Vector2<int> Application::GetResolution() const
	{
		return Utils::Vector2<int>(myWindow->GetWidth(), myWindow->GetHeight());
	}

	bool Application::GetIsFullscreen() const
	{
		return myWindow->GetIsFullscreen();
	}

	void Application::CloseApplication()
	{
#ifndef FF_SHIPIT
		EditorStopEvent e;
		OnEvent(e);
#else
		delete myInstance;
		exit(0);
#endif
}

	void Application::LockPhysXSimulationMutex()
	{
		myPhysXSimulateLock.lock();
	}

	void Application::UnlockPhysXSimulationMutex()
	{
		myPhysXSimulateLock.unlock();
	}
}
