#include "EditorPch.h"
#include "HenkeWindow.h"
#include "imgui_node_editor.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include "Utils/Math/Random.hpp"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Utils/TimerManager.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"


REGISTER_WINDOW(HenkeEditor);



HenkeEditor::HenkeEditor() : EditorWindow("Henkes Window")
{

}

void HenkeEditor::OnImGui()
{
	ImGui::DragFloat3("Offset", &pos.x);
	ImGui::DragFloat3("Offset2", &pos2.x);
	ImGui::DragInt("amount", &amount, 0, INT_MAX);
	ImGui::DragFloat("Tiden", &tiden);
	ImGuiUtils::FileParameter("FilePath", filename, ".prefab");
	ImGui::Checkbox("RigidBody", &RigidBool);

	// Firefly::Renderer::SubmitDebugLine(pos, pos2);
	Firefly::Renderer::SubmitDebugCuboid((pos2 + pos) * 0.5f, Utils::Vec3(Utils::Abs(pos.x - pos2.x), Utils::Abs(pos.y - pos2.y), Utils::Abs(pos.z - pos2.z)));


	if (ImGui::Button("SpawnButton"))
	{
		for (int i = 0;i < amount;i++)
		{

			Utils::TimerManager::AddTimer([&]()
				{
					auto obj = Firefly::Instantiate(Firefly::ResourceCache::GetAsset<Firefly::Prefab>(filename, true)).lock();
					Utils::Vec3 Finalpos;
					Finalpos.x = Utils::RandomFloat(pos.x, pos2.x);
					Finalpos.y = Utils::RandomFloat(pos.y, pos2.y);
					Finalpos.z = Utils::RandomFloat(pos.z, pos2.z);

					if (obj)
					{
						if (obj->HasComponent<Firefly::RigidbodyComponent>())
						{
							obj->GetComponent<Firefly::RigidbodyComponent>().lock()->Teleport(Finalpos);

						}
						else
						{
							obj->GetTransform().SetPosition(Finalpos);
						}
					}

				}, i * tiden);
		}

	}
}
