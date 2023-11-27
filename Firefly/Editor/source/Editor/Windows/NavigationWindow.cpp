#include "EditorPch.h"
#include "NavigationWindow.h"

#include "WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Firefly/ComponentSystem/SceneManager.h"
//#include "Firefly/Navigation/NavMeshObstacleComponent.h"
//#include "Firefly/Navigation/NavMeshSurfaceComponent.h"
//#include "Editor/EditorLayer.h"
//
//#include "Explorer/NavMesh/NavMesh.h"
//
//#include "Firefly/ComponentSystem/Entity.h"
//#include "Firefly/Navigation/FireflyNavMesh.h"

#include "Utils/Timer.h"

REGISTER_WINDOW(NavigationWindow);

NavigationWindow::NavigationWindow() : EditorWindow("Navigation")
{

}

void NavigationWindow::OnImGui()
{
	//static Explorer::NavMeshBuildSettings settings;
	//static bool ShowNavMesh = true;
	//static bool LiveUpdate = false;
	//static float timer = 1;
	//ImGui::Checkbox("Show NavMesh", &ShowNavMesh);
	//ImGui::InputFloat("Radius", &settings.Radius);
	//ImGui::InputFloat("MaxSlope", &settings.MaxSlope);
	//ImGui::InputFloat("StepHeight", &settings.StepHeight);
	//ImGui::InputFloat("BindDistance", &settings.BindDistance);

	//if (ImGui::Button("Hide Meshes"))
	//{
	//	Firefly::SceneManager::Get().SetMavmeshMeshes(false);
	//}
	//ImGui::SameLine();
	//if (ImGui::Button("Show Meshes"))
	//{
	//	Firefly::SceneManager::Get().SetMavmeshMeshes(true);
	//}


	//const std::vector<Ptr<Firefly::Entity>>& entites = EditorLayer::GetSelectedEntities();

	//ImGui::Text("Object NavType:");
	//if (entites.size() == 1)
	//{
	//	const auto firstEntity = entites[0].lock();
	//	const ImVec2 buttonSize = { 60, 25 };

	//	if (firstEntity->HasComponent<NavMeshObstacleComponent>() && firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//	{
	//		firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshObstacleComponent>());
	//		firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshSurfaceComponent>());
	//	}

	//	// None Button
	//	{
	//		if (!firstEntity->HasComponent<NavMeshObstacleComponent>() && !firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5f));
	//		}
	//		else
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 0.5f));
	//		}

	//		if (ImGui::Button("None", buttonSize))
	//		{
	//			if (firstEntity->HasComponent<NavMeshObstacleComponent>())
	//			{
	//				firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshObstacleComponent>());
	//			}
	//			if (firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//			{
	//				firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshSurfaceComponent>());
	//			}
	//		}
	//		ImGui::PopStyleColor(1);
	//	}
	//	ImGui::SameLine();
	//	// Surface Button
	//	{
	//		if (firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5f));
	//		}
	//		else
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 0.5f));
	//		}

	//		if (ImGui::Button("Surface", buttonSize))
	//		{
	//			if (firstEntity->HasComponent<NavMeshObstacleComponent>())
	//			{
	//				firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshObstacleComponent>());
	//			}
	//			if (!firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//			{
	//				firstEntity->AddComponent(CreateRef< NavMeshSurfaceComponent>());
	//			}
	//		}
	//		ImGui::PopStyleColor(1);
	//	}
	//	ImGui::SameLine();
	//	// Obstacle Button
	//	{
	//		if (firstEntity->HasComponent<NavMeshObstacleComponent>())
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5f));
	//		}
	//		else
	//		{
	//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 0.5f));
	//		}

	//		if (ImGui::Button("Obstacle", buttonSize))
	//		{
	//			if (firstEntity->HasComponent<NavMeshSurfaceComponent>())
	//			{
	//				firstEntity->RemoveComponent(firstEntity->GetComponent<NavMeshSurfaceComponent>());
	//			}
	//			if (!firstEntity->HasComponent<NavMeshObstacleComponent>())
	//			{
	//				firstEntity->AddComponent(CreateRef<NavMeshObstacleComponent>());
	//			}
	//		}
	//		ImGui::PopStyleColor(1);
	//	}
	//}
	//else
	//{
	//	ImGui::Text("Please select ONE entity to edit");
	//}
	//if (myBenneIsCringe < 20)
	//{
	//	myBenneIsCringe++;
	//}
	//ImGui::Checkbox("LiveUpdate", &LiveUpdate);
	//timer += Utils::Timer::GetDeltaTime();
	//if (ImGui::Button("Bake Navmesh"))
	//{
	//	if (timer >= 1)
	//	{
	//		ShowNavMesh = false;
	//		Firefly::SceneManager::Get().BakeNavMesh(settings);
	//		myBenneIsCringe = 0;
	//		Explorer::BuildInfo info;

	//		const auto& navPtr = Firefly::SceneManager::Get().GetNavMesh();

	//		if (navPtr)
	//		{
	//			info = navPtr->GetBuildInfo();
	//		}

	//		std::string message = std::format("NavMesh Build With {} Surfaces, {} Obstacles, In {} ms", info.NumberOfSurfaces, info.NumberOfObstacles, info.BuildTimeInMS);

	//		ImGui::InsertNotification({ ImGuiToastType_Success, 3000, message.c_str() });
	//		timer = 0;
	//	}
	//}
	//

	//if (ShowNavMesh && timer >= 0.5f)
	//{
	//	Firefly::SceneManager::Get().RenderNavMesh();
	//}
}