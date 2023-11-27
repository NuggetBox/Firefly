#include "EditorPch.h"
#include "FoliagePaintingWindow.h"

#include <Firefly/Application/Application.h>
#include <Firefly/Asset/ResourceCache.h>
#include <Firefly/Components/Utilities/FoliagePlaneComponent.h>
#include <Firefly/ComponentSystem/Entity.h>
#include <Firefly/ComponentSystem/SceneManager.h>
#include <Firefly/Rendering/Renderer.h>
#include <Utils/Math/Intersection.hpp>
#include <Utils/Math/Ray.hpp>

#include "ViewportWindow.h"

#include "Utils/InputHandler.h"

#include "WindowRegistry.h"
#include "Editor/Event/EditorOnlyEvents.h"
#include "Editor/UndoSystem/Commands/SceneCommands/PrefabCreateCommand.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/EditorLayer.h"
#include "Editor/UndoSystem/Commands/EntityCommands/AbsoluteTransformCommand.h"
#include "Editor/UndoSystem/Commands/EntityCommands/TransformCommand.h"

#include "Firefly/ComponentSystem/Scene.h"

#include "Utils/Math/Random.hpp"

REGISTER_WINDOW(FoliagePaintingWindow);

FoliagePaintingWindow::FoliagePaintingWindow() : EditorWindow("Foliage Painting")
{
	myFoliageIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_foliage.dds");
	myPaintingActive = false;
	myRandomRotations = false;
}

void FoliagePaintingWindow::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<SaveEditorSettingsEvent>([this](SaveEditorSettingsEvent& aEvent)
		{
			auto& json = aEvent.GetJson();
			auto& foliageJson = json["FoliagePainting"];

			foliageJson.clear();

			for (int i = 0; i < myFoliagePrefabs.size(); ++i)
			{
				foliageJson[i] = myFoliagePrefabs[i]->GetPrefabID();
			}

			return false;
		});

	dispatcher.Dispatch<LoadEditorSettingsEvent>([this](LoadEditorSettingsEvent& aEvent)
		{
			auto& json = aEvent.GetJson();
			auto& foliageJson = json["FoliagePainting"];

			for (int i = 0; i < foliageJson.size(); ++i)
			{
				Ref<Firefly::Prefab> prefab = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(foliageJson[i].get<uint64_t>());

				if (prefab)
				{
					myFoliagePrefabs.push_back(prefab);
				}
				else
				{
					LOGWARNING("A broken prefab was found in the Foliage Painting Window, removing it from available foliage.");
				}
			}

			return false;
		});
}

void FoliagePaintingWindow::OnImGui()
{
	AcceptPrefabDragDrop();

	if (ImGui::Checkbox("Painting Active", &myPaintingActive))
	{
		EditorLayer::GetWindow<ViewportWindow>()->EnableMousePicking(!myPaintingActive);
	}

	ImGui::Checkbox("Random Rotation", &myRandomRotations);

	if (ImGui::Button("Clear Selected Foliage"))
	{
		mySelectedIndices.clear();
	}

	const float height = 50.0f + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;
	const float maxVerticalitems = (ImGui::GetContentRegionMax().y) / height;
	const float width = 50.0f + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().CellPadding.x;
	const float maxHorizontalitems = (ImGui::GetWindowContentRegionWidth()) / width;

	constexpr float existingContentHeight = 110.0f; //lol
	ImGui::BeginChild("##FoliageContentArea", { maxHorizontalitems * width, maxVerticalitems * height - existingContentHeight }, true);

	if (ImGui::BeginTable("##FoliageTable", Utils::Max(static_cast<int>(maxHorizontalitems), 1), ImGuiTableFlags_SizingFixedFit))
	{
		for (int i = 0; i < myFoliagePrefabs.size(); ++i)
		{
			auto& foliage = myFoliagePrefabs[i];
			auto indexIT = std::find(mySelectedIndices.begin(), mySelectedIndices.end(), i);

			ImGui::TableNextColumn();

			ImVec4 tint;

			if (indexIT != mySelectedIndices.end())
			{
				tint = { 1, 1, 1, 1 };
			}
			else
			{
				tint = { 0.25f, 0.25f, 0.25f, 0.25f };
			}

			ImGui::PushID(foliage->GetPath().filename().string().c_str());
			if (ImGui::ImageButton(myFoliageIcon->GetSRV().Get(), ImVec2(50.0f, 50.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tint))
			{
				if (Utils::InputHandler::GetKeyHeld(VK_CONTROL))
				{
					if (indexIT == mySelectedIndices.end())
					{
						mySelectedIndices.push_back(i);
					}
					else
					{
						mySelectedIndices.erase(indexIT);
					}
				}
				else
				{
					mySelectedIndices.clear();
					mySelectedIndices.push_back(i);
				}
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove"))
				{
					mySelectedIndices.clear();
					myFoliagePrefabs.erase(myFoliagePrefabs.begin() + i);
					--i;
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();

			//Tack fabian
			auto displayText = foliage->GetPath().filename().string();
			auto textSize = ImGui::CalcTextSize(displayText.c_str());
			bool isTooLong = textSize.x > 50.0f;
			while (textSize.x > 50.0f)
			{
				displayText = displayText.substr(0, displayText.size() - 1);
				textSize = ImGui::CalcTextSize(displayText.c_str());
			}
			if (isTooLong)
			{
				displayText = displayText.substr(0, displayText.size() - 1);
				displayText += "...";
			}

			ImGui::Text(displayText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::SetTooltip(foliage->GetPath().filename().string().c_str());
				ImGui::EndTooltip();
			}
		}
		ImGui::EndTable();
	}

	ImGui::EndChild();

	if (myPaintingActive)
	{
		if (Utils::InputHandler::GetLeftClickDown())
		{
			Utils::Vector2f mousePos = { Utils::InputHandler::GetMouseRelativeXPos(), Utils::InputHandler::GetMouseRelativeYPos() };
			auto windowSize = Utils::InputHandler::GetWindowSize();

			//If mouse within viewport
			if (mousePos.x > 3 && mousePos.x < windowSize.x - 4 && mousePos.y > 3 && mousePos.y < windowSize.y - 4)
			{
				if (!myFoliagePrefabs.empty())
				{
					if (!mySelectedIndices.empty())
					{
						auto cam = Firefly::Renderer::GetActiveCamera();

						Utils::Vector3f dir = cam->ScreenPosToWorldDirection(mousePos, windowSize);

						Utils::Ray<float> ray;
						ray.InitWithOriginAndDirection(cam->GetTransform().GetPosition(), dir);

						std::vector<std::pair<Utils::Vector3f, Ptr<Firefly::Entity>>> detectedEntityCollisions;

						for (auto scene : EditorLayer::GetEditingScenes())
						{
							if (scene.expired())
							{
								LOGERROR("Scene expired");
								continue;
							}
							for (auto entity : scene.lock()->GetEntities())
							{
								if (entity.expired())
								{
									LOGERROR("Entity expired");
									continue;
								}
								
								if (auto comp = entity.lock()->GetComponent<Firefly::FoliagePlaneComponent>().lock())
								{
									Utils::Vector3f intersection;
									if (Utils::IntersectionBoundingPlaneRay(comp->GetBoundingPlane(), ray, intersection))
									{
										detectedEntityCollisions.push_back(std::pair(intersection, entity));
									}
								}
							}
						}

						if (!detectedEntityCollisions.empty())
						{
							if (detectedEntityCollisions.size() > 1)
							{
								std::sort(detectedEntityCollisions.begin(), detectedEntityCollisions.end(), [&](auto first, auto second)
									{
										return (first.first - cam->GetTransform().GetPosition()).LengthSqr() <
											(second.first - cam->GetTransform().GetPosition()).LengthSqr();
									});
							}

							std::pair<Utils::Vector3f, Ptr<Firefly::Entity>> closestCollision = detectedEntityCollisions[0];
							Utils::Quaternion targetRot = closestCollision.second.lock()->GetTransform().GetQuaternion();
							Utils::Vector3f targetPos = closestCollision.first;

							int indexToSpawn = 0;
							if (mySelectedIndices.size() > 1)
							{
								indexToSpawn = Utils::RandomInt(0, mySelectedIndices.size());
							}

							Ref<Firefly::Prefab> foliageToSpawn = myFoliagePrefabs[mySelectedIndices[indexToSpawn]];

							EditorLayer::BeginEntityUndoSeries();

							Ref<PrefabCreateCommand> prefabInstantiate = CreateRef<PrefabCreateCommand>(foliageToSpawn, Ptr<Firefly::Entity>(), closestCollision.second.lock()->GetParentScene(), false);
							EditorLayer::ExecuteAndAddEntityUndo(prefabInstantiate);
							auto entity = prefabInstantiate->GetCreatedEntity();

							if (myRandomRotations)
							{
								targetRot = targetRot * Utils::Quaternion::CreateFromEulerAngles(0, Utils::RandomAngleDegrees(), 0);
							}

							if (!entity.expired())
							{
								Ref<AbsoluteTransformCommand> transformationCmd = CreateRef<AbsoluteTransformCommand>(entity, targetPos, targetRot, entity.lock()->GetTransform().GetScale());
								EditorLayer::ExecuteAndAddEntityUndo(transformationCmd);
							}

							EditorLayer::EndEntityUndoSeries();

							if (!entity.expired())
							{
								const auto& col = closestCollision.second;

								if (!col.expired())
								{
									const auto& parent = col.lock()->GetParent();

									if (!parent.expired())
									{
										entity.lock()->SetParent(parent.lock());
									}
									else
									{
										LOGERROR("Foliage Plane has no parent, couldn't set parent");
									}
								}
								else
								{
									LOGERROR("Raycasted entity expired");
								}
							}
						}
					}
					else
					{
						ImGuiUtils::NotifyWarning("No foliage prefabs have been selected! Click on one in the Foliage Painting Window to select, CTRL + Click to select multiple.");
					}
				}
				else
				{
					ImGuiUtils::NotifyWarning("No prefabs have been added to the foliage painting window! Drag & Drop a prefab from the content browser to add it as available foliage.");
				}
			}
		}
	}
}

void FoliagePaintingWindow::AcceptPrefabDragDrop()
{
	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".prefab", false))
	{
		std::filesystem::path droppedFile = static_cast<const char*>(payload->Data);

		auto prefab = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(droppedFile, true);
		auto it = std::find(myFoliagePrefabs.begin(), myFoliagePrefabs.end(), prefab);

		if (it == myFoliagePrefabs.end())
		{
			myFoliagePrefabs.push_back(prefab);
		}
		else
		{
			ImGuiUtils::NotifyWarningLocal("That prefab has already been added as foliage!");
		}
	}
}