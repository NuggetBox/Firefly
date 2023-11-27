#pragma once
#include "Firefly/Asset/ResourceCache.h"

#include "Editor/Windows/EditorWindow.h"
#include <future>

namespace Firefly
{
	class BlendSpace;
	class AvatarMask;
	class Camera;
	class SubMesh;
}

class Entry
{
public:
	Entry* Parent;
	std::string Name;
	std::string Extension;
	bool IsDirectory;
	bool IsFavorite = false;
	std::vector<std::shared_ptr<Entry>> Children;
	std::filesystem::path Path;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	Ref<Firefly::Asset> Asset;
	Utils::Vector2f LastRenderSize;
};

//when adding here, also add to GetEntryTypeFromExtension, GetExtensionFromEntryType and GetIconFromEntryType
enum class EntryType
{
	Unknown,
	Directory,
	Animator,
	Animation,
	Mesh,
	Skeleton,
	Material,
	Texture,
	Prefab,
	Font,
	Emitter,
	Scene,
	Audio,
	FBX,
	SoundBank,
	BlendSpace,
	Pipeline,
	AvatarMask,
	VisualScript,
	GameplayStateMachine,
	VoiceLineData
};

class ContentBrowser : public EditorWindow
{
public:
	ContentBrowser();
	virtual ~ContentBrowser() = default;

	void RegenerateEntries();

	void OpenEntryInBrowser(const std::filesystem::path& aPath);
	void OpenEntryInBrowser(Entry* aEntry);

	void SelectEntry(const std::filesystem::path& aPath);
	void SetSearchString(std::string aString);

	void OnImGui() override;
	void OnEvent(Firefly::Event& aEvent) override;
	void OnOpenEntry(Entry& aEntry);

	static Entry* GetSelectedEntry();
	Entry* GetEntryFromPath(const std::filesystem::path& aPath);

	void WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam) override;

	std::vector<Entry*> GetAllEntriesOfTypes(std::vector<EntryType> someTypes);
	static EntryType GetEntryTypeFromExtension(std::string aExtension);
	static std::string GetExtensionFromEntryType(EntryType aType);

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<ContentBrowser>(); }
	static std::string GetFactoryName() { return "ContentBrowser"; }
	std::string GetName() const override { return GetFactoryName(); }
private:

	Ref<Firefly::Texture2D> GetIconFromExtension(std::string aExtension);
	Ref<Firefly::Texture2D> GetIconFromEntryType(EntryType aType);
	std::string GetDisplayNameFromEntryType(EntryType aType);
	std::string GetDisplayNameFromExtension(const std::string& aExtension);


	float myCameraDistMul = 0.8f;
	float myCameraFov = 90;


	void DrawFbxImportWindow();
	void DrawTextureImportWindow();
	std::vector<std::string> GetEntryNamesFromPath(const std::filesystem::path& aPath);

	void AddPrimitiveEntries(Ref<Entry> aPrimitiveFolder);
	void GenerateEntriesRecursive(Entry& aEntry);
	void SetCurrentEntry(Entry& aEntry, bool aPushEntryToBackStack = true);

	void DrawTopBar();
	void DrawSearchBar();
	void ClearSearchText();
	void CollectSearchEntries();

	void OnPressBackButton();
	void OnPressForwardButton();


	void DrawMiddleBar();
	void ResizeMiddleBar();
	void DrawDirectoryHierarchy();
	void DrawFavoritesRecursive(Entry& aEntry);
	void DrawDirectoryHierarchyRecursive(Entry& aEntry);
	void DrawEntryTile(Entry& aEntry);
	void DrawEntryList(Entry& aEntry);

	void CheckEntryInteractionItem(Entry& aEntry, bool aCanDragFlag);

	void RenameInputText(Entry& aEntry);
	void DrawEntries(std::vector<std::shared_ptr<Entry>> someEntries);
	void SetEntryAsPayload(Entry& aEntry);


	void DrawBottomBar();

	void BeginRename(Entry& aEntry);
	void EndRename(const std::filesystem::path& aNewPath);
	void AcceptEntityToCreatePrefab();

	void OnRightClickEmptySpace();
	void OnRightClickEntry(Entry& aEntry);

	void SetAsFavorite(Entry& aEntry);
	void RemoveFromFavorites(Entry& aEntry);

	void DuplicateEntry(Entry& aEntry);

	void MoveEntry(std::filesystem::path aPath, Entry& aToEntry);

	void CalcCameraPositionData(std::vector< std::vector<Firefly::SubMesh>*>& someSubMeshes, Utils::Vector3f& aCenter, float& aRadius);

	std::vector<std::shared_ptr<Entry>> GetChildrenRecursive(Entry& aEntry);




	void OpenCreateWindow(const std::string& aName, EntryType aType, const std::filesystem::path& aPath);
	void UpdateCreateWindow();

	std::string mySearchString;

	Utils::Vector2f myContentWindowPos;
	Utils::Vector2f myContentWindowSize;

	std::string myRenameBuffer;
	std::filesystem::path myRenamePath;
	bool myRenamingFlag;

	bool myShowHidden = false;
	bool myResizingMiddleBar;
	bool myLockedFlag;
	bool myShowMeshPreviewOnRight = true;
	float myMiddleBarResizeValue;
	float myDragStartValue;
	float myMiddleBarRightChildBeginX;
	float myFurthestToTheRightListEntryX = 0.f;
	int myTileSize = 100;
	int myTopBarIconSize = 25;
	Utils::Vector2f myWindowMin;
	Utils::Vector2f myWindowMax;

	Ref<Firefly::Texture2D>	myDirectoryIcon;
	Ref<Firefly::Texture2D>	myFileIcon;
	Ref<Firefly::Texture2D>	myMeshIcon;
	Ref<Firefly::Texture2D>	myTextureIcon;
	Ref<Firefly::Texture2D>	myMaterialIcon;
	Ref<Firefly::Texture2D>	myPipelineIcon;
	Ref<Firefly::Texture2D>	myPrefabIcon;
	Ref<Firefly::Texture2D>	myAnimatorIcon;
	Ref<Firefly::Texture2D>	mySceneIcon;
	Ref<Firefly::Texture2D>	myAudioIcon;
	Ref<Firefly::Texture2D> myEmitterIcon;
	Ref<Firefly::Texture2D> myAnimationIcon;
	Ref<Firefly::Texture2D> mySkeletonIcon;
	Ref<Firefly::Texture2D> myFBXIcon;
	Ref<Firefly::Texture2D> mySoundBankIcon;
	Ref<Firefly::Texture2D> myBlendspaceIcon;
	Ref<Firefly::Texture2D> myAvatarMaskIcon;
	Ref<Firefly::Texture2D> myVisualScriptIcon;
	Ref<Firefly::Texture2D> myGameplayStateMachineIcon;
	Ref<Firefly::Texture2D> myVoiceLineDataIcon;


	Ref<Firefly::Texture2D>	myStarIcon;
	Ref<Firefly::Texture2D>	myBackIcon;
	Ref<Firefly::Texture2D>	mySearchIcon;
	Ref<Firefly::Texture2D>	myRefreshIcon;
	Ref<Firefly::Texture2D>	myFontIcon;
	Ref<Firefly::Texture2D>	mySettingsIcon;
	Ref<Firefly::Texture2D> myLockUnlockedIcon;
	Ref<Firefly::Texture2D> myLockLockedIcon;
	Ref<Firefly::Texture2D> myReimportAllIcon;
	Ref<Firefly::Texture2D> myEyeOpenIcon;
	Ref<Firefly::Texture2D> myEyeClosedIcon;
	Ref<Firefly::Texture2D> myErrorIcon;


	Entry* myCurrentRenderingEntry;
	uint32_t myRenderID;
	Ref<Firefly::Camera> myCamera;
	float myAnimationTime = 0;


	enum class RenderMode : uint32_t
	{
		List,
		Tiles
	}myCurrentRenderMode;

	std::vector<std::shared_ptr<Entry>> mySearchEntries;

	std::shared_ptr<Entry> myRootEntry;
	Entry* myCurrentEntry;
	Entry* myRenamingEntry;

	std::vector<Entry*> myForwardEntryStack;
	std::vector<Entry*> myPrevOpenEntriesStack;
	const int  myMaxOpenEntryStackCount = 20;

	std::vector<std::filesystem::path> myFavorites;

	inline static Entry* mySelectedEntry = nullptr;


	uint32_t myCurrentCompileCount;
	uint32_t myTotalCompileCount;
	bool myIsCompiling;
	bool myCompilingDone;
	bool myCompilingDonePopupFlag;
	std::future<void> myCompileFuture;
	std::filesystem::path myCompilingPath;

	//Create window data
	std::string myCreateWindowName;
	std::string myCreateWindowAssetName;
	std::filesystem::path myCreateWindowDirectoryPath; // with trailing slash
	EntryType myCreateWindowAssetType;
	bool myCreateWindowFlag;

	Ref<Firefly::BlendSpace> myCreateWindowBlendSpaceToCreate;
	Ref<Firefly::AvatarMask> myCreateWindowAvatarMaskToCreate;
	std::string myCreateWindowSkeletonPath;
	// For Graphics pipeline creation
	Firefly::GraphicsPipeline myCreateWindowPipelineToCreate;
	bool myWantGeometryStage = true;
	bool myWantPixelStage = true;
	uint32_t myCurrentPipelineChoice = 0;

	//

	struct FbxImportData
	{

		bool IsImporting = false;
		std::vector<std::filesystem::path> FromPathAlreadyExisting;
		std::vector<std::filesystem::path> FromPaths;
		std::filesystem::path ToDirectoryPath = "";
		std::string SkeletonPath = "";
		enum class FbxImportType : int
		{
			Animation,
			Skeleton,
			StaticMesh
		} FbxTypeToImport = FbxImportType::StaticMesh;

	}myFbxImportData;

	//texture import
	struct TextureImportData
	{
		bool IsImporting = false;
		std::vector<std::filesystem::path> PathAlreadyExisting;
		std::vector<std::filesystem::path> FromPaths;
		std::filesystem::path ToDirectoryPath = "";

		bool IsSrgb = false;
	}myTextureImportData;
};