#include "EditorPch.h"
#include "PhysicsWindow.h"
#include "WindowRegistry.h"
#include "Firefly/Physics/PhysicsLayerHandler.h"
#include <Firefly/Core/Log/DebugLogger.h>

REGISTER_WINDOW(PhysicsWindow);
PhysicsWindow::PhysicsWindow() : EditorWindow("Physics Window")
{
	myInt = 1;
}

void PhysicsWindow::OnImGui()
{
	auto layers = Firefly::PhysicsLayerHandler::GetAllLayers();
	auto namesOfLayers = Firefly::PhysicsLayerHandler::GetNamesOflayers();
	myInt = layers.size();
	if (ImGui::Button("Save"))
	{
		Firefly::PhysicsLayerHandler::Save("FireflyEngine/Physics/PhysicsLayers.json");
	}
	bool openRename = false;
	for (auto& layer : layers)
	{
		if (layer <= 1)
		{
			continue;
		}

		auto name = Firefly::PhysicsLayerHandler::GetNameOfLayer(layer) + "	ID: " + std::to_string(layer);
		ImGui::PushID(name.c_str());
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImGui::Indent();
			for (auto& layerToCheck : layers)
			{
				auto layerToCheckName = Firefly::PhysicsLayerHandler::GetNameOfLayer(layerToCheck);
				static bool checkBoxFlag = true;
				checkBoxFlag = Firefly::PhysicsLayerHandler::GetLayerActive(layer, layerToCheck);
				std::string n = layerToCheckName + "##" + std::to_string(layerToCheck);
				if (ImGui::Checkbox(n.c_str(), &checkBoxFlag))
				{
					Firefly::PhysicsLayerHandler::SetLayerActive(layer, layerToCheck, checkBoxFlag);
				}
			}


			ImGui::PushStyleColor(ImGuiCol_Button, { 0.4,0,0,1 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2,0,0,1 });
			if (ImGui::Button("Remove"))
			{
				Firefly::PhysicsLayerHandler::Remove(layer);
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0.4,0,1 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0.2,0,1 });
			if (ImGui::Button("Rename"))
			{
				
				openRename = true;
				myRenameID = layer;
				myRenameBuffer = Firefly::PhysicsLayerHandler::GetNameOfLayer(layer);
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::Unindent();
		}
		ImGui::PopID();
	}
	if (ImGui::Button("+##physicsWindowAdd", { 21, 0 }))
	{
		ImGui::OpenPopup("##physicsWindowAddPopUp");
	}
	if (openRename)
	{
		ImGui::OpenPopup("##physicsWindowRename");
	}
	if (ImGui::BeginPopupModal("##physicsWindowRename"))
	{
		ImGui::InputText("new name", &myRenameBuffer);
		bool canReanme = true;
		if (std::find(namesOfLayers.begin(), namesOfLayers.end(), myRenameBuffer) != namesOfLayers.end())
		{
			canReanme = false;
			ImGui::TextColored({ 1, 0,0,1 }, "Layer allready exists!");
		}
		if (ImGui::Button("Rename"))
		{
			if (canReanme)
			{
				Firefly::PhysicsLayerHandler::Add(myRenameID, myRenameBuffer, Firefly::PhysicsLayerHandler::GetLayersToCollideWith(myRenameID));
				ImGui::CloseCurrentPopup();
				myRenameBuffer = "";
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
			myRenameBuffer = "";
			myRenameID = -1;
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("##physicsWindowAddPopUp"))
	{
		ImGui::InputText("name", &myInputName);
		bool canCreate = true;
		if (std::find(namesOfLayers.begin(), namesOfLayers.end(), myInputName) != namesOfLayers.end())
		{
			canCreate = false;
			ImGui::TextColored({ 1, 0,0,1 }, "Layer allready exists!");
		}
		if (ImGui::Button("Create"))
		{
			if (canCreate)
			{
				myInt = 0;
				while (Firefly::PhysicsLayerHandler::LayerExist((1 << myInt)))
				{
					myInt++;
				}
				Firefly::PhysicsLayerHandler::Add((1 << myInt), myInputName, ~0u);
				ImGui::CloseCurrentPopup();
				myInputName = "";
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
			myInputName = "";
		}
		ImGui::EndPopup();
	}
}
