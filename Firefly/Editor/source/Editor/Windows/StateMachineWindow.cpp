#include "EditorPch.h"
#include "StateMachineWindow.h"

#include "WindowRegistry.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Application/Application.h"
#include "Editor/Utilities/EditorUtils.h"

REGISTER_WINDOW(StateMachineWindow);

namespace ed = ax::NodeEditor;

StateMachineWindow::StateMachineWindow() : EditorWindow("StateMachine")
{
	ed::Config config;
	config.SettingsFile = "Editor/Config/Simple.json";
	context = ed::CreateEditor(&config);
	myWindowFlags = ImGuiWindowFlags_MenuBar;
	myCurrentPath = "";
}

void StateMachineWindow::OnImGui()
{
	if (Firefly::Application::Get().GetWindow()->IsMinimized())
		return;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				Clear();
			}
			if (ImGui::MenuItem("Save"))
			{
				if (myCurrentPath == "")
				{
					SaveAs();
				}
				else
				{
					Save(myCurrentPath);
				}
			}
			if (ImGui::MenuItem("Save As"))
			{
				SaveAs();
			}
			if (ImGui::MenuItem("Load"))
			{
				Load();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings"))
		{
			ImGui::DragFloat("Width", &myNodeSize);

			ImGui::EndMenu();
		}
		if (ImGui::Button("Add Node"))
		{
			AddNode();
		}
		ImGui::EndMenuBar();
	}
	RenderWindow();
}

void StateMachineWindow::AddNode()
{
	Nodes node;
	ed::NodeId nodeID = GetNextID();
	node.nodeId = nodeID;
	node.nodeName = "Test Node";
	if (myNodes.size() == 1)
		node.myIsStart = true;

	PinInfo pinStuff;
	pinStuff.Id = GetNextID();
	pinStuff.PinName = "In";
	pinStuff.PinType = ed::PinKind::Input;
	pinStuff.ParentID = nodeID.Get();
	node.pinInfo.push_back(pinStuff);
	pinStuff.Id = GetNextID();
	pinStuff.PinName = "Out";
	pinStuff.PinType = ed::PinKind::Output;
	node.pinInfo.push_back(pinStuff);

	myNodes.push_back(node);
}

void StateMachineWindow::AddAnyState()
{
	Nodes node;
	ed::NodeId nodeID = GetNextID();
	node.nodeId = nodeID;
	node.nodeName = "Any State";
	node.IsAnyState = true;

	PinInfo pinStuff;
	pinStuff.Id = GetNextID();
	pinStuff.PinName = "Out";
	pinStuff.PinType = ed::PinKind::Output;
	node.pinInfo.push_back(pinStuff);

	myNodes.push_back(node);
}

void StateMachineWindow::RenderNodes()
{
	for (size_t i = 0; i < myNodes.size(); i++)
	{
		if (myNodes[i].myIsStart)
			ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBorder, { 1,0,0,1 });

		if (myNodes[i].IsAnyState)
			ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, { 0,0,0.5f,1 });

		ed::BeginNode(myNodes[i].nodeId);

		if (myNodes[i].myIsStart || myNodes[i].IsAnyState)
			ed::PopStyleColor();

		ImGui::Text(myNodes[i].nodeName.c_str());
		for (auto pin : myNodes[i].pinInfo)
		{
			ed::BeginPin(pin.Id, pin.PinType);
			ImGui::Text(pin.PinName.c_str());
			ed::EndPin();
			ImGui::SameLine(myNodeSize);
		}
		ed::SetGroupSize(myNodes[i].nodeId, { 1000,1000 });
		ed::EndNode();
		if (myFirstFrame)
			ed::SetNodePosition(myNodes[i].nodeId, myNodes[i].Position);
		else
			myNodes[i].Position = GetNodePosition(myNodes[i].nodeId);
	}
	myFirstFrame = false;
}

bool StateMachineWindow::LinkIsValid(const ax::NodeEditor::PinId& aIn, const ax::NodeEditor::PinId& aOut, Link& aLink)
{
	for (size_t i = 0; i < myLinks.size(); i++)
	{
		const auto& link = myLinks[i];
		if (link.InputId == aIn || link.OutputId == aIn)
		{
			if (link.InputId == aOut || link.OutputId == aOut)
			{
				return false;
			}
		}
	}
	PinInfo* in = nullptr;
	PinInfo* out = nullptr;
	for (size_t i = 0; i < myNodes.size(); i++)
	{
		for (size_t j = 0; j < myNodes[i].pinInfo.size(); j++)
		{
			if (myNodes[i].pinInfo[j].Id == aIn)
			{
				in = &myNodes[i].pinInfo[j];
				aLink.InputId = in->Id;
				aLink.InputNode = myNodes[i].nodeId;
			}
			if (myNodes[i].pinInfo[j].Id == aOut)
			{
				out = &myNodes[i].pinInfo[j];
				aLink.OutputId = out->Id;
				aLink.OutputNode = myNodes[i].nodeId;
			}
		}
		if (in != nullptr && out != nullptr)
		{
			break;
		}
	}
	if (in->PinType == out->PinType)
		return false;
	if (in->ParentID == out->ParentID)
		return false;

	return true;
}

void StateMachineWindow::RenderEditor()
{
	ed::Begin("Editor");

	if (myNodes.size() == 0)
		AddAnyState();

	if (ed::BeginCreate())
	{
		ed::PinId inputPinId, outputPinId;
		if (ed::QueryNewLink(&inputPinId, &outputPinId))
		{
			if (inputPinId && outputPinId)
			{
				Link link;
				if (ed::AcceptNewItem() && LinkIsValid(inputPinId, outputPinId, link))
				{
					link.Id = GetNextID();
					link.Layers.push_back(Layer());
					myLinks.push_back(link);
				}
			}
		}
	}
	ed::EndCreate();

	if (ed::BeginDelete())
	{
		ed::NodeId deletedNodeId;
		while (ed::QueryDeletedNode(&deletedNodeId))
		{
			if (ed::AcceptDeletedItem())
			{
				for (size_t i = 0; i < myNodes.size(); i++)
				{
					if (myNodes[i].nodeId == deletedNodeId && !myNodes[i].IsAnyState)
					{
						bool findNewStart = false;
						if (myNodes[i].myIsStart)
							findNewStart = true;

						myNodes.erase(myNodes.begin() + i);
						i--;

						if (findNewStart)
						{
							for (size_t j = 0; j < myNodes.size(); j++)
							{
								if (!myNodes[j].IsAnyState)
								{
									myNodes[j].myIsStart = true;
									break;
								}
							}
						}

						break;
					}
				}
			}
		}
		ed::LinkId deletedLinkId;
		while (ed::QueryDeletedLink(&deletedLinkId))
		{
			if (ed::AcceptDeletedItem())
			{
				for (size_t i = 0; i < myLinks.size(); i++)
				{
					if (myLinks[i].Id == deletedLinkId)
					{
						myLinks.erase(myLinks.begin() + i);
						i--;
						break;
					}
				}
			}
		}
	}
	ed::EndDelete();

	RenderNodes();

	ed::PushStyleVar(ed::StyleVar_LinkStrength, 0);
	ed::PushStyleVar(ed::StyleVar_FlowSpeed, 5);
	for (size_t i = 0; i < myLinks.size(); i++)
	{
		ed::Link(myLinks[i].Id, myLinks[i].InputId, myLinks[i].OutputId);
		ed::Flow(myLinks[i].Id);
	}

	int size = ed::GetSelectedObjectCount();
	if (size > 1)
		size = 1;
	ed::NodeId selectedNode;
	ed::GetSelectedNodes(&selectedNode, size);
	mySelectedNode = selectedNode;

	ed::LinkId selectedLink;
	ed::GetSelectedLinks(&selectedLink, size);
	mySelectedLink = selectedLink;

	ed::End();
}

void StateMachineWindow::RenderWindow()
{
	ed::SetCurrentEditor(context);

	if (ImGui::BeginTable("Editor", 3, ImGuiTableFlags_Resizable)) {

		ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch, 100.0F);
		ImGui::TableSetupColumn("Editor", ImGuiTableColumnFlags_WidthStretch, 300.0F);
		ImGui::TableSetupColumn("Node Info", ImGuiTableColumnFlags_WidthStretch, 500.0F);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::BeginChild("Properties", ImVec2(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::Button("Add Parameter");
			if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
			{
				for (int i = 0; i < static_cast<int>(ParameterTypes::Count); i++)
				{
					auto type = static_cast<ParameterTypes>(i);
					PushColorByType(type);
					if (ImGui::MenuItem(ParameterToString(type).c_str()))
					{
						AddParameter(type);
					}
					ImGui::PopStyleColor(1);
				}
				ImGui::EndPopup();
			}

			for (size_t i = 0; i < myParams.size(); i++)
			{
				auto param = &myParams[i];
				PushColorByType(param->Type);
				ImGui::Text(ParameterToString(param->Type).c_str());
				ImGui::PopStyleColor(1);
				ImGui::SameLine();
				ImGui::InputText(("##ParamName" + std::to_string(param->Id)).c_str(), &param->Name);

				if (ImGui::BeginPopupContextItem(("##ParamDelete" + std::to_string(myParams[i].Id)).c_str()))
				{
					myParams.erase(myParams.begin() + i);
					i--;
					ImGui::EndPopup();
				}
			}

			ImGui::EndChild();
		}
		ImGui::TableNextColumn();

		RenderEditor();

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("Node Info", ImVec2(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (mySelectedLink)
			{
				Link* selLink = nullptr;
				for (size_t i = 0; i < myLinks.size(); i++)
				{
					if (myLinks[i].Id == mySelectedLink)
					{
						selLink = &myLinks[i];
						break;
					}
				}
				if (selLink == nullptr)
				{
					ImGui::EndChild();
					ImGui::EndTable();
					ed::SetCurrentEditor(nullptr);
					return;
				}
				if (ImGui::Button("Add Layer"))
				{
					selLink->Layers.push_back(Layer());
				}
				for (size_t i = 0; i < selLink->Layers.size(); i++)
				{
					std::string name;

					if (i == selLink->SelectedLayer)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
						name = "Selected Layer";
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
						name = "Layer: " + std::to_string(i + 1);
					}

					ImGui::Text(name.c_str());
					ImGui::PopStyleColor(1);
					if (ImGui::BeginPopupContextItem(("##SwapLayer" + std::to_string(i)).c_str(), ImGuiPopupFlags_MouseButtonLeft))
					{
						selLink->SelectedLayer = i;
						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}
					if (ImGui::IsMouseReleased(1) && ImGui::BeginPopupContextItem(("##LayerDelete" + std::to_string(i)).c_str()))
					{
						selLink->Layers.erase(selLink->Layers.begin() + i);
						ImGui::EndPopup();
						break;
					}
				}
				ImGui::Button("Add Transition");
				if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
				{
					for (int i = 0; i < myParams.size(); i++)
					{
						if (ImGui::MenuItem(myParams[i].Name.c_str()))
						{
							bool found = false;
							for (auto param : selLink->Layers[selLink->SelectedLayer].Params)
							{
								if (param.Id == myParams[i].Id)
								{
									found = true;
									break;
								}
							}
							if (!found)
								selLink->Layers[selLink->SelectedLayer].Params.push_back(myParams[i]);
						}
					}
					ImGui::EndPopup();
				}

				for (size_t i = 0; i < selLink->Layers[selLink->SelectedLayer].Params.size(); i++)
				{
					ParamTypeImGui(selLink->Layers[selLink->SelectedLayer].Params[i]);

					if (ImGui::BeginPopupContextItem(("##ParamDelete" + std::to_string(selLink->Layers[selLink->SelectedLayer].Params[i].Id)).c_str()))
					{
						selLink->Layers[selLink->SelectedLayer].Params.erase(selLink->Layers[selLink->SelectedLayer].Params.begin() + i);
						i--;
						ImGui::EndPopup();
					}
				}
			}

			if (mySelectedNode)
			{
				auto node = GetNodeById(mySelectedNode);
				if (!node->IsAnyState)
				{
					ImGui::InputText("Name", &node->nodeName);
					if (ImGui::Checkbox("Start Node", &node->myIsStart) && ImGui::IsItemEdited())
					{
						for (size_t i = 0; i < myNodes.size(); i++)
						{
							myNodes[i].myIsStart = false;
						}
						node->myIsStart = true;
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndTable();
	}
	ed::SetCurrentEditor(nullptr);
}

Nodes* StateMachineWindow::GetNodeById(const ax::NodeEditor::NodeId& aId)
{
	for (size_t i = 0; i < myNodes.size(); i++)
	{
		if (myNodes[i].nodeId == aId)
			return &myNodes[i];
	}
	return nullptr;
}

std::string StateMachineWindow::ParameterToString(ParameterTypes aType)
{
	switch (aType)
	{
	case Bool:
		return "Bool";
	case Float:
		return "Float";
	case Int:
		return "Int";
	default:
		return "Nothing";
	}
}

std::string StateMachineWindow::GetParamName(const uint64_t& aID)
{
	for (size_t i = 0; i < myParams.size(); i++)
	{
		if (myParams[i].Id == aID)
		{
			return myParams[i].Name;
		}
	}
	return "Can't Find Name";
}

void StateMachineWindow::PushColorByType(const ParameterTypes& aType)
{
	switch (aType)
	{
	case Bool:
		ImGui::PushStyleColor(ImGuiCol_Text, { 1,0.15f,0.105f,1.f });
		return;
	case Float:
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.564f, 0.93f, 0.564f, 1.f });
		return;
	case Int:
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.53f, 0.6f, 1.f });
		return;
	}
}

void StateMachineWindow::ParamTypeImGui(Parameter& aParam)
{
	ImGui::BeginTable("##Win", 3, ImGuiTableFlags_Resizable);
	ImGui::TableNextColumn();
	std::string paramName = GetParamName(aParam.Id);
	PushColorByType(aParam.Type);
	ImGui::Text(paramName.c_str());
	aParam.Name = paramName;
	ImGui::PopStyleColor(1);
	ImGui::TableNextColumn();
	std::vector<std::string> options;

	if (aParam.Type != ParameterTypes::Bool)
	{
		options.push_back("Less");
		options.push_back("Equal");
		options.push_back("Greater");
		ImGuiUtils::Combo(("##" + aParam.Name + std::to_string(aParam.Id)), aParam.Condition, options, ImGuiComboFlags_NoArrowButton);
	}
	ImGui::TableNextColumn();
	switch (aParam.Type)
	{
	case Bool:
		options.push_back("False");
		options.push_back("True");
		ImGuiUtils::Combo(("##" + aParam.Name + std::to_string(aParam.Id)), aParam.Condition, options, ImGuiComboFlags_NoArrowButton);
		break;
	case Float:
		ImGuiUtils::DragFloat(("##" + std::to_string(aParam.Id)), std::get<float>(aParam.Value));
		break;
	case Int:
		ImGuiUtils::DragInt(("##" + std::to_string(aParam.Id)), std::get<int>(aParam.Value));
		break;
	default:
		break;
	}
	ImGui::EndTable();
}

void StateMachineWindow::AddParameter(ParameterTypes aType)
{
	Parameter param;
	param.Type = aType;
	param.Name = "New Param";
	param.Id = GetNextID();
	myParams.push_back(param);
}

uint64_t StateMachineWindow::GetNextID()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis;
	return dis(gen);
}

void StateMachineWindow::Save(const std::filesystem::path& aPath)
{
	std::ofstream file(aPath);
	if (!file.is_open())
	{
		ImGuiUtils::NotifyError("Could not save state to path \"{}\"\n File could not be opened for write, make sure it is checked out in perforce!", myCurrentPath.string());
		return;
	}
	nlohmann::json json;

	json["Params"];
	int index = 0;
	for (auto param : myParams)
	{
		json["Params"][index]["Name"] = param.Name;
		json["Params"][index]["ID"] = param.Id;
		json["Params"][index]["Type"] = param.Type;
		index++;
	}

	index = 0;
	json["Nodes"];
	for (auto node : myNodes)
	{
		json["Nodes"][index]["Name"] = node.nodeName;
		json["Nodes"][index]["ID"] = node.nodeId.Get();
		json["Nodes"][index]["Pos"] = { node.Position.x, node.Position.y };
		json["Nodes"][index]["IsStart"] = node.myIsStart;
		json["Nodes"][index]["IsAnyState"] = node.IsAnyState;
		json["Nodes"][index]["Pins"];
		int tempIndex = 0;
		for (auto pin : node.pinInfo)
		{
			json["Nodes"][index]["Pins"][tempIndex]["Name"] = pin.PinName;
			json["Nodes"][index]["Pins"][tempIndex]["ID"] = pin.Id.Get();
			json["Nodes"][index]["Pins"][tempIndex]["ParentID"] = pin.ParentID.Get();
			json["Nodes"][index]["Pins"][tempIndex]["Type"] = pin.PinType;
			tempIndex++;
		}
		index++;
	}
	index = 0;
	json["Links"];
	for (auto link : myLinks)
	{
		json["Links"][index]["ID"] = link.Id.Get();
		json["Links"][index]["InID"] = link.InputId.Get();
		json["Links"][index]["InNode"] = link.InputNode.Get();
		json["Links"][index]["OutID"] = link.OutputId.Get();
		json["Links"][index]["OutNode"] = link.OutputNode.Get();
		json["Links"][index]["Layers"];
		for (size_t i = 0; i < link.Layers.size(); i++)
		{
			int tempIndex = 0;
			json["Links"][index]["Layers"][i]["Params"];
			for (auto param : link.Layers[i].Params)
			{
				json["Links"][index]["Layers"][i]["Params"][tempIndex]["Name"] = param.Name;
				json["Links"][index]["Layers"][i]["Params"][tempIndex]["ID"] = param.Id;
				json["Links"][index]["Layers"][i]["Params"][tempIndex]["Type"] = param.Type;
				json["Links"][index]["Layers"][i]["Params"][tempIndex]["Condition"] = param.Condition;
				switch (param.Type)
				{
				case ParameterTypes::Bool:
				{
					json["Links"][index]["Layers"][i]["Params"][tempIndex]["Value"] = std::get<bool>(param.Value);
					break;
				}
				case ParameterTypes::Float:
					json["Links"][index]["Layers"][i]["Params"][tempIndex]["Value"] = std::get<float>(param.Value);
					break;
				case ParameterTypes::Int:
					json["Links"][index]["Layers"][i]["Params"][tempIndex]["Value"] = std::get<int>(param.Value);
					break;
				}
				tempIndex++;
			}
		}
		index++;
	}

	file << std::setw(4) << json;
	file.close();
	ImGuiUtils::NotifySuccess("Saved :)");
}

void StateMachineWindow::SaveAs()
{
	std::filesystem::path path = EditorUtils::GetSaveFilePath("StateMachine (*.state)\0*.state\0", "state");
	if (path == "")
		return;
	Save(path);
	myCurrentPath = path;
}

void StateMachineWindow::Load()
{
	std::filesystem::path path = std::filesystem::relative(EditorUtils::GetOpenFilePath("StateMachine (*.state)\0*.state\0"));
	if (path == "")
		return;
	myCurrentPath = path;
	Clear();
	nlohmann::json json;
	std::ifstream ifStream(path);
	if (ifStream.fail())
		return;
	ifStream >> json;

	for (size_t i = 0; i < json["Params"].size(); i++)
	{
		Parameter param;
		param.Id = json["Params"][i]["ID"];
		param.Name = json["Params"][i]["Name"];
		param.Type = json["Params"][i]["Type"];
		myParams.push_back(param);
	}

	for (size_t i = 0; i < json["Nodes"].size(); i++)
	{
		Nodes node;
		int id = json["Nodes"][i]["ID"];
		node.nodeId = id;
		node.nodeName = json["Nodes"][i]["Name"];
		node.Position = { json["Nodes"][i]["Pos"][0], json["Nodes"][i]["Pos"][1] };
		node.myIsStart = json["Nodes"][i]["IsStart"];
		node.IsAnyState = json["Nodes"][i]["IsAnyState"];
		for (size_t j = 0; j < json["Nodes"][i]["Pins"].size(); j++)
		{
			PinInfo pin;
			pin.Id = (int)json["Nodes"][i]["Pins"][j]["ID"];
			pin.ParentID = id;
			pin.PinName = json["Nodes"][i]["Pins"][j]["Name"];
			pin.PinType = json["Nodes"][i]["Pins"][j]["Type"];

			node.pinInfo.push_back(pin);
		}
		myNodes.push_back(node);
	}

	for (size_t i = 0; i < json["Links"].size(); i++)
	{
		Link link;
		link.Id = (int)json["Links"][i]["ID"];
		link.InputId = (int)json["Links"][i]["InID"];
		link.InputNode = (int)json["Links"][i]["InNode"];
		link.OutputId = (int)json["Links"][i]["OutID"];
		link.OutputNode = (int)json["Links"][i]["OutNode"];

		for (size_t j = 0; j < json["Links"][i]["Layers"].size(); j++)
		{
			Layer layer;
			for (size_t k = 0; k < json["Links"][i]["Layers"][j]["Params"].size(); k++)
			{
				Parameter param;
				param.Id = json["Links"][i]["Layers"][j]["Params"][k]["ID"];
				param.Name = json["Links"][i]["Layers"][j]["Params"][k]["Name"];
				param.Type = json["Links"][i]["Layers"][j]["Params"][k]["Type"];
				param.Condition = json["Links"][i]["Layers"][j]["Params"][k]["Condition"];

				switch (param.Type)
				{
				case ParameterTypes::Bool:
					std::get<bool>(param.Value) = json["Links"][i]["Layers"][j]["Params"][k]["Value"];
					break;
				case ParameterTypes::Float:
					std::get<float>(param.Value) = json["Links"][i]["Layers"][j]["Params"][k]["Value"];
					break;
				case ParameterTypes::Int:
					std::get<int>(param.Value) = json["Links"][i]["Layers"][j]["Params"][k]["Value"];
					break;
				}
				layer.Params.push_back(param);
			}
			link.Layers.push_back(layer);
		}
		myLinks.push_back(link);
	}
	ifStream.close();
	myFirstFrame = true;
}

void StateMachineWindow::Clear()
{
	myNodes.clear();
	myLinks.clear();
	myParams.clear();
	mySelectedNode = 0;
	mySelectedLink = 0;
	myFirstFrame = true;
}