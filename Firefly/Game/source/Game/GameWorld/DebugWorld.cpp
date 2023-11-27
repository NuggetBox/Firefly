#include "Gamepch.h"
#include "DebugWorld.h"

#include <Utils/Timer.h>

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Rendering/Renderer.h"

#include "Utils/InputHandler.h"


namespace Firefly
{
	void DebugWorld::Start()
	{
#ifndef FF_INLAMNING
		myFPSFont = Firefly::ResourceCache::GetAsset<Firefly::Font>("FireflyEngine/Fonts/Roboto.ttf", true);
#endif
	}

	void DebugWorld::Update()
	{
#ifndef FF_INLAMNING
		ShowDebugFPS();
#endif
	}

	void DebugWorld::Exit()
	{
	}

	void DebugWorld::ShowDebugFPS()
	{
		static bool showDebug = false;
		if (Utils::InputHandler::GetKeyDown('M'))
		{
			showDebug = !showDebug;
		}

		if (showDebug)
		{
			static std::array<float, 20> fpsArray{};
			static int fpsIndex = 0;
			static int lowestFPSInTheArray = 0;
			static int highestFPSInTheArray = 0;
			fpsArray[fpsIndex] = 1 / Utils::Timer::GetDeltaTime();

			lowestFPSInTheArray = 1000;
			highestFPSInTheArray = 0;
			for (auto fps : fpsArray)
			{
				if (fps < lowestFPSInTheArray)
				{
					lowestFPSInTheArray = fps;
				}
				if (fps > highestFPSInTheArray)
				{
					highestFPSInTheArray = fps;
				}
			}

			fpsIndex++;
			if (fpsIndex >= fpsArray.size())
			{
				fpsIndex = 0;
			}

			Firefly::TextInfo info{};
			auto mod = 0.01f;
			info.Position = { -0.99f,0.89f,0 };
			const float ratio = 0.5625f; // 16:9 ratio basically 1080/1920 or 720/1280
			info.Size = { 0.1f, 0.1f };
			info.Size.x *= ratio;
			info.Font = myFPSFont;
			float fps = std::accumulate(fpsArray.begin(), fpsArray.end(), 0.0f) / fpsArray.size();
			info.Text = "FPS: " + std::to_string(static_cast<int>(fps));
			info.Is3D = false;

			info.Color = { 1,1,1,1 };

			Firefly::Renderer::Submit(info);

			info.Position = { -0.99f,0.89f - 0.1f,0 };
			info.Text = "Lowest FPS: " + std::to_string(lowestFPSInTheArray);
			Firefly::Renderer::Submit(info);

			info.Position = { -0.99f, 0.89f - 0.2f,0 };
			info.Text = "Highest FPS: " + std::to_string(highestFPSInTheArray);
			Firefly::Renderer::Submit(info);
		}
	}
}

