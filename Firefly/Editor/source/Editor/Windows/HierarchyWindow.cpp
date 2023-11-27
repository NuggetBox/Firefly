#include "EditorPch.h"
#include "HierarchyWindow.h"

#include <Firefly/Asset/Animation.h>
#include <Firefly/Components/Mesh/MeshComponent.h>
#include <Firefly/Components/Mesh/AnimatedMeshComponent.h>
#include <Firefly/Components/Animation/AnimationPlayerComponent.h>
#include <Firefly/Components/ParticleSystem/ParticleEmitterComponent.h>
#include <Firefly/Rendering/Renderer.h>

#include "ViewportWindow.h"
#include "Editor/EditorLayer.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"

#include "Firefly/Event/ApplicationEvents.h"

#include "Editor/Utilities/EditorUtils.h"
#include "Editor/Utilities/ImGuiUtils.h"

#include "Editor/UndoSystem/Commands/SceneCommands/EntityReorderCommand.h"
#include "Editor/UndoSystem/Commands/SceneCommands/EntityCreateCommand.h"
#include "Editor/UndoSystem/Commands/SceneCommands/EntityDeleteCommand.h"
#include "Editor/UndoSystem/Commands/SceneCommands/PrefabCreateCommand.h"

#include "Editor/Windows/WindowRegistry.h"
#include "ContentBrowser.h"

#include "Editor/Event/EditorOnlyEvents.h"

#include "SerializationUtils.h"
#include <imgui/imgui_internal.h>

#include "Editor/UndoSystem/Commands/EntityCommands/EntityDeselectAllCommand.h"
#include "Editor/UndoSystem/Commands/EntityCommands/EntitySelectCommand.h"

#include "Firefly/Application/Application.h"

#include "Utils/InputHandler.h"

REGISTER_WINDOW(HierarchyWindow);

HierarchyWindow::HierarchyWindow() : EditorWindow("Hierarchy")
{
	myBetweenIndex = -1;
	myDragging = false;
	myExpandAll = false;
	myCollapseAll = false;
	myOpenAllParents = false;
	myAlreadyAutoScrolled = false;
	mySearchString = "";
}

void HierarchyWindow::OnImGui()
{
	FF_PROFILEFUNCTION();
	myUnsavedChangesFlag = EditorLayer::IsAnyEditSceneModified();

	// if user pressed anywhere in the hierarchy window except on an entity
	if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
	{
		//EditorLayer::DeselectAllEntities();
		EditorLayer::DeselectAllEntitiesCommand();
	}

	if (ImGui::BeginPopupContextWindow())
	{

		if (ImGui::MenuItem("Create Entity"))
		{
			DoCreateEntity(Ptr<Firefly::Entity>());
		}
		ImGui::EndPopup();
	}
	myDragging = false;
	ImGui::InputTextWithHint("##HIERARCHY SEARCH BAR", "Search...", &mySearchString);
	ImGui::SameLine();
	if (ImGui::Selectable("Comp##SearchComponentInHier", mySearchComponent))
	{
		mySearchComponent = !mySearchComponent;
	}
	for (int i = 0; i < EditorLayer::GetEditingScenes().size(); i++)
	{
		auto scene = EditorLayer::GetEditingScenes()[i].lock();
		auto name = scene->GetPath().stem().string() + (scene->IsModified() ? "*" : "");
		if (name == "")
		{
			name = "Untitled";
		}
		if (ImGui::CollapsingHeader((name).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginPopupContextItem())
			{
				if (i != 0)
				{
					if (ImGui::MenuItem("UnloadScene"))
					{
						EditorLayer::UnloadScene(scene);
						ImGui::EndPopup();
						i--;
						continue;
					}
				}
				if (ImGui::MenuItem("Save Scene"))
				{
					EditorLayer::SaveScene(scene);
				}
				if (ImGui::MenuItem("Save Scene As"))
				{
					EditorLayer::SaveSceneAs(scene);
				}
				if (ImGui::MenuItem("Expand All"))
				{
					myExpandAll = true;
				}
				if (ImGui::MenuItem("Collapse All"))
				{
					myCollapseAll = true;
				}
				ImGui::EndPopup();
			}
			if (scene)
			{
				ImVec2 cursorPos = ImGui::GetCursorPos();
				ImVec2 elementSize = ImGui::GetItemRectSize();
				elementSize.x -= ImGui::GetStyle().FramePadding.x;
				elementSize.y = ImGui::GetStyle().FramePadding.y;
				cursorPos.y -= ImGui::GetStyle().FramePadding.y;
				ImVec2 windowPos = ImGui::GetWindowPos();
				myBetweenRects.emplace_back(ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y),
					ImVec2(windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y), 0, -1, scene.get(), false);
				auto entities = scene->GetEntities();

				for (int i = 0; i < entities.size(); i++)
				{
					if (entities[i].expired())
					{
						LOGERROR("HierarchyWindow::OnImGui: Entity is expired");
						continue;
					}
					if (!entities[i].lock()->HasParent())
					{
						FF_PROFILESCOPE("Draw root entity");
						int index = i;
						bool draw = true;

						if (draw)
						{
							DrawEntity(entities[i], index, -1);
						}
					}
				}
			}
		}
	}
	// handle hovering between entities
	if (myDragging)
	{
		myBetweenIndex = -1;


		for (int i = 0; i < myBetweenRects.size(); i++)
		{
			auto& rect = myBetweenRects[i];
			if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
			{
				myBetweenIndex = i;
			}
		}
		//if holding mouse below last entity select the last between rect
		if (myBetweenIndex == -1)
		{
			auto windowPos = ImGui::GetWindowPos();
			auto min = ImGui::GetWindowContentRegionMin();
			min.x += windowPos.x;
			min.y += windowPos.y;
			auto max = ImGui::GetWindowContentRegionMax();
			max.x += windowPos.x;
			max.y += windowPos.y;

			if (ImGui::IsMouseHoveringRect(min, max) && ImGui::GetMousePos().y > myBetweenRects.back().Max.y)
			{
				myBetweenIndex = myBetweenRects.size() - 1;
			}
		}
		//if we are currently editing a prefab, dont allow for unchilding by dropping above the first entity
		//also dont allow for dropping below last entity for unchilding
		if (EditorLayer::GetEditingPrefab())
		{
			if (myBetweenIndex == 0 || myBetweenIndex == myBetweenRects.size() - 1)
			{
				myBetweenIndex = -1;
			}
		}
	}

	if (myBetweenIndex != -1 && mySearchString.empty())
	{
		//draw a line between entities
		ImGui::GetWindowDrawList()->AddRectFilled(myBetweenRects[myBetweenIndex].Min, myBetweenRects[myBetweenIndex].Max, ImGui::GetColorU32(ImGuiCol_DragDropTarget));

		//handle dropping entity package between entities
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			EditorUtils::SetImGuiPayloadActive(true);
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY", ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				//collect the entity from the payload
				Ptr<Firefly::Entity> droppedEntity = *(Ptr<Firefly::Entity>*)payload->Data;
				OnDropEntityBetweenEntities(droppedEntity, myBetweenIndex);
			}
			else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITIES", ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				std::vector<Ptr<Firefly::Entity>>& droppedEntities = *(std::vector<Ptr<Firefly::Entity>>*)payload->Data;

				auto scenes = EditorLayer::GetEditingScenes();
				auto sceneEntities = scenes[0].lock()->GetEntities();
				for (int i = 1; i < scenes.size(); i++)
				{
					auto ents = scenes[i].lock()->GetEntities();
					sceneEntities.insert(sceneEntities.end(), ents.begin(), ents.end());
				}
				//order selected entities according to their index in the scene and 
				std::sort(droppedEntities.begin(), droppedEntities.end(),
					[=](Ptr<Firefly::Entity> a, Ptr<Firefly::Entity> b)
					{
						int aIndex = std::find_if(sceneEntities.begin(), sceneEntities.end(), [=](Ptr<Firefly::Entity> ent) {return ent.lock()->GetID() == a.lock()->GetID(); }) - sceneEntities.begin();
						int bIndex = std::find_if(sceneEntities.begin(), sceneEntities.end(), [=](Ptr<Firefly::Entity> ent) {return ent.lock()->GetID() == b.lock()->GetID(); }) - sceneEntities.begin();

						return aIndex < bIndex;
					});

				EditorLayer::BeginEntityUndoSeries();
				int betweenIndex = myBetweenIndex;
				for (int i = droppedEntities.size() - 1; i >= 0; i--)
				{
					int droppedSceneIndex = std::find_if(sceneEntities.begin(), sceneEntities.end(),
						[=](Ptr<Firefly::Entity> ent) {return ent.lock()->GetID() == droppedEntities[i].lock()->GetID(); }) - sceneEntities.begin();
					OnDropEntityBetweenEntities(droppedEntities[i], betweenIndex);
					if (droppedSceneIndex < betweenIndex) // have to account for the moved entities above
					{
						//betweenIndex--;
					}
				}
				EditorLayer::EndEntityUndoSeries();
			}
			EditorUtils::SetImGuiPayloadActive(false);
			myBetweenIndex = -1;
		}
	}

	myBetweenRects.clear();
	myExpandAll = false;
	myCollapseAll = false;
	myOpenAllParents = false;

	if (EditorUtils::AcceptAllDragDrops())
	{
		SetFocused();
	}
}

void HierarchyWindow::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<EntitySelectedEvent>([&](EntitySelectedEvent& e)
		{
			if (EditorLayer::GetSelectedEntities().size() == 1)
			{
				myAlreadyAutoScrolled = false;
			}

			myOpenAllParents = true;
			return false;
		});
}

void HierarchyWindow::ScrollToSelected()
{
	myAlreadyAutoScrolled = false;
	myOpenAllParents = true;
}

void HierarchyWindow::DrawEntity(Ptr<Firefly::Entity> aEntityWeak, int& aIndexInScene, int aParentIndexInScene)
{
	auto drawingEntity = aEntityWeak.lock();
	bool draw = true;
	if (!mySearchString.empty())
	{
		draw = false;
		auto children = drawingEntity->GetChildrenRecursive();

		if (mySearchComponent)
		{
			for (auto child : children)
			{
				if (!child.expired())
				{
					for (auto comp : child.lock()->GetComponents())
					{
						if (comp.lock()->GetName().find(mySearchString) != std::string::npos)
						{
							draw = true;
							break;
						}
					}
				}
			}
			for (auto comp : drawingEntity->GetComponents())
			{
				if (comp.lock()->GetName().find(mySearchString) != std::string::npos)
				{
					draw = true;
					break;
				}
			}
		}
		else
		{
			for (auto child : children)
			{
				if (!child.expired())
				{
					if (child.lock()->GetName().find(mySearchString) != std::string::npos)
					{
						draw = true;
						break;
					}
				}
			}
			if (drawingEntity->GetName().find(mySearchString) != std::string::npos)
			{
				draw = true;
			}
		}
	}

	if (!draw)
	{
		return;
	}


	auto id = drawingEntity->GetName() /*+" Index: " + std::to_string(itEntIndex) +*/ + "##HierarchyEntity" + std::to_string(drawingEntity->GetID());
	int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	const auto& selectedEntities = EditorLayer::GetSelectedEntities();
	if (selectedEntities.empty())
	{
		myAlreadyAutoScrolled = false;
	}

	//If entity selected, draw as selected
	bool entitySelected = false;
	if (EditorLayer::IsEntitySelected(drawingEntity->GetID()))
	{
		entitySelected = true;
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	bool isMarkedForMove = false;
	for (int i = 0; i < myMovingEntitiesWithMoveTo.size(); i++)
	{
		if (!myMovingEntitiesWithMoveTo[i].expired())
		{
			if (myMovingEntitiesWithMoveTo[i].lock()->GetID() == drawingEntity->GetID())
			{
				isMarkedForMove = true;
				break;
			}
		}
	}

	if (myOpenAllParents)
	{
		if (entitySelected)
		{
			//Scroll to selected entity
			if (!myAlreadyAutoScrolled)
			{
				ImGui::SetScrollHereY();
				myAlreadyAutoScrolled = true;
			}
		}

		//Open parent if it has selected children recursive
		if (EditorLayer::HasSelectedChildren(drawingEntity))
		{
			ImGui::SetNextItemOpen(true);
		}
	}

	//if Entity has no children, dont draw an arrow to the left of the name
	if (!drawingEntity->HasChildren())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
		auto cursorYPos = ImGui::GetCursorPosY();
		auto scrollY = ImGui::GetScrollY();
		auto topPos = scrollY;
		auto bottomPos = topPos + ImGui::GetWindowSize().y;
		if (cursorYPos < topPos || cursorYPos > bottomPos)
		{
			ImGui::TreeNodeEx(("FAKE: " + drawingEntity->GetName()).c_str(), ImGuiTreeNodeFlags_Leaf);
			ImGui::TreePop();
			return;
		}
	}

	//find what index the entity has in it's scene 
	int entityIndexInScene = aIndexInScene;

	Utils::Vector4f entityColor(1.0f, 1.0f, 1.0f, 1.0f);
	bool isPrefabRoot = drawingEntity->IsPrefab() && drawingEntity->GetPrefabRootEntityID() == drawingEntity->GetID();

	//if entity is a prefab change text to light blue 
	if (drawingEntity->IsPrefab())
	{
		if (isPrefabRoot)
		{
			entityColor = Utils::Vector4f(0.4f, 0.6f, 1.f, 1.0f);
		}
		else
		{
			entityColor = Utils::Vector4f(0.3f, 0.5f, 0.8f, 1.0f);
		}
	}

	//If entity is inactive, lower alpha of color
	if (!drawingEntity->GetIsActive())
	{
		entityColor = Utils::Vector4f(1.0f, 1.0f, 1.0f, 0.4f);
	}

	//Complete the command to expand/collapse all entities
	if (myExpandAll)
	{
		ImGui::SetNextItemOpen(true);
	}
	else if (myCollapseAll)
	{
		ImGui::SetNextItemOpen(false);
	}

	//Color the entity in the hierarchy
	ImVec4 imColor = { entityColor.x, entityColor.y, entityColor.z, entityColor.w };
	ImGui::PushStyleColor(ImGuiCol_Text, imColor);
	if (isPrefabRoot)
	{
		ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
	}
	bool isOpen = ImGui::TreeNodeEx(id.c_str(), flags);
	if (isPrefabRoot)
	{
		ImGuiUtils::PopFont();
	}

	if (isMarkedForMove)
	{
		auto lastItemSameLinePosX = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.x + 10.f;
		auto lastItemSameLinePosY = ImGui::GetCurrentWindow()->DC.CursorPosPrevLine.y + ImGui::CalcTextSize("").y / 2.f + ImGui::GetStyle().FramePadding.y / 2.f;
		ImGui::GetWindowDrawList()->AddCircleFilled({ lastItemSameLinePosX, lastItemSameLinePosY }, 5.f, ImGui::GetColorU32({ 1.f,0.8f,0.f,1.f }));
	}

	ImGui::PopStyleColor();

	// Track the in-between size for this tree node and add it to the list
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 elementSize = ImGui::GetItemRectSize();
	elementSize.x -= ImGui::GetStyle().FramePadding.x;
	elementSize.y = ImGui::GetStyle().FramePadding.y;
	cursorPos.y -= ImGui::GetStyle().FramePadding.y;
	ImVec2 windowPos = ImGui::GetWindowPos();
	int betweenRectParentIndex = -1;
	if (drawingEntity->HasChildren() && isOpen)
	{
		betweenRectParentIndex = entityIndexInScene;
	}
	else if (drawingEntity->GetParentID() != 0)
	{
		betweenRectParentIndex = aParentIndexInScene;
	}

	myBetweenRects.emplace_back(ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y),
		ImVec2(windowPos.x + cursorPos.x + elementSize.x, windowPos.y + cursorPos.y + elementSize.y),
		entityIndexInScene + 1, betweenRectParentIndex, drawingEntity->GetParentScene(), isOpen && drawingEntity->HasChildren());

	if (ImGui::BeginDragDropSource())
	{

		std::string displayText = "";
		if (EditorLayer::GetSelectedEntities().size() > 1 && entitySelected)
		{
			ImGui::SetDragDropPayload("ENTITIES", &EditorLayer::GetSelectedEntities(),
				sizeof(std::vector<Ptr<Firefly::Entity>>));
			displayText = std::to_string(EditorLayer::GetSelectedEntities().size()) + " Entities";
		}
		else
		{
			ImGui::SetDragDropPayload("ENTITY", &aEntityWeak, sizeof(Ptr<Firefly::Entity>));
			displayText = drawingEntity->GetName();
		}
		ImGui::Text(displayText.c_str());
		myDragging = true;
		ImGui::EndDragDropSource();
	}

	//handle dropping other entities on this entity
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			if (!drawingEntity->HasChildren())
			{
				ImGui::TreeNodeSetOpen(ImGui::GetID(id.c_str()), true);
			}
			//collect the dropped entity
			Ptr<Firefly::Entity> droppedEntity = *(Ptr<Firefly::Entity>*)payload->Data;
			OnDropEntityOnEntity(droppedEntity, drawingEntity);
		}
		else if (!entitySelected) //dont allow dropping on a selected entity
		{
			//Accept multi selected entity drop
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITIES"))
			{
				if (!drawingEntity->HasChildren())
				{
					ImGui::TreeNodeSetOpen(ImGui::GetID(id.c_str()), true);
				}
				//get the droppedEntities
				std::vector<Ptr<Firefly::Entity>>& droppedEntities = *(std::vector<Ptr<Firefly::Entity>>*)payload->Data;
				//Begin a undo series
				//order selected entities according to their index in the scene
				auto sceneEntities = drawingEntity->GetParentScene()->GetEntities();
				std::sort(droppedEntities.begin(), droppedEntities.end(),
					[=](Ptr<Firefly::Entity> a, Ptr<Firefly::Entity> b)
					{
						int aIndex = std::find_if(sceneEntities.begin(), sceneEntities.end(), [a](Ptr<Firefly::Entity> aEntityToCompare) { return a.lock()->GetID() == aEntityToCompare.lock()->GetID(); }) - sceneEntities.begin();
						int bIndex = std::find_if(sceneEntities.begin(), sceneEntities.end(), [b](Ptr<Firefly::Entity> aEntityToCompare) { return b.lock()->GetID() == aEntityToCompare.lock()->GetID(); }) - sceneEntities.begin();
						return aIndex < bIndex;
					});
				EditorLayer::BeginEntityUndoSeries();
				//Do the drop logic for all dropped entities
				for (auto droppedEntity : droppedEntities)
				{
					OnDropEntityOnEntity(droppedEntity, drawingEntity);
				}
				// End the undo series
				EditorLayer::EndEntityUndoSeries();
			}
		}
		ImGui::EndDragDropTarget();
	}



	//Ignore selecting entity when clicking on arrow
	auto min = ImGui::GetCurrentContext()->LastItemData.Rect.Min;
	min.x += 25; // offset to ignore arrow, magic number dont touch

	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !myDragging && ImGui::IsMouseHoveringRect(min, ImGui::GetCurrentContext()->LastItemData.Rect.Max))
	{
		//TODO: Fix shift select selecting children
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
		{
			if (EditorLayer::GetSelectedEntities().size() > 0)
			{
				auto lastSelectedEntity = EditorLayer::GetLastSelectedEntity();
				auto sceneEntities = drawingEntity->GetParentScene()->GetEntities();
				auto itEnt = std::find_if(sceneEntities.begin(), sceneEntities.end(),
					[drawingEntity](Ptr<Firefly::Entity> aEntityToCompare) { return drawingEntity->GetID() == aEntityToCompare.lock()->GetID(); });

				int itEntIndex = itEnt - sceneEntities.begin();
				auto itLastEnt = std::find_if(sceneEntities.begin(), sceneEntities.end(),
					[lastSelectedEntity](Ptr<Firefly::Entity> aEntityToCompare) { return lastSelectedEntity.lock()->GetID() == aEntityToCompare.lock()->GetID(); });
				int itLastEntIndex = itLastEnt - sceneEntities.begin();

				EditorLayer::BeginEntityUndoSeries();

				if (itEntIndex < itLastEntIndex)
				{
					for (int i = itEntIndex; i < itLastEntIndex; i++)
					{
						//EditorLayer::AddSelectedEntity(sceneEntities[i]);
						const auto select = CreateRef<EntitySelectCommand>(sceneEntities[i], true);
						EditorLayer::ExecuteAndAddEntityUndo(select);
					}
				}
				else
				{
					for (int i = itLastEntIndex + 1; i <= itEntIndex; i++)
					{
						//EditorLayer::AddSelectedEntity(sceneEntities[i]);
						const auto select = CreateRef<EntitySelectCommand>(sceneEntities[i], true);
						EditorLayer::ExecuteAndAddEntityUndo(select);
					}
				}

				EditorLayer::EndEntityUndoSeries();
			}
			else
			{
				//EditorLayer::SelectEntity(drawingEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
				const auto select = CreateRef<EntitySelectCommand>(drawingEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
				EditorLayer::ExecuteAndAddEntityUndo(select);
			}
		}
		else
		{
			//EditorLayer::SelectEntity(drawingEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
			const auto select = CreateRef<EntitySelectCommand>(drawingEntity, Utils::InputHandler::GetKeyHeld(VK_CONTROL));
			EditorLayer::ExecuteAndAddEntityUndo(select);
		}
	}
	//on right click entity
	if (ImGui::BeginPopupContextItem())
	{
		if (!entitySelected)
		{
			EditorLayer::SelectEntity(drawingEntity);
		}

		bool singleEntity = selectedEntities.size() == 1;

		if (singleEntity && drawingEntity->IsPrefab() && drawingEntity->GetPrefabRootEntityID() == drawingEntity->GetID())
		{
			if (ImGui::MenuItem("Unpack Prefab"))
			{
				drawingEntity->UnpackPrefab();
			}

			if (ImGui::MenuItem("Show Prefab in Content Browser"))
			{
				auto prefab = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(drawingEntity->GetPrefabID());
				EditorLayer::GetOrCreateWindow<ContentBrowser>()->SelectEntry(prefab->GetPath());
			}
		}



		if (singleEntity && ImGui::MenuItem("Create Entity"))
		{
			DoCreateEntity(drawingEntity);
			ImGui::TreeNodeSetOpen(ImGui::GetID(id.c_str()), true);
		}

		if (ImGui::MenuItem("Delete"))
		{
			EditorLayer::DeleteSelectedEntities();

			myEntityDeletedFlag = true;
			ImGui::EndPopup();
			return;
		}

		//if (ImGui::MenuItem("Rename") && singleEntity)
		//{
		//	//EditorLayer::RenameSelectedEntity();

		//	ImGui::EndPopup();
		//	return;
		//}

		if (ImGui::MenuItem("Duplicate"))
		{
			EditorLayer::DuplicateSelectedEntities();

			ImGui::EndPopup();
			return;
		}

		if (ImGui::MenuItem(("Mark " + std::to_string(selectedEntities.size()) + " Selected For Move...").c_str()))
		{
			myMovingEntitiesWithMoveTo = selectedEntities;
		}

		if (!isMarkedForMove && !myMovingEntitiesWithMoveTo.empty() && singleEntity &&
			ImGui::MenuItem(("Move " + std::to_string(myMovingEntitiesWithMoveTo.size()) + " Marked Here.").c_str()))
		{
			for (auto ent : myMovingEntitiesWithMoveTo)
			{
				if (!ent.expired())
				{
					if (!Firefly::GetEntityWithID(ent.lock()->GetID()).expired())
					{
						OnDropEntityOnEntity(ent, drawingEntity);
					}
				}
			}
			myMovingEntitiesWithMoveTo.clear();
		}
		if (singleEntity && ImGui::MenuItem("Copy World Transform Data"))
		{
			//shell command
			std::string command = "Position:" + std::to_string(drawingEntity->GetTransform().GetPosition().x)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetPosition().y)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetPosition().z)
				+ " \nRotation:"
				+ std::to_string(drawingEntity->GetTransform().GetRotation().x)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetRotation().y)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetRotation().z)
				+ " \nScale:"
				+ std::to_string(drawingEntity->GetTransform().GetScale().x)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetScale().y)
				+ " "
				+ std::to_string(drawingEntity->GetTransform().GetScale().z);

			OpenClipboard(Firefly::Application::GetWindow()->GetHandle());
			EmptyClipboard();
			HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, command.size() + 1);
			if (!hg) {
				CloseClipboard();
				return;
			}
			memcpy(GlobalLock(hg), command.c_str(), command.size() + 1);
			GlobalUnlock(hg);
			SetClipboardData(CF_TEXT, hg);
			CloseClipboard();
			GlobalFree(hg);
		}



		ImGui::EndPopup();
	}

	//ImGui::PushStyleColor(ImGuiCol_Text, imColor);
	//if (aEntity->IsPrefab() && aEntity->GetPrefabRootEntityID() == aEntity->GetID())
	//{
	//	ImGui::SameLine();
	//	ImGui::Text("<<prefab root>>");
	//}
	//ImGui::PopStyleColor();
	if (isOpen)
	{
		ImGui::TreePush(id.c_str());
		auto children = drawingEntity->GetChildren();
		for (int i = 0; i < children.size(); ++i)
		{
			aIndexInScene++; // we can add here because we always know that the children are recursively indexed after their parent
			DrawEntity(children[i], aIndexInScene, entityIndexInScene);
			if (myEntityDeletedFlag)
			{
				i--;
				myEntityDeletedFlag = false;
				//we might not need to subtract from index here since we wont have time to drop something the same frame we delete something
			}
		}

		ImGui::TreePop();
	}
}

bool HierarchyWindow::CheckIfCanReorderEntity(Ptr<Firefly::Entity> aEntity)
{
	auto entity = aEntity.lock();
	//if we arent in prefab edito mode
	//and we dropped a prefab instance that is not the root of the prefab dont allow that movement
	if (!EditorLayer::GetEditingPrefab() &&
		entity->IsPrefab() && entity->GetPrefabRootEntityID() != entity->GetID())
	{
		ImGuiUtils::OpenYesNoPopupModal("Cannot Reorder the child of a prefab instance!",
			"If you want to reorder the child of a prefab you have to edit the prefab itself. Do you want to open prefab for edit?",
			[=]()
			{
				PrefabAssetOpenForEditEvent ev(entity->GetPrefabRoot().lock()->GetPrefabID());
				Firefly::Application::Get().OnEvent(ev);
			},
			[=]()
			{
				//do nothing
			});
		return false;
	}
	return true;
}

void HierarchyWindow::OnDropEntityBetweenEntities(Ptr<Firefly::Entity> aDroppedEntity, int aTargetBetweenIndex)
{
	if (!CheckIfCanReorderEntity(aDroppedEntity))
	{
		return;
	}

	const auto& droppedEntity = aDroppedEntity.lock();

	auto fromSceneEntities = droppedEntity->GetParentScene()->GetEntities();
	auto toSceneEntities = myBetweenRects[aTargetBetweenIndex].Scene->GetEntities();

	auto  droppedEntityIt = std::find_if(fromSceneEntities.begin(), fromSceneEntities.end(), [=](const Ptr<Firefly::Entity>& aEntity)
		{
			return aEntity.lock()->GetID() == droppedEntity->GetID();
		});
	uint32_t droppedEntityIndex = droppedEntityIt - fromSceneEntities.begin();

	int insertIndex = myBetweenRects[aTargetBetweenIndex].InsertIndex;
	int parentIndex = myBetweenRects[aTargetBetweenIndex].ParentIndex;
	//find new parent
	Ptr<Firefly::Entity> parent;
	if (parentIndex != -1)
	{
		parent = toSceneEntities[myBetweenRects[aTargetBetweenIndex].ParentIndex];
	}

	//check if parent is valid
	bool invalid = false;
	if (parent.lock())
	{
		auto testParent = parent.lock();
		while (testParent)
		{
			if (testParent->GetID() == droppedEntity->GetID())
			{
				invalid = true;
				break;
			}
			testParent = testParent->GetParent().lock();
		}
	}

	if (!invalid)
	{
		uint32_t indexToRemoveAt = droppedEntityIt - fromSceneEntities.begin();

		//find what index the child should be at
		int childIndex = -1;
		if (parent.lock())
		{
			auto childrenOfParent = parent.lock()->GetChildren();

			if (insertIndex >= toSceneEntities.size())
			{
				//benne fix crash, drop at end of hierarchy, end of children
				childIndex = childrenOfParent.size();
			}
			else
			{
				//find what iterator in the parent's children the entity at the dropped position is at relative to the scene entities
				auto insertEntityInChildrenIt = std::find_if(childrenOfParent.begin(), childrenOfParent.end(), [toSceneEntities, insertIndex](const Ptr<Firefly::Entity>& aEntity)
					{
						return aEntity.lock()->GetID() == toSceneEntities[insertIndex].lock()->GetID();
					});
				childIndex = static_cast<int>(insertEntityInChildrenIt - childrenOfParent.begin());
			}

			//if the entity was dropped on it's own parent adjust the child index accordingly
			if (droppedEntity->GetParentID() != 0)
			{
				if (droppedEntity->GetParent().lock()->GetID() == parent.lock()->GetID())
				{
					//find the current index of the droppedEntity in the parent's children
					uint32_t currentChildIndex = std::find_if(childrenOfParent.begin(), childrenOfParent.end(), [aDroppedEntity](Ptr<Firefly::Entity> ent) {return ent.lock()->GetID() == aDroppedEntity.lock()->GetID(); }) - childrenOfParent.begin();
					//if the current child index is less than(is before) the child index previously calculated
					//then we have to subtract one from child index since it will change the index of the other children when removed
					if (childIndex > currentChildIndex)
					{
						childIndex--;
					}
				}
			}
		}
		//Add move command
		EditorLayer::ExecuteAndAddEntityUndo(std::make_shared<EntityReorderCommand>(indexToRemoveAt, insertIndex,
			parent, childIndex, aDroppedEntity.lock()->GetParentScene(), myBetweenRects[myBetweenIndex].Scene));
	}
}

void HierarchyWindow::OnDropEntityOnEntity(Ptr<Firefly::Entity> aDroppedEntityWeak, Ptr<Firefly::Entity> aTargetEntityWeak)
{
	auto droppedEntity = aDroppedEntityWeak.lock();
	auto targetEntity = aTargetEntityWeak.lock();
	if (!CheckIfCanReorderEntity(droppedEntity))
	{
		return;
	}

	//get the vector of entities from the current scene
	auto targetSceneEntities = targetEntity->GetParentScene()->GetEntities();
	auto fromSceneEntities = droppedEntity->GetParentScene()->GetEntities();

	//make sure you arent setting an entity as its own child
	auto testParent = targetEntity->GetParent().lock();
	bool invalid = false;
	while (testParent)
	{
		if (testParent == droppedEntity)
		{
			invalid = true;
			break;
		}
		testParent = testParent->GetParent().lock();
	}

	//if this entity is valid as a parent to droppedEntity
	if (!invalid)
	{
		//find what index the dropped entiy has in the old scene
		auto droppedEntityIt = std::find_if(fromSceneEntities.begin(), fromSceneEntities.end(), [droppedEntity](const Ptr<Firefly::Entity>& aEntity)
			{
				return aEntity.lock()->GetID() == droppedEntity->GetID();
			});
		uint32_t droppedEntityIndex = droppedEntityIt - fromSceneEntities.begin();

		//find what index the target entity has in the new scene
		auto parentIt = std::find_if(targetSceneEntities.begin(), targetSceneEntities.end(), [targetEntity](const Ptr<Firefly::Entity>& aEntity)
			{
				return aEntity.lock()->GetID() == targetEntity->GetID();
			});
		uint32_t parentItIndex = parentIt - targetSceneEntities.begin();

		//calculate what index in the scene corresponds to putting the child after the last child in the children list
		//the index of the parent in the scene + 1 since we want the index of the entity right after the parent to get to the first child
		//then offset by the recursive child count to get the last index
		int insertIndex = parentItIndex + 1 + targetEntity->GetRecursiveChildrenCount();

		//calculate the local child index
		int childIndex = -1;
		auto childrenOfParent = targetEntity->GetChildren();
		childIndex = childrenOfParent.size();

		//if dropped on its own parent, we have to subtract one since the entity will be removed from above the insert index
		if (droppedEntity->GetParent().lock() == targetEntity)
		{
			childIndex--;
		}
		//Move Entity
		EditorLayer::ExecuteAndAddEntityUndo(
			std::make_shared<EntityReorderCommand>(droppedEntityIndex, insertIndex,
				targetEntity, childIndex, droppedEntity->GetParentScene(), targetEntity->GetParentScene()));
	}
}

Ptr<Firefly::Entity> HierarchyWindow::DoCreateEntity(Ptr<Firefly::Entity> aParentEntity)
{
	auto parent = aParentEntity.lock();
	bool isPrefabEditor = EditorLayer::GetEditingPrefab().get() != nullptr;
	if (isPrefabEditor && parent == nullptr) // set parent to root entity if is currently editing prefab
	{
		parent = EditorLayer::GetEditingScenes()[0].lock()->GetEntities()[0].lock();
	}
	Firefly::Scene* targetScene = parent ? parent->GetParentScene() : EditorLayer::GetEditingScenes()[0].lock().get();
	auto command = std::make_shared<EntityCreateCommand>(parent, targetScene, isPrefabEditor, true);
	EditorLayer::ExecuteAndAddEntityUndo(command);
	return command->GetCreatedEntity();
}
