#pragma once
#include <future>

#include "Firefly/Core/Core.h"
#include "Firefly/Core/Layer/Layer.h"
#include "Editor/Windows/EditorWindow.h"

#include "UndoSystem/UndoHandler.h"

namespace Firefly
{
	class AppUpdateEvent;
	class EditorAppUpdateEvent;
	class Entity;
	class Scene;
	class Prefab;
}

class EditorLayer : public Firefly::Layer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Firefly::Event& aEvent) override;

	void WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam) override;

	static const std::vector<Ptr<Firefly::Entity>>& GetSelectedEntities();
	static const Ptr<Firefly::Entity>& GetFirstSelectedEntity();
	static const Ptr<Firefly::Entity>& GetLastSelectedEntity();

	//Accelerated
	static bool IsEntitySelected(const Ptr<Firefly::Entity>& anEntity);
	static bool IsEntitySelected(uint64_t anID);
	static const Ptr<Firefly::Entity>& GetSelectedEntityById(uint64_t anID);

	template<class T>
	static std::shared_ptr<T> GetWindow();
	
	template<class T> 
	static std::shared_ptr<T> GetOrCreateWindow();

	template<class T> 
	static std::vector<Ref<T>> GetWindows();


	static std::vector<Ptr<Firefly::Scene>>& GetEditingScenes();
	static std::shared_ptr<Firefly::Prefab>& GetEditingPrefab();

	//TODO: Benne, overlook bool argument, CTRL+Z per window stuff
	static void ExecuteAndAddEntityUndo(const Ref<UndoCommand>& aCommand, bool aOverrideFocusRequirement = false);
	static void AddEntityUndo(const Ref<UndoCommand>& aCommand, bool aOverrideFocusRequirement = false);
	static void ClearAllEntityUndos(bool aOverrideFocusRequirement = false);
	static void BeginEntityUndoSeries(bool aOverrideFocusRequirement = false);
	static void EndEntityUndoSeries(bool aOverrideFocusRequirement = false);

	static bool IsHierarchyFocused();
	static bool IsCoreWindowsFocused();

	static void TogglePlayMode(bool aPlayFromHere = false);
	static void OnEnterPlay(bool aPlayFromHere = false);
	static void OnExitPlay();

	static void SelectEntity(Ptr<Firefly::Entity> aEntity, bool aAddSelected = false);
	/// <summary>
	/// Adds the entity to the selected entities list if it is not already in the list.
	/// </summary>
	/// <param name="aEntity"></param>
	static void AddSelectedEntity(Ptr<Firefly::Entity> aEntity);
	//Returns true if an entity was deselected, false if it couldn't find it among selected
	static bool DeselectEntity(Ptr<Firefly::Entity> aEntity);
	static void DeselectAllEntities();
	static void DeselectAllEntitiesCommand();

	static void DeselectEntitiesWithSelectedParents();

	static void RemoveOutlineFromSelectedChildren();

	static void DeleteSelectedEntities();

	static void DuplicateEntities(const std::vector<Ptr<Firefly::Entity>>& someEntities, UndoHandler& aUndoHandler = ourEntityUndoHandler);
	static void DuplicateSelectedEntities();

	static void CreateEntityFolderForSelectedEntities();

	static void ToggleHideEntityCommand(const Ptr<Firefly::Entity>& aEntity);
	static void HideEntityCommand(const Ptr<Firefly::Entity>& aEntity, bool aHide = true);

	static void RenameEntity(const std::vector<Ptr<Firefly::Entity>>& someEntities );
	static void RenameSelectedEntity();

	static bool IsAnyEditSceneModified();

	static void SaveScene(Ptr<Firefly::Scene> aScene);
	static void SaveSceneAs(Ptr<Firefly::Scene> aScene);
	static void UnloadScene(Ptr<Firefly::Scene> aScene);

	static void SetShouldOutline(Ptr<Firefly::Entity> aEntity, bool aShouldOutline = true);

	static bool HasSelectedParentRecursive(Ptr<Firefly::Entity> aEntity);
	static bool HasSelectedChildren(Ptr<Firefly::Entity> aEntity);

	static void SetImGuiInputEnabled(bool aEnable = true);


private:
	static void SaveCrashedScene();
	static void SetEditingScenes(std::vector<Ptr<Firefly::Scene>> aScene);

	bool OnEditorUpdate(Firefly::EditorAppUpdateEvent& aEvent);

	void CreateWindowsImGuiMenu();
	std::shared_ptr<EditorWindow> GetWindow(std::string aName);
	std::vector<Ref<EditorWindow>> GetWindows(std::string aName);

	void DoSaveEditorSettings();

	void SaveCurrentScenes();

	void OnClickFabianColliderButton();

	void DoSave();
	void DoSaveAs();
	void DoLoad();
	void DoNew();

	void NewScene();
	void SaveScenes();
	void SaveCurrentSceneAs();
	void LoadScene();

	void BuildProject();
	void StartBuild();
	void UpdateBuildProjectImGui();

	void ChangeSceneModal(std::function<void()> anAction);
	void UnsavedChangesInPrefabModal(std::function<void()> aYesFunc, std::function<void()> aNoFunc);
	void UnsavedChangesInSceneModal(std::function<void()> aYesFunc, std::function<void()> aNoFunc);

	void InputShortcuts();

	void SavePrefab();

	static void CreateEditorWindow(const std::string& aWindowFactoryName);

	static inline std::vector<std::shared_ptr<EditorWindow>> myEditorWindows;
	static inline std::unordered_map<std::string, std::string > myEditorWindowsDisplayNames;

	static inline std::vector<Ptr<Firefly::Entity>> ourSelectedEntities;
	static inline std::unordered_map<uint64_t, Ptr<Firefly::Entity>> ourSelectedEntitiesMap;

	static inline Ref<Firefly::Scene> ourPrefabScene;
	// this needs to be shared ptr because we need to keep the scenes alive even if they are not in the scene manager
	static inline std::vector<Ref<Firefly::Scene>> ourCachedEditingScenes;
	static inline std::vector<Ptr<Firefly::Scene>> ourEditingScenes;

	static inline std::shared_ptr<Firefly::Prefab> ourEditingPrefab;

	static inline UndoHandler ourEntityUndoHandler;

	bool myBuildingProjectSetting = false;
	bool myIsBuildingProject = false;
	
	bool myBuildIsCompilingShaderKeys = false;
	bool myBuildIsCopyingFiles = false;
	bool myBuildIsCheckingForGarbage = false;
	bool myBuildIsDone = false;

	uint32_t myNumberOfCompiledShaders = 0;
	uint32_t myTotalNumberToCompiledShaders = 0;
	std::string myCurrentCompilingShader = "";
	bool myIsCompilingShaders = false;

	std::vector<std::string> myBuildWarningMessages;

	std::string myBuildFolder;
	std::string myBuildFolderName = "Fisk Build";
	std::string myBuildExeName = "Fisk";
	std::string myBuildPath;
	std::vector<std::string> myCurrentBuildOperationsText;

	std::vector<std::future<void>> myBuildOperationsFutures;

	std::vector<std::filesystem::path> myScenesToIncludeInBuild;
	std::vector<std::filesystem::path> myBuildFiles;
	std::vector<std::string> myShaderKeysToCompileInBuild;

	uint32_t myBuildOperationsTotalCount = 0;
	uint32_t myBuildOperationsCompletedCount = 0;

	std::future<void> myBuildFuture;

	static inline std::string myRenameBuffer;
	bool myRenamingFlag;

	private:
		//Ljud Bool
		bool isON = false;
	//DefaultEditorScene
		std::string myDefaultEditorScenePath = "NULL";

};

template<class T>
inline std::shared_ptr<T> EditorLayer::GetWindow()
{
	for (auto& window : myEditorWindows)
	{
		if (window->GetName() == T::GetFactoryName())
		{
			return std::reinterpret_pointer_cast<T>(window);
		}
	}
	return std::shared_ptr<T>();
}

template<class T>
inline std::shared_ptr<T> EditorLayer::GetOrCreateWindow()
{
	auto window = GetWindow<T>();

	if (!window)
	{
		CreateEditorWindow(T::GetFactoryName());
		window = GetWindow<T>();
	}
	return window;
}

template<class T>
inline std::vector<Ref<T>> EditorLayer::GetWindows()
{
	std::vector<Ref<T>> windows;
	for (auto& window : myEditorWindows)
	{
		if (window->GetName() == T::GetFactoryName())
		{
			windows.push_back(std::reinterpret_pointer_cast<T>(window));
		}
	}
	return windows;
}
