#include "EditorPch.h"
#include "RenderStatsWindow.h"

#include <Firefly/Rendering/RenderCommands.h>
#include <Utils/Timer.h>

#include "WindowRegistry.h"
#include <Firefly/Rendering/Renderer.h>
#include "Firefly/Rendering/Framebuffer.h"

REGISTER_WINDOW(RenderStatsWindow);

RenderStatsWindow::RenderStatsWindow() : EditorWindow("Render Stats")
{
	myGetFPSTimer = 0.0f;
	myPrevDelta = Utils::Timer::GetUnscaledDeltaTime();
}

void RenderStatsWindow::OnImGui()
{
	constexpr float DeltaTimeSamplesPerSec = 5.0f;
	constexpr float diff = 1 / DeltaTimeSamplesPerSec;

	myGetFPSTimer += Utils::Timer::GetUnscaledDeltaTime();

	if (myGetFPSTimer >= diff)
	{
		myPrevDelta = Utils::Timer::GetUnscaledDeltaTime();
		myGetFPSTimer = 0.0f;
	}

	ImGui::Text("Firefly Rendering Stats");
	ImGui::Separator();
	ImGui::Text("Drawcalls: %d", Firefly::globalRendererStats.DrawCalls);
	ImGui::Text("Point lights: %d", Firefly::globalRendererStats.PointLightCount);
	ImGui::Text("Spot lights: %d", Firefly::globalRendererStats.SpotLightCount);
	ImGui::Text("Sprite count: %d", Firefly::globalRendererStats.QuadCount);
	ImGui::Text("Tri count: %d", Firefly::globalRendererStats.TriCount);
	ImGui::Text("Line count: %d", Firefly::globalRendererStats.LineCount);
	ImGui::Text("Frame time (ms): %f", Utils::Timer::GetUnscaledDeltaTime() * 1000.f);
	ImGui::Text("FPS: %i", static_cast<int>(1 / myPrevDelta));

	auto cache = Firefly::Renderer::GetShadowMap(0);
	if (ImGui::CollapsingHeader("Shadow maps"))
	{
		for (auto& v : cache)
		{

		ImGui::Image(v->GetSRV().Get(), { 128, 128 });
		}

	}
}
