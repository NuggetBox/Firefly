#pragma once
#include "EditorWindow.h"
#include "imgui_node_editor.h"
#include <tuple>

enum ParameterTypes
{
	Bool,
	Float,
	Int,
	Count
};

struct Parameter
{
	ParameterTypes Type = ParameterTypes::Count;
	std::string Name;
	uint64_t Id = -1;
	std::tuple<float, int, bool> Value = { 0,0,0 };
	uint32_t Condition = 0;
};

struct Layer
{
	std::vector<Parameter> Params;
};

struct PinInfo
{
	ax::NodeEditor::PinId Id = -1;
	ax::NodeEditor::PinKind PinType = ax::NodeEditor::PinKind::Input;
	ax::NodeEditor::NodeId ParentID = -1;
	std::string PinName = "";
};

struct Nodes
{
	ax::NodeEditor::NodeId nodeId = -1;
	std::string nodeName = "";
	bool myIsStart = false;
	bool IsAnyState = false;
	ImVec2 Position = { 0,0 };
	std::vector<PinInfo> pinInfo;
};

struct Link
{
	ax::NodeEditor::LinkId Id = -1;
	ax::NodeEditor::PinId InputId = -1;
	ax::NodeEditor::PinId OutputId = -1;
	ax::NodeEditor::NodeId InputNode = -1;
	ax::NodeEditor::NodeId OutputNode = -1;
	int SelectedLayer = 0;
	std::vector<Layer> Layers;
};

class StateMachineWindow : public EditorWindow
{
public:
	StateMachineWindow();

	static std::string GetFactoryName() { return "StateMachine"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<StateMachineWindow>(); }

protected:
	void OnImGui() override;
private:
	ax::NodeEditor::EditorContext* context = nullptr;

	std::vector<Link> myLinks;
	std::vector<Nodes> myNodes;

	ax::NodeEditor::LinkId mySelectedLink = -1;
	ax::NodeEditor::NodeId mySelectedNode = -1;

	std::vector<Parameter> myParams;

	std::filesystem::path myCurrentPath = "";
	bool myFirstFrame = false;
	float myNodeSize = 20;

	void AddNode();
	void AddAnyState();
	void RenderNodes();
	bool LinkIsValid(const ax::NodeEditor::PinId& aIn, const ax::NodeEditor::PinId& aOut, Link& aLink);
	void RenderEditor();
	void RenderWindow();

	Nodes* GetNodeById(const ax::NodeEditor::NodeId& aId);

	std::string ParameterToString(ParameterTypes aType);

	std::string GetParamName(const uint64_t& aID);
	void PushColorByType(const ParameterTypes& aType);

	void ParamTypeImGui(Parameter& aParam);

	void AddParameter(ParameterTypes aType);

	uint64_t GetNextID();

	void Save(const std::filesystem::path& aPath);
	void SaveAs();
	void Load();
	void Clear();
};