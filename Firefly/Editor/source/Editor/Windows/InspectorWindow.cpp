#include "EditorPch.h"
#include "InspectorWindow.h"

#include <Editor/UndoSystem/Commands/SceneCommands/EntityCreateCommand.h>
#include <Firefly/ComponentSystem/Scene.h>
#include <Firefly/ComponentSystem/SceneManager.h>

#include "Editor/EditorLayer.h"
#include "Editor/Utilities/EditorUtils.h"
#include "Editor/UndoSystem/UndoHandler.h"
#include "Editor/UndoSystem/Commands/EntityCommands/TransformCommand.h"
#include "Editor/UndoSystem/UndoHandler.h"
#include "Editor/UndoSystem/Commands/EntityCommands/ComponentAddCommand.h"

#include "Editor/Event/EditorOnlyEvents.h"
#include "Editor/UndoSystem/Commands/EntityCommands/AbsoluteTransformCommand.h"

#include "Utils/StringUtils.hpp"

#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Component.h"

#include "Editor/Utilities/ImGuiUtils.h"

#include "Firefly/Event/EntityEvents.h"

#include "Editor/Windows/WindowRegistry.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"

#include "Utils/InputHandler.h"

#include "Firefly/ComponentSystem/EntityTags.h"
#include <Firefly/Components/Mesh/MeshComponent.h>

REGISTER_WINDOW(InspectorWindow);
InspectorWindow::InspectorWindow() : EditorWindow("Inspector")
{
	myResetIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_reset.dds");
	myCopyIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_copy.dds");
	myPasteIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_paste.dds");
	myEntityFolderIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_entityfolder_add.dds");
	myMaterialSwapIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_swapmaterial.dds");
}

void InspectorWindow::OnImGui()
{
	const auto entityCount = EditorLayer::GetSelectedEntities().size();
	if (entityCount == 0)
	{
		ImGui::Text("No entities selected");
	}
	else if (entityCount == 1)
	{
		const auto& firstWeak = EditorLayer::GetFirstSelectedEntity();
		if (firstWeak.expired())
		{
			return;
		}
		const auto& firstSelected = firstWeak.lock();

		DrawEntity(firstSelected);

		bool addCompPressed = ImGui::Button("AddComponent");

		if (!firstSelected->IsPrefab() || EditorLayer::GetEditingPrefab())
		{
			static bool wasOpen = false;
			ImGui::SetNextWindowSize({ ImGui::GetWindowWidth(), 300 });
			if (ImGui::BeginPopupContextItem(std::to_string(firstSelected->GetID()).c_str(), ImGuiPopupFlags_MouseButtonLeft))
			{
				auto& compRegMap = Firefly::ComponentRegistry::GetFactoryMap();

				// Sort component map when adding component
				std::vector<std::string> keys;
				keys.reserve(compRegMap.size());
				{
					for (auto it = compRegMap.begin(); it != compRegMap.end(); ++it)
					{
						keys.push_back(it->first);
					}
					std::sort(keys.begin(), keys.end(), [&](auto first, auto second) {return first < second; });
				}
				if (!wasOpen)
				{
					ImGui::SetKeyboardFocusHere();
					mySearchComponentString = "";
				}
				ImGui::InputTextWithHint("##AddComponentSearchBar", "Search...", &mySearchComponentString);
				ImGui::BeginChild("##InspectorAddComponentChild");
				std::string searchStr = Utils::ToLower(mySearchComponentString);
				//iterator for-loop with compRegMap
				for (std::string& key : keys)
				{
					if (Utils::ToLower(key).find(searchStr) != std::string::npos)
					{
						if (ImGui::MenuItem(key.c_str()))
						{
							EditorLayer::ExecuteAndAddEntityUndo(std::make_shared<ComponentAddCommand>(firstSelected, key));
							ImGui::CloseCurrentPopup();
						}
					}
				}
				ImGui::EndChild();
				wasOpen = true;
				ImGui::EndPopup();
			}
			else
			{
				wasOpen = false;
			}
		}
		else if (addCompPressed)
		{
			ImGuiUtils::OpenYesNoPopupModal("Cannot add component to prefab locally!",
				"You cannot add components to prefabs outside of the prefab editing mode. Do you want to open the prefab for editing?.",
				[&]()
				{
					PrefabAssetOpenForEditEvent ev(firstSelected->GetPrefabRoot().lock()->GetPrefabID());
					Firefly::Application::Get().OnEvent(ev);
				},
				[&]()
				{
					//do nothing if press no
				});
		}
	}
	else if (entityCount > 1)
	{
		ImGui::Text((std::to_string(entityCount) + " entities selected").c_str());
		ImGui::Text("Cannot edit multiple entities at once in the inspector(yet o.o)");

		if (ImGui::ImageButton("##CreateEmptyEntityFolder", myEntityFolderIcon->GetSRV().Get(), { 100, 100 }))
		{
			EditorLayer::CreateEntityFolderForSelectedEntities();
		}
		ImGui::SameLine();
		ImGui::Text("Create Empty Entity Folder containing current selected entities");
		if (mySwapMaterialpath.empty() == false)
		{

			if (ImGui::ImageButton("##SwapMaterialIcon", myMaterialSwapIcon->GetSRV().Get(), { 100, 100 }))
			{
				auto& selectedEntities = EditorLayer::GetSelectedEntities();

				for (auto& entity : selectedEntities)
				{
					if (entity.expired())
					{
						continue;
					}

					auto entityPtr = entity.lock();



					if (!entityPtr->HasComponent<Firefly::MeshComponent>())
					{
						continue;
					}

					auto meshComponent = entityPtr->GetComponent<Firefly::MeshComponent>();

					if (meshComponent.expired())
					{
						continue;
					}

					if (entityPtr->IsPrefab())
					{
						Firefly::EntityModification entityMod;

						entityMod.ID = entityPtr->GetID();
						entityMod.StringValues.push_back(mySwapMaterialpath);
						entityMod.Key = meshComponent.lock()->GetName() + "_" + "Material Paths";
						entityPtr->AddModification(entityMod, true);

						Firefly::EntityPropertyUpdatedEvent ev("Material Paths", Firefly::ParameterType::List);
						entityPtr->OnEvent(ev);
					}
					meshComponent.lock()->SetMaterialPath(mySwapMaterialpath);
				}
			}
		}
		ImGuiUtils::BeginParameters("SwapMaterials");
		ImGuiUtils::FileParameter("Material", mySwapMaterialpath, ".mat");
		ImGuiUtils::EndParameters();
	}
}

void InspectorWindow::DrawEntity(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		return;
	}

	auto entity = aEntity.lock();
	std::string entityID = std::to_string(entity->GetID());
	ImGui::InputText(("Name##" + entityID).c_str(), &const_cast<std::string&>(entity->GetName()));
	//todo: if inspector laggy, move this to a member variable
	std::vector<std::string> tags;
	const auto tagCount = static_cast<uint32_t>(Firefly::EntityTag::COUNT);
	tags.resize(tagCount);
	for (uint32_t i = 0; i < static_cast<uint32_t>(Firefly::EntityTag::COUNT); i++)
	{
		tags[i] = Firefly::EntityTagToString(static_cast<Firefly::EntityTag>(i));
	}
	uint32_t tagNum = static_cast<uint32_t>(Firefly::StringToEntityTag(entity->GetTag()));
	if (ImGuiUtils::Combo(("Tag##" + entityID).c_str(), tagNum, tags, 0))
	{
		entity->SetTag(tags[tagNum]);
	}

	//Transform Changes through inspector
	UpdateTransformComponent(aEntity);

	const auto& components = entity->GetComponents();
	for (int i = 0; i < components.size(); i++)
	{
		if (components[i].expired())
		{
			continue;
		}

		auto component = components[i].lock();
		const auto removeButtonSize = ImGui::CalcTextSize("").y + ImGui::GetStyle().FramePadding.y * 2.f;
		ImGui::BeginTable(("RemoveComponentTable" + component->GetName()).c_str(), 2);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x - removeButtonSize);
		ImGui::TableNextColumn();
		bool headerOpen = ImGui::CollapsingHeader(component->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::TableNextColumn();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(181 / 255.f, 55 / 255.f, 55 / 255.f, 1));
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - removeButtonSize / 2.f);

		const bool shouldRemoveComponent = ImGui::Button(("-##RemoveComponentButton" + entityID + std::to_string(i)).c_str(), { removeButtonSize ,removeButtonSize });
		ImGui::EndTable();
		ImGui::PopStyleColor();
		if (shouldRemoveComponent)
		{
			if (!EditorLayer::GetFirstSelectedEntity().lock()->IsPrefab() || EditorLayer::GetEditingPrefab())
			{
				entity->RemoveComponent(components[i]);
				i--;
				continue;
			}
			else
			{
				ImGuiUtils::OpenYesNoPopupModal("Cannot remove component from prefab locally!",
					"You cannot remove components from prefabs outside of the prefab editing mode. Do you want to open the prefab for editing?.",
					[&]()
					{
						PrefabAssetOpenForEditEvent ev(EditorLayer::GetFirstSelectedEntity().lock()->GetPrefabRoot().lock()->GetPrefabID());
						Firefly::Application::Get().OnEvent(ev);
					},
					[&]()
					{
						//do nothing if press no
					});
			}
		}

		if (headerOpen)
		{
			auto& variables = component->mySerializedVariables;

			int treeOpenCount = 0;
			bool treeOpen = true;
			ImGuiUtils::BeginParameters((component->GetName() + std::to_string(entity->GetID())).c_str(), true, component);
			for (int j = 0; j < variables.size(); j++)
			{
				auto& variable = variables[j];

				if (variable.Hidden)
				{
					continue;
				}

				if (variable.Type == Firefly::ParameterType::HeaderEnd)
				{
					if (variable.HeaderDepth == treeOpenCount)
					{
						ImGuiUtils::EndHeaderParameter();
						treeOpen = true;
						treeOpenCount--;
					}
				}
				if (treeOpen)
				{
					if (variable.Type == Firefly::ParameterType::Header)
					{
						treeOpen = ImGuiUtils::HeaderParameter(variable.Name + "##" + std::to_string(entity->GetID()) + std::to_string(j));
						treeOpenCount++;
					}
					else
					{

						bool isPrefabParameterModified = false;
						if (entity->IsPrefab())
						{
							isPrefabParameterModified = entity->IsParameterModified(components[i].lock()->GetName() + "_" + variable.Name);
						}
						if (isPrefabParameterModified)
						{
							ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
							ImVec2 startPos = { ImGui::GetWindowPos().x, ImGui::GetCursorScreenPos().y };
							ImVec2 endPos = { startPos.x + 7 ,startPos.y + ImGui::CalcTextSize("").y };
							ImGui::GetWindowDrawList()->AddRectFilled(startPos, endPos, ImGui::GetColorU32({ 0.4f,0.6f,1.f,1.f }), 1);
						}

						bool changed = DrawVariable(variable);

						if (isPrefabParameterModified)
						{
							ImGuiUtils::PopFont();
							if (ImGuiUtils::BeginPopupRect(("##PrefabRevert" + std::to_string(entity->GetID()) + variable.Name).c_str(),
								{ ImGui::GetWindowPos().x,ImGui::GetItemRectMin().y }, { ImGui::GetItemRectMax().x,ImGui::GetItemRectMax().y }))
							{
								if (ImGui::MenuItem("Revert"))
								{
									entity->RevertModification(variable, components[i].lock().get());
								}
								ImGui::EndPopup();
							}
						}

						if (changed)
						{
							entity->GetParentScene()->SetAsModified();
							Firefly::EntityPropertyUpdatedEvent ev(variable.Name, variable.Type);
							entity->OnEvent(ev);

							//if we are changing the variable of a prefab while not in the prefab editor
							if (entity->IsPrefab() && !EditorLayer::GetEditingPrefab() && variable.Type != Firefly::ParameterType::Button)
							{
								auto root = entity->GetPrefabRoot();
								if (root.expired())
								{
									LOGERROR("Prefab root expired!");
									continue;
								}

								Firefly::EntityModification mod;
								mod.ID = entity->GetID();
								mod.Key = components[i].lock()->GetName() + "_" + variable.Name;
								if (variable.Type == Firefly::ParameterType::String || variable.Type == Firefly::ParameterType::File)
								{
									mod.StringValues.clear();
									mod.StringValues.push_back(*reinterpret_cast<std::string*>(variable.Pointer));
								}
								else if (variable.Type == Firefly::ParameterType::Vec2)
								{
									mod.FloatValues.clear();
									const auto& vec = *reinterpret_cast<Utils::Vector2f*>(variable.Pointer);
									mod.FloatValues.push_back(vec.x);
									mod.FloatValues.push_back(vec.y);

								}
								else if (variable.Type == Firefly::ParameterType::Vec3)
								{
									mod.FloatValues.clear();
									const auto& vec = *reinterpret_cast<Utils::Vector3f*>(variable.Pointer);
									mod.FloatValues.push_back(vec.x);
									mod.FloatValues.push_back(vec.y);
									mod.FloatValues.push_back(vec.z);
								}
								else if (variable.Type == Firefly::ParameterType::Vec4 || variable.Type == Firefly::ParameterType::Color)
								{
									mod.FloatValues.clear();
									const auto& vec = *reinterpret_cast<Utils::Vector4f*>(variable.Pointer);
									mod.FloatValues.push_back(vec.x);
									mod.FloatValues.push_back(vec.y);
									mod.FloatValues.push_back(vec.z);
									mod.FloatValues.push_back(vec.w);
								}
								else if (variable.Type == Firefly::ParameterType::Int)
								{
									mod.IntValues.clear();
									mod.IntValues.push_back(*reinterpret_cast<int*>(variable.Pointer));
								}
								else if (variable.Type == Firefly::ParameterType::Float)
								{
									mod.FloatValues.clear();
									mod.FloatValues.push_back(*reinterpret_cast<float*>(variable.Pointer));
								}
								else if (variable.Type == Firefly::ParameterType::Bool)
								{
									mod.UintValues.clear();
									mod.UintValues.push_back(*reinterpret_cast<bool*>(variable.Pointer));
								}
								else if (variable.Type == Firefly::ParameterType::Enum)
								{
									mod.UintValues.clear();
									mod.UintValues.push_back(*reinterpret_cast<uint32_t*>(variable.Pointer));
								}
								else if (variable.Type == Firefly::ParameterType::Entity)
								{
									mod.EntityIDValues.clear();

									auto ptr = *reinterpret_cast<Ptr<Firefly::Entity>*>(variable.Pointer);
									if (ptr.expired())
									{
										LOGERROR("Entity Paramater with key {} was NULL", mod.Key);
									}
									else
									{
										mod.EntityIDValues.push_back(ptr.lock()->GetID());
									}
								}
								else if (variable.Type == Firefly::ParameterType::List)
								{
									switch (variable.ListType)
									{

									case Firefly::ParameterType::File:
									case Firefly::ParameterType::String:
									{
										mod.StringValues.clear();
										const auto& list = *reinterpret_cast<std::vector<std::string>*>(variable.Pointer);
										for (const auto& str : list)
										{
											mod.StringValues.push_back(str);
										}
										break;
									}

									case Firefly::ParameterType::Vec2:
									{
										mod.FloatValues.clear();
										const auto& list = *reinterpret_cast<std::vector<Utils::Vector2f>*>(variable.Pointer);
										for (const auto& vec : list)
										{
											mod.FloatValues.push_back(vec.x);
											mod.FloatValues.push_back(vec.y);
										}
										break;
									}

									case Firefly::ParameterType::Vec3:
									{
										mod.FloatValues.clear();
										const auto& list = *reinterpret_cast<std::vector<Utils::Vector3f>*>(variable.Pointer);
										for (const auto& vec : list)
										{
											mod.FloatValues.push_back(vec.x);
											mod.FloatValues.push_back(vec.y);
											mod.FloatValues.push_back(vec.z);
										}
										break;
									}

									case Firefly::ParameterType::Vec4:
									case Firefly::ParameterType::Color:
									{
										mod.FloatValues.clear();
										const auto& list = *reinterpret_cast<std::vector<Utils::Vector4f>*>(variable.Pointer);
										for (const auto& vec : list)
										{
											mod.FloatValues.push_back(vec.x);
											mod.FloatValues.push_back(vec.y);
											mod.FloatValues.push_back(vec.z);
											mod.FloatValues.push_back(vec.w);
										}
										break;
									}

									case Firefly::ParameterType::Int:
									{
										mod.IntValues.clear();
										const auto& list = *reinterpret_cast<std::vector<int>*>(variable.Pointer);
										for (const auto& i : list)
										{
											mod.IntValues.push_back(i);
										}
										break;
									}

									case Firefly::ParameterType::Float:
									{
										mod.FloatValues.clear();
										const auto& list = *reinterpret_cast<std::vector<float>*>(variable.Pointer);
										for (const auto& f : list)
										{
											mod.FloatValues.push_back(f);
										}
										break;
									}

									case Firefly::ParameterType::Enum:
									{
										mod.UintValues.clear();
										const auto& list = *reinterpret_cast<std::vector<uint32_t>*>(variable.Pointer);
										for (const auto& u : list)
										{
											mod.UintValues.push_back(u);
										}
										break;
									}
									}

								}
								else
								{
									assert(false && "Unknown parameter type");
								}

								root.lock()->AddModification(mod, true);
							}
						}
					}
				}

			}
			for (int j = 0; j < treeOpenCount; j++)
			{
				ImGui::TreePop();
			}

			ImGuiUtils::EndParameters();
		}




	}
}
bool InspectorWindow::DrawVariable(Firefly::Variable& aVariable)
{
	using namespace Firefly;
	bool changed = false;
	switch (aVariable.Type)
	{
	case ParameterType::Int:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<int*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Float:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<float*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Bool:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<bool*>(aVariable.Pointer));
		break;
	}
	case ParameterType::String:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<std::string*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Vec2:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<Utils::Vector2f*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Vec3:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<Utils::Vector3f*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Vec4:
	{
		changed = ImGuiUtils::Parameter(aVariable.Name, *reinterpret_cast<Utils::Vector4f*>(aVariable.Pointer));
		break;
	}
	case ParameterType::Color:
	{

		changed = ImGuiUtils::ColorParameter(aVariable.Name, *reinterpret_cast<Utils::Vector4f*>(aVariable.Pointer), aVariable.UseAlpha);
		break;
	}
	case ParameterType::Button:
	{
		if (ImGuiUtils::Button(aVariable.Name))
		{
			aVariable.ButtonFunction();
		}
		break;
	}
	case ParameterType::Entity:
	{
		Ptr<Firefly::Entity>& value = *reinterpret_cast<Ptr<Firefly::Entity>*>(aVariable.Pointer);
		changed = ImGuiUtils::Parameter(aVariable.Name, value);

		if (changed)
		{
			aVariable.EntityID = value.lock()->GetID();
		}
		break;
	}
	case ParameterType::File:
	{
		changed = ImGuiUtils::FileParameter(aVariable.Name, *reinterpret_cast<std::string*>(aVariable.Pointer), aVariable.FileExtensions);
		break;
	}
	case ParameterType::Enum:
	{
		if (aVariable.EnumNames.empty())
		{

			std::vector<std::string> enumNames;
			for (int i = 0; i < aVariable.EnumCount; i++)
			{
				enumNames.push_back(aVariable.EnumToStringFunction(i));
			}
			changed = ImGuiUtils::EnumParameter(aVariable.Name, *reinterpret_cast<uint32_t*>(aVariable.Pointer), enumNames);
		}
		else
		{
			changed = ImGuiUtils::EnumParameter(aVariable.Name, *reinterpret_cast<uint32_t*>(aVariable.Pointer), aVariable.EnumNames);
		}
		break;
	}
	case ParameterType::List:
	{
		changed = DrawListVariable(aVariable);
		break;
	}


	default:
		break;
	}
	return changed;
}

bool InspectorWindow::DrawListVariable(Firefly::Variable& aListVariable)
{
	ImGui::TableNextColumn();
	bool changed = false;
	std::vector<uint8_t>& vec = *reinterpret_cast<std::vector<uint8_t>*>(aListVariable.Pointer);

	const auto byteSize = Firefly::GetByteSize(aListVariable.ListType);
	const auto size = vec.size() / byteSize;

	if (aListVariable.DefaultOpen)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	}

	const bool open = ImGui::CollapsingHeader(aListVariable.Name.c_str());

	ImGui::TableNextColumn();
	int vectorSize = size;
	if (ImGui::InputInt(("##ListSizeInputInt" + aListVariable.Name).c_str(), &vectorSize, 1, 1, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (vectorSize < 0)
		{
			vectorSize = 0;
		}
		if (vectorSize > 1024)
		{
			vectorSize = 1024;
		}
		vec.resize(vectorSize * byteSize);
		int addedElements = (vectorSize - size);
		int addedBytes = addedElements * byteSize;
		if (addedBytes > 0)
		{
			memset(&vec[(size)*byteSize], 0, addedBytes);
		}

		changed = true;

	}
	if (open)
	{
		ImGuiUtils::SetAlignNamesToLeft(true);
		for (size_t i = 0; i < size; i++)
		{
			Firefly::Variable var;
			var.Pointer = vec.data() + i * GetByteSize(aListVariable.ListType);
			var.Type = aListVariable.ListType;
			var.Name = "[" + std::to_string(i) + "]";
			var.FileExtensions = aListVariable.FileExtensions;
			var.EnumCount = aListVariable.EnumCount;
			var.EnumToStringFunction = aListVariable.EnumToStringFunction;
			var.Increment = aListVariable.Increment;
			var.Min = aListVariable.Min;
			var.Max = aListVariable.Max;
			changed |= DrawVariable(var);
		}
		ImGuiUtils::SetAlignNamesToLeft(false);
	}
	return changed;
}

void InspectorWindow::DrawTransformFloatField(const char* aButtonLabel, const char* aFieldLabel, ImU32 aColor, float aWidth, float& aValue, bool& aStart, bool& aStop, float aPressedValue, bool aWonkyBool)
{
	bool zeroButtonPressed = false;

	ImGui::PushStyleColor(ImGuiCol_Button, aColor);
	if (ImGui::Button(aButtonLabel))
	{
		if (aWonkyBool)
		{
			ImGuiUtils::NotifyWarning("Resetting X rotation can be wonky when object is rotated around multiple axes /Benne");
		}

		zeroButtonPressed = true;
		aValue = aPressedValue;
		aStart = true;
		aStop = true;
	}
	ImGui::PopStyleColor();

	ImGui::SameLine();

	ImGui::PushItemWidth(aWidth);
	ImGui::DragFloat(aFieldLabel, &aValue);
	ImGui::PopItemWidth();

	aStart |= ImGui::IsItemActivated();
	aStop |= ImGui::IsItemDeactivatedAfterEdit();

	if (!myFloatFieldDragBegin)
	{
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && Utils::InputHandler::GetKeyHeld(VK_CONTROL) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (aWonkyBool)
				{
					ImGuiUtils::NotifyWarning("Manually inputting X rotation can be wonky when object is rotated around multiple axes /Benne");
				}

				myFloatFieldManualEditBegin = true;
			}
		}
	}

	if (!myFloatFieldManualEditBegin)
	{
		if (ImGui::IsItemActivated())
		{
			myFloatFieldDragBegin = true;
		}
	}

	if (ImGui::IsItemDeactivated())
	{
		myFloatFieldEditEnd = true;
	}

	if (zeroButtonPressed)
	{
		myZeroButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
	}
}

void InspectorWindow::DrawQuatCopyButton(const char* aButtonLabel, const Utils::Vector3f& aValueToCopy)
{
	ImGui::SameLine();

	if (ImGui::ImageButton(aButtonLabel, myCopyIcon->GetSRV().Get(), { ButtonWidth / 2, ButtonWidth / 2 + 4 }))
	{
		myCopiedRotation = Utils::Quaternion::CreateFromEulerAngles(aValueToCopy.x, 0, 0);
		myCopiedRotation = Utils::Quaternion::CreateFromEulerAngles(0, aValueToCopy.y, 0) * myCopiedRotation;
		myCopiedRotation = myCopiedRotation * Utils::Quaternion::CreateFromEulerAngles(0, 0, aValueToCopy.z);
	}
}

void InspectorWindow::DrawCopyButton(const char* aButtonLabel, TransformType aTransformType, const Utils::Vector3f& aValueToCopy)
{
	ImGui::SameLine();

	if (ImGui::ImageButton(aButtonLabel, myCopyIcon->GetSRV().Get(), { ButtonWidth / 2, ButtonWidth / 2 + 4 }))
	{
		switch (aTransformType)
		{
		case TransformType::Position: myCopiedPosition = aValueToCopy; break;
		case TransformType::Scale: myCopiedScale = aValueToCopy; break;
		}
	}
}

void InspectorWindow::DrawQuatPasteButton(const char* aButtonLabel, bool& aStart, bool& aStop)
{
	ImGui::SameLine();

	if (ImGui::ImageButton(aButtonLabel, myPasteIcon->GetSRV().Get(), { ButtonWidth / 2, ButtonWidth / 2 + 4 }))
	{
		myPasteButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
		aStart = true;
		aStop = true;
	}
}

void InspectorWindow::DrawPasteButton(const char* aButtonLabel, TransformType aTransformType, Utils::Vector3f& aValueToPasteTo, bool& aStart, bool& aStop)
{
	ImGui::SameLine();

	if (ImGui::ImageButton(aButtonLabel, myPasteIcon->GetSRV().Get(), { ButtonWidth / 2, ButtonWidth / 2 + 4 }))
	{
		switch (aTransformType)
		{
		case TransformType::Position: aValueToPasteTo = myCopiedPosition; break;
		case TransformType::Scale: aValueToPasteTo = myCopiedScale; break;
		}

		myZeroButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
		aStart = true;
		aStop = true;
	}
}

bool InspectorWindow::DrawResetButton(const char* aLabel, Utils::Vector3f& aValue, const Utils::Vector3f& aResetValue, bool& aStart, bool& aStop)
{
	if (!aValue.IsAlmostEqual(aResetValue))
	{
		ImGui::SameLine();

		if (ImGui::ImageButton(aLabel, myResetIcon->GetSRV().Get(), { ButtonWidth / 2, ButtonWidth / 2 + 4 }))
		{
			aValue = aResetValue;
			aStart = true;
			aStop = true;
			return true;
		}
	}

	return false;
}

void InspectorWindow::DrawPositionSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aPosition, bool& aStart, bool& aStop)
{
	bool prefabValueModified = false;
	if (aEntity->IsPrefab())
	{
		prefabValueModified = aEntity->IsParameterModified("Transform_Pos");
		if (prefabValueModified)
		{
			ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
		}
	}

	DrawTransformFloatField("X##Pos", ("##PositionX##" + std::to_string(aEntity->GetID())).c_str(), ColorRed, ImGui::GetContentRegionAvail().x / 5, aPosition.x, aStart, aStop);
	ImGui::SameLine();
	DrawTransformFloatField("Y##Pos", ("##PositionY##" + std::to_string(aEntity->GetID())).c_str(), ColorGreen, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth, aPosition.y, aStart, aStop);
	ImGui::SameLine();
	DrawTransformFloatField("Z##Pos", ("##PositionZ##" + std::to_string(aEntity->GetID())).c_str(), ColorBlue, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth * 2, aPosition.z, aStart, aStop);

	DrawCopyButton("##CopyPosition", TransformType::Position, aPosition);
	DrawPasteButton("##PastePosition", TransformType::Position, aPosition, aStart, aStop);

	if (DrawResetButton("##PosReset", aPosition, Utils::Vector3f::Zero(), aStart, aStop))
	{
		myZeroButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
	}

	if (prefabValueModified)
	{
		ImGuiUtils::PopFont();
	}
}

void InspectorWindow::DrawRotationSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aRotation, bool& aStart, bool& aStop)
{
	bool prefabValueModified = false;
	if (aEntity->IsPrefab())
	{
		prefabValueModified = aEntity->IsParameterModified("Transform_Rot");
		if (prefabValueModified)
		{
			ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
		}
	}

	bool wonky = !Utils::IsAlmostEqual(aRotation.z, 0.0f, 0.001f);

	DrawTransformFloatField("X##Rot", ("##RotationX##" + std::to_string(aEntity->GetID())).c_str(), ColorRed, ImGui::GetContentRegionAvail().x / 5, aRotation.x, aStart, aStop, 0, wonky);
	ImGui::SameLine();
	DrawTransformFloatField("Y##Rot", ("##RotationY##" + std::to_string(aEntity->GetID())).c_str(), ColorGreen, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth, aRotation.y, aStart, aStop);
	ImGui::SameLine();
	DrawTransformFloatField("Z##Rot", ("##RotationZ##" + std::to_string(aEntity->GetID())).c_str(), ColorBlue, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth * 2, aRotation.z, aStart, aStop);

	DrawQuatCopyButton("##CopyRotation", aRotation);
	DrawQuatPasteButton("##PasteRotation", aStart, aStop);

	if (DrawResetButton("##RotReset", aRotation, Utils::Vector3f::Zero(), aStart, aStop))
	{
		myZeroButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
	}

	if (prefabValueModified)
	{
		ImGuiUtils::PopFont();
	}
}

void InspectorWindow::DrawScaleSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aScale, bool& aStart, bool& aStop)
{
	bool prefabValueModified = false;
	if (aEntity->IsPrefab())
	{
		prefabValueModified = aEntity->IsParameterModified("Transform_Scale");
		if (prefabValueModified)
		{
			ImGuiUtils::PushFont(ImGuiUtilsFont_RobotoBold_16);
		}
	}

	DrawTransformFloatField("X##Scale", ("##ScaleX##" + std::to_string(aEntity->GetID())).c_str(), ColorRed, ImGui::GetContentRegionAvail().x / 5, aScale.x, aStart, aStop, 1.0f);
	ImGui::SameLine();
	DrawTransformFloatField("Y##Scale", ("##ScaleY##" + std::to_string(aEntity->GetID())).c_str(), ColorGreen, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth, aScale.y, aStart, aStop, 1.0f);
	ImGui::SameLine();
	DrawTransformFloatField("Z##Scale", ("##ScaleZ##" + std::to_string(aEntity->GetID())).c_str(), ColorBlue, ImGui::GetContentRegionAvail().x / 5 + ButtonWidth * 2, aScale.z, aStart, aStop, 1.0f);

	DrawCopyButton("##CopyScale", TransformType::Scale, aScale);
	DrawPasteButton("##PasteScale", TransformType::Scale, aScale, aStart, aStop);

	if (DrawResetButton("##ScaleReset", aScale, Utils::Vector3f::One(), aStart, aStop))
	{
		myZeroButtonPressed = true;
		myFloatFieldManualEditBegin = true;
		myFloatFieldEditEnd = true;
	}

	if (prefabValueModified)
	{
		ImGui::PopFont();
	}
}

void InspectorWindow::UpdateTransformComponent(Ptr<Firefly::Entity> aEntity)
{
	if (aEntity.expired())
	{
		LOGERROR("Entity is expired");
		return;
	}

	auto entity = aEntity.lock();

	ImGui::Text("Transform");

	static Utils::Vector3f initialPos;
	static Utils::Vector3f initialRot;
	static Utils::Vector3f initialScale;

	bool shouldStartTracking = false;
	bool shouldStopTracking = false;

	Utils::Vector3f position = entity->GetTransform().GetLocalPosition();
	DrawPositionSlider(entity, position, shouldStartTracking, shouldStopTracking);

	const Utils::Vector3f startOfFrameRot = { entity->GetTransform().GetLocalRotation().x, entity->GetTransform().GetRotation().y, entity->GetTransform().GetLocalRotation().z };
	Utils::Vector3f displayRot = startOfFrameRot;
	DrawRotationSlider(entity, displayRot, shouldStartTracking, shouldStopTracking);

	Utils::Vector3f scale = entity->GetTransform().GetLocalScale();
	DrawScaleSlider(entity, scale, shouldStartTracking, shouldStopTracking);

	if (shouldStartTracking)
	{
		initialPos = entity->GetTransform().GetLocalPosition();
		myInitialQuat = entity->GetTransform().GetLocalQuaternion();
		initialRot = startOfFrameRot;
		initialScale = entity->GetTransform().GetLocalScale();
	}

	if (myFloatFieldDragBegin && !myZeroButtonPressed)
	{
		entity->GetTransform().SetLocalPosition(position);
		entity->GetTransform().AddLocalRotation({ displayRot.x - startOfFrameRot.x, 0, displayRot.z - startOfFrameRot.z });
		entity->GetTransform().AddRotation({ 0, displayRot.y - startOfFrameRot.y, 0 });
		entity->GetTransform().SetLocalScale(scale);
	}

	if (myFloatFieldEditEnd)
	{
		if (shouldStopTracking)
		{
			if (myFloatFieldDragBegin)
			{
				Utils::Vector3f posDiff = position - initialPos;
				Utils::Quaternion rotDiff = myInitialQuat.GetInverse() * entity->GetTransform().GetLocalQuaternion();
				Utils::Vector3f scaleDiff = scale / initialScale;

				Ref<TransformCommand> transformCommand = std::make_shared<TransformCommand>(aEntity, posDiff, rotDiff, scaleDiff);
				transformCommand->UpdateModification();
				EditorLayer::AddEntityUndo(transformCommand);
			}
			else if (myFloatFieldManualEditBegin)
			{
				if (myZeroButtonPressed)
				{
					Utils::Vector3f newPos = position;
					Utils::Quaternion newRot = myInitialQuat;
					newRot = Utils::Quaternion::CreateFromEulerAngles(0, displayRot.y - startOfFrameRot.y, 0) * newRot;
					newRot = newRot * Utils::Quaternion::CreateFromEulerAngles(0, 0, displayRot.z - startOfFrameRot.z);
					newRot = newRot * Utils::Quaternion::CreateFromEulerAngles(displayRot.x - startOfFrameRot.x, 0, 0);
					Utils::Vector3f newScale = scale;

					Ref<AbsoluteTransformCommand> transformCommand = std::make_shared<AbsoluteTransformCommand>(aEntity, newPos, newRot, newScale);
					EditorLayer::ExecuteAndAddEntityUndo(transformCommand);
				}
				else if (myPasteButtonPressed)
				{
					Utils::Vector3f newPos = position;
					Utils::Quaternion newRot = myCopiedRotation;
					Utils::Vector3f newScale = scale;

					Ref<AbsoluteTransformCommand> transformCommand = std::make_shared<AbsoluteTransformCommand>(aEntity, newPos, newRot, newScale);
					EditorLayer::ExecuteAndAddEntityUndo(transformCommand);
				}
				else
				{
					Utils::Vector3f posDiff = position - initialPos;
					Utils::Quaternion rot = entity->GetTransform().GetLocalQuaternion();
					rot = rot * Utils::Quaternion::CreateFromEulerAngles(displayRot.x - initialRot.x, 0, displayRot.z - initialRot.z);
					rot = Utils::Quaternion::CreateFromEulerAngles(0, displayRot.y - initialRot.y, 0) * rot;
					Utils::Quaternion rotDiff = entity->GetTransform().GetLocalQuaternion().GetInverse() * rot;
					Utils::Vector3f scaleDiff = scale / initialScale;

					Ref<TransformCommand> transformCommand = std::make_shared<TransformCommand>(aEntity, posDiff, rotDiff, scaleDiff);
					EditorLayer::ExecuteAndAddEntityUndo(transformCommand);
				}
			}
		}

		myPasteButtonPressed = false;
		myFloatFieldManualEditBegin = false;
		myFloatFieldDragBegin = false;
		myFloatFieldEditEnd = false;
		myZeroButtonPressed = false;
	}
}