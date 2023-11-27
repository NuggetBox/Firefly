#pragma once
#include <memory>
#include <vector>

#include "imgui/imgui.h"

#include "MuninScriptGraph.h"

namespace Firefly
{
	class Texture2D;
}

class ScriptGraphEditor
{
	struct SearchMenuItem
	{
		std::string Title;
		std::string Value;
		size_t Rank = SIZE_MAX;
		std::string Tag;
	};

	struct ContextMenuItem
	{
		std::string Title;
		std::string Value;
		std::string Tag;
	};

	struct ContextMenuCategory
	{
		std::string Title;
		std::vector<ContextMenuItem> Items;
	};

	struct EditorInterfaceState
	{
		// The current value of the node search list.
		std::string bgCtxtSearchField;
		bool bgCtxtSearchFieldChanged = false;
		bool bgCtxtSearchFocus = false;
		std::vector<SearchMenuItem> bgCtxtSearchFieldResults;

		std::string varNewVarNameField;
		int varNewVarTypeIdx = 0;
		ScriptGraphDataObject varNewVarValue;
		int varInlineEditingIdx = -1;

		ContextMenuCategory bgCtxtVariablesCategory;
		std::string varToDelete;

		bool flowShowFlow = false;
		bool isTicking = false;

		bool errorIsErrorState = false;
		std::string errorMessage;
		size_t errorNodeId = 0;

		bool initNavToContent = false;

		//Benne variables
		ImVec2 rightClickPos = { 0, 0 };

		bool contextSensitive = false;
		bool isOutPin = true;
		bool backgroundWasClosed = false;
		size_t sourcePin = 0;
		std::string targetLabel;
		std::type_index nodeCreationPinType = typeid(float);

		ImVec2 copyMiddlePos;
		std::vector<size_t> copiedNodes;
		std::vector<ImVec2> copiedPositions;
		std::vector<std::pair<size_t, size_t>> copiedLinks;
		std::vector<ScriptGraphPin> copiedPinData;
	} myState;

	// Actual storage of looking up categories when generating.
	static inline std::unordered_map<std::string, ContextMenuCategory> myBgContextCategories;
	// Sorted list of context category names.
	static inline std::vector<std::string> myBgContextCategoryNamesList;

	std::unique_ptr<ScriptGraphSchema> mySchema;

	std::shared_ptr<Firefly::Texture2D> myNodeHeaderTexture;
	std::shared_ptr<Firefly::Texture2D> myGetterGradient;
	std::unordered_map<ScriptGraphNodeType, std::shared_ptr<Firefly::Texture2D>> myNodeTypeIcons;

	// TEMP
	std::shared_ptr<ScriptGraph> myGraph;

	void UpdateVariableContextMenu();

	// Context Menues
	void BackgroundContextMenu();
	void NodeContextMenu(size_t aNodeUID);
	void LinkContextMenu(size_t aLinkUID);

	// Modals
	void TriggerEntryPoint();
	void EditVariables();

	// Event Stuff
	void HandleEditorCreate();
	void HandleEditorDelete();

	// Graph Rendering stuff
	void RenderNode(const ScriptGraphNode* aNode);
	void RenderPin(const ScriptGraphPin* aPin);
	void DrawPinIcon(const ScriptGraphPin* aPin, ImVec4 aPinColor, bool isConnected) const;

	void CreateEmptyGraph();

	void HandleScriptGraphError(const ScriptGraph& aGraph, size_t aNodeUID, const std::string& anErrorMessage);

	void ClearErrorState();

	void Save();
	void Load();
	void Copy();
	void Paste();
	void HandleCopyPaste();

	bool CanConnectPinInCategory(const std::vector<ContextMenuItem>& aCategory, std::type_index aPinType, bool aIsOutPin);
	bool CanConnectPinType(const ContextMenuItem& aMenuNode, std::type_index aPinType, bool aIsOutPin, std::string& aOutPinLabel);

public:
	void OpenScriptGraph(const std::filesystem::path& aVisualScriptFile);

	void Init();
	void Update(float aDeltaTime);

	void Render();
};
