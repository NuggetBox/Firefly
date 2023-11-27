#include "EditorPch.h"
#include "ContentBrowser.h"

#include "Firefly/Asset/Importers/ShaderImporter.h"
#include "Firefly/Asset/Animations/BlendSpace.h"
#include "Firefly/Asset/Animations/AvatarMask.h"

#include <Firefly/Rendering/Pipeline/PipelineLibrary.h>

#include "imgui/imgui_internal.h"

#include "Editor/Utilities/ImGuiUtils.h"
#include "SerializationUtils.h"

#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Application/Application.h"

#include "Editor/EditorLayer.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Editor/Event/EditorOnlyEvents.h"
#include <Editor/Windows/MaterialEditorWindow.h>

#include "Firefly/Asset/Prefab.h"
#include "Utils/Timer.h"
#include "Utils/Math/Transform.h"

#include <Editor/Windows/AnimatorWindow.h>

#include "VisualScriptingWindow.h"

#include "Editor/Windows/PipelineWindow.h"
#include "Editor/Windows/MaterialEditorWindow.h"
#include "Editor/Windows/ParticleEditorWindow.h"
#include "Editor/Windows/BlendSpaceEditorWindow.h"
#include "Editor/Windows/AvatarMaskEditorWindow.h"
#include "Editor/Windows/AnimationEditor.h"
#include "Editor/Windows/VoiceLineDataEditor.h"

#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/Postprocess/PostProcessUtils.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "Firefly/Event/ApplicationEvents.h"

#include "Firefly/Components/Mesh/AnimatedMeshComponent.h"
#include "Firefly/Components/Mesh/MeshComponent.h"


#include "Firefly/Asset/Animation.h"

#include "ImageConverter.h"
#include <Firefly/Rendering/Shader/ShaderLibrary.h>

REGISTER_WINDOW(ContentBrowser);

ContentBrowser::ContentBrowser()
	:EditorWindow("Content Browser")
{
	myWindowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

	myRenameBuffer = "";
	myRenamingFlag = false;
	myRenamingEntry = nullptr;
	mySelectedEntry = nullptr;
	myResizingMiddleBar = false;
	myMiddleBarResizeValue = 200;
	myDragStartValue = 200;
	myMiddleBarRightChildBeginX = 203;
	myCurrentRenderMode = RenderMode::Tiles;
	myLockedFlag = false;
	myCompilingDone = false;

	myIsCompiling = false;
	myCompilingDonePopupFlag = false;
	myTotalCompileCount = 0;
	myCurrentCompileCount = 0;

	myRenderID = Firefly::Renderer::InitializeScene(true);
	//Firefly::Renderer::GetSceneFrameBuffer(myRenderID)->Resize({ static_cast<uint32_t>(95), static_cast<uint32_t>(95) }, true);


	myCamera = Firefly::Camera::Create(Firefly::CameraInfo());
	myCamera->SetNearPlane(10);
	myDirectoryIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_directory.dds", true);
	myFileIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_file.dds", true);
	myMeshIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_mesh.dds", true);
	myTextureIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_texture.dds", true);
	myBackIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_back.dds", true);
	myMaterialIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_material.dds", true);
	myPipelineIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_pipeline.dds", true);
	myPrefabIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_prefab.dds", true);
	mySearchIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_search.dds", true);
	myRefreshIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_refresh.dds", true);
	myFontIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_font.dds", true);
	mySettingsIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_settings.dds", true);
	myEmitterIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_emitter.dds", true);
	myAnimatorIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_animator.dds", true);
	myStarIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_star.dds", true);
	mySceneIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_scene.dds", true);
	myAudioIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_audio.dds", true);
	mySkeletonIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_skeleton.dds", true);
	myAnimationIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_animation.dds", true);
	myFBXIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_fbx.dds", true);
	mySoundBankIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_sound_bank.dds", true);
	myBlendspaceIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_blendspace.dds", true);
	myAvatarMaskIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_avatar_mask.dds", true);
	myGameplayStateMachineIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_gameplay_state_machine.dds", true);
	myVoiceLineDataIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_voice_line_data.dds", true);

	myLockLockedIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_lock_locked.dds", true);
	myLockUnlockedIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_lock_unlocked.dds", true);
	myReimportAllIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_reimport_all.dds", true);
	myEyeOpenIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_eye_open.dds", true);
	myEyeClosedIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_eye_closed.dds", true);
	myErrorIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_error.dds", true);



	myVisualScriptIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor\\Icons\\icon_flow.dds");



	myRootEntry = std::make_shared<Entry>();
	myRootEntry->IsDirectory = true;
	myRootEntry->Name = "Assets";
	myRootEntry->Path = "Assets";
	myRootEntry->Parent = nullptr;
	myRootEntry->Extension = "";
	SetCurrentEntry(*myRootEntry);
	RegenerateEntries();

	myCamera->GetTransform().SetRotation(35.f, 157.5f, 0);
	myCamera->SetFov(150);

	myCameraFov = 17.5f;
	myCameraDistMul = 7.f;

	myCreateWindowFlag = false;
}

void ContentBrowser::RegenerateEntries()
{
	myCurrentRenderingEntry = nullptr;
	myForwardEntryStack.clear();
	myPrevOpenEntriesStack.clear();
	auto currentPath = myCurrentEntry->Path;

	myRootEntry->Children.clear();

	//Create the meshes folder because the primitive folder depends on it
	const char* meshesPath = "Assets\\Graphical Assets";
	if (!std::filesystem::exists(meshesPath))
	{
		std::filesystem::create_directory(meshesPath);
	}

	GenerateEntriesRecursive(*myRootEntry);
	mySelectedEntry = nullptr;
	myCurrentEntry = myRootEntry.get();

	auto primitivesFolder = std::make_shared<Entry>();
	primitivesFolder->Name = "Primitives";
	primitivesFolder->Parent = myRootEntry.get();
	primitivesFolder->Path = "Assets\\Graphical Assets\\Primitives";
	primitivesFolder->IsDirectory = true;
	primitivesFolder->Extension = "";

	auto meshesIt = std::find_if(myRootEntry->Children.begin(), myRootEntry->Children.end(), [&](Ref<Entry> ent) {return ent->Name == "Graphical Assets"; });
	(*meshesIt)->Children.insert((*meshesIt)->Children.begin(), primitivesFolder);

	AddPrimitiveEntries(primitivesFolder);

	OpenEntryInBrowser(currentPath);

	for (int i = 0; i < myFavorites.size(); i++)
	{
		auto& fav = myFavorites[i];
		auto favEntry = GetEntryFromPath(fav);
		if (favEntry == nullptr)
		{
			myFavorites.erase(myFavorites.begin() + i);
			i--;
		}
		else
		{
			favEntry->IsFavorite = true;
		}
	}

	Firefly::ResourceCache::IndexAllAssets(); // TODO: Do this in a better place
}
std::vector<std::string> ContentBrowser::GetEntryNamesFromPath(const std::filesystem::path& aPath)
{
	auto path = "\\" + aPath.string();
	//split path into names
	std::vector<std::string> names;
	size_t offset = 0;
	auto lastOffset = offset;
	while (offset != path.npos)
	{
		lastOffset = offset;
		offset = path.find_first_of('\\', offset + 1);
		if (offset != path.npos)
		{
			names.push_back(path.substr(lastOffset + 1, offset - (lastOffset)-1));
		}
	}
	//get entry without trailing slash
	names.push_back(path.substr(lastOffset + 1, path.size() - lastOffset - 1));
	return names;
}
Entry* ContentBrowser::GetEntryFromPath(const std::filesystem::path& aPath)
{
	if (aPath == myRootEntry->Path)
	{
		return myRootEntry.get();
	}

	auto names = GetEntryNamesFromPath(aPath);


	//find entry
	Entry* currentEntry = myRootEntry.get();
	names.erase(names.begin());
	for (auto& name : names)
	{
		bool found = false;
		for (auto& child : currentEntry->Children)
		{
			if ((child->Name + child->Extension) == name)
			{
				currentEntry = child.get();
				found = true;
				break;
			}
		}
		if (!found)
		{
			//LOGERROR("Tried to get entry with path that does not seem to exist, try reloading the content browser, Path: \"{}\", Name of file not found: {}", aPath.string(), name);
			return nullptr;
		}
	}
	return currentEntry;
}

//TODO: Add primitives here
void ContentBrowser::AddPrimitiveEntries(Ref<Entry> aPrimitiveFolder)
{
	//cube
	auto cubeEntry = std::make_shared<Entry>();
	cubeEntry->Name = "Cube";
	cubeEntry->Parent = aPrimitiveFolder.get();
	cubeEntry->Path = "Cube";
	cubeEntry->IsDirectory = false;
	cubeEntry->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(cubeEntry);

	//pyramid
	auto pyramidEntry = std::make_shared<Entry>();
	pyramidEntry->Name = "Pyramid";
	pyramidEntry->Parent = aPrimitiveFolder.get();
	pyramidEntry->Path = "Pyramid";
	pyramidEntry->IsDirectory = false;
	pyramidEntry->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(pyramidEntry);

	//Plane
	auto planeEntry = std::make_shared<Entry>();
	planeEntry->Name = "Plane";
	planeEntry->Parent = aPrimitiveFolder.get();
	planeEntry->Path = "Plane";
	planeEntry->IsDirectory = false;
	planeEntry->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(planeEntry);

	//LDCube
	auto ldCube = std::make_shared<Entry>();
	ldCube->Name = "LDCube";
	ldCube->Parent = aPrimitiveFolder.get();
	ldCube->Path = "LDCube";
	ldCube->IsDirectory = false;
	ldCube->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(ldCube);

	//Triangle
	auto triangle = std::make_shared<Entry>();
	triangle->Name = "Triangle";
	triangle->Parent = aPrimitiveFolder.get();
	triangle->Path = "Triangle";
	triangle->IsDirectory = false;
	triangle->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(triangle);

	//Cylinder
	auto cylinder = std::make_shared<Entry>();
	cylinder->Name = "Cylinder";
	cylinder->Parent = aPrimitiveFolder.get();
	cylinder->Path = "Cylinder";
	cylinder->IsDirectory = false;
	cylinder->Extension = ".mesh";
	aPrimitiveFolder->Children.push_back(cylinder);
}

void ContentBrowser::GenerateEntriesRecursive(Entry& aEntry)
{
	for (const auto& entry : std::filesystem::directory_iterator(aEntry.Path))
	{
		auto newEntry = std::make_shared<Entry>();
		newEntry->Name = entry.path().stem().string();
		newEntry->Parent = &aEntry;
		newEntry->Path = aEntry.Path.string() + "\\" + entry.path().filename().string();
		newEntry->IsDirectory = entry.is_directory();
		newEntry->Extension = entry.path().extension().string();

		aEntry.Children.push_back(newEntry); // push back the new entry into parent



		if (newEntry->IsDirectory)
		{
			GenerateEntriesRecursive(*newEntry);
		}
		//sort so that entries are sorted alphabetically
		std::sort(newEntry->Children.begin(), newEntry->Children.end(),
			[](const std::shared_ptr<Entry>& a, const std::shared_ptr<Entry>& b) { return a->Name < b->Name; });
		//sort so that directories are first
		std::sort(newEntry->Children.begin(), newEntry->Children.end(),
			[](const std::shared_ptr<Entry>& a, const std::shared_ptr<Entry>& b) {return a->IsDirectory > b->IsDirectory; });
	}
}

void ContentBrowser::SetCurrentEntry(Entry& aEntry, bool aPushEntryToBackStack)
{
	if (mySearchString != "")
	{
		ClearSearchText();
	}
	if (myCurrentEntry != &aEntry)
	{

		if (aPushEntryToBackStack && &aEntry != myRootEntry.get() && &aEntry != myCurrentEntry)
		{
			if (myPrevOpenEntriesStack.size() >= myMaxOpenEntryStackCount)
			{
				myPrevOpenEntriesStack.erase(myPrevOpenEntriesStack.begin());
			}
			myPrevOpenEntriesStack.push_back(myCurrentEntry);
			myForwardEntryStack.clear();
		}
		myCurrentEntry = &aEntry;
		mySelectedEntry = nullptr;
	}
}




void ContentBrowser::DrawTopBar()
{
	//ImGui::BeginChild("##ConentBrowserTopBar", ImVec2(0, 0), false,
	//	ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushID(ImGui::GetID("##ContentBrowserBackButton"));
	bool backDisabled = myPrevOpenEntriesStack.size() == 0;
	if (ImGui::ImageButton(myBackIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), backDisabled ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(1, 1, 1, 1)))
	{
		if (!backDisabled)
		{
			OnPressBackButton();
		}
	}
	if (myPrevOpenEntriesStack.size() > 0)
	{
		ImGuiUtils::ToolTip(("Back to " + myPrevOpenEntriesStack.back()->Path.string()).c_str());
	}
	ImGui::PopID();
	ImGui::SameLine();
	ImGui::PushID(ImGui::GetID("##ContentBrowserForwardButton"));
	bool forwardDisabled = myForwardEntryStack.size() == 0;
	if (ImGui::ImageButton(myBackIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize),
		ImVec2(1, 0), ImVec2(0, 1), 0, ImVec4(0, 0, 0, 0), forwardDisabled ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(1, 1, 1, 1)))
	{
		if (!forwardDisabled)
		{
			OnPressForwardButton();
		}
	}
	if (myForwardEntryStack.size() > 0)
	{
		ImGuiUtils::ToolTip(("Forward to " + myForwardEntryStack.back()->Path.string()).c_str());
	}
	ImGui::PopID();

	//Mouse 4 and 5 back and forward
	if (ImGui::IsKeyPressed(ImGuiKey_MouseX1, false))
	{
		if (!backDisabled)
		{
			OnPressBackButton();
		}
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_MouseX2, false))
	{
		if (!forwardDisabled)
		{
			OnPressForwardButton();
		}
	}


	ImGui::SameLine();
	DrawSearchBar();

	ImGui::SameLine();
	auto cursorPos = ImGui::GetCursorScreenPos();
	ImGui::Text("?");
	ImGui::SameLine();
	ImGui::ItemAdd(ImRect(cursorPos, ImVec2(cursorPos.x + ImGui::CalcTextSize("").y, cursorPos.y + ImGui::CalcTextSize("").y)),
		ImGui::GetID("##ContentBrowserTopBarSearchTip"));
	std::string tooltip = "Search for files and folders.\n";
	tooltip += "Example: \"torii\" will search for all files and folders with \"torii\" in their name.\n \n";
	tooltip += "To Search for a specific file type, type \"t:\" followed by the desired file extension.\n";
	tooltip += "Example: \"t:fbx\" will search for all fbx files.\n";
	tooltip += "Example: \"torii t:fbx\" will search for all fbx files with \"torii\" in their name.\n";
	ImGuiUtils::ToolTip(tooltip.c_str());

	ImGui::SameLine();
	if (ImGui::ImageButton(myRefreshIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize)))
	{
		RegenerateEntries();
	}
	ImGui::SameLine();
	ImGui::ImageButton(mySettingsIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize));
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
	{
		ImGui::Text("Settings");
		ImGui::Separator();
		ImGui::Text("RenderMode");
		//item mode 
		if (ImGui::RadioButton("Tiles", myCurrentRenderMode == RenderMode::Tiles))
		{
			myCurrentRenderMode = RenderMode::Tiles;
		}
		if (myCurrentRenderMode == RenderMode::Tiles)
		{
			ImGui::SliderInt("##ContentBrowserSettingsTileSizeSlider", &myTileSize, 20, 300);
		}
		//list mode
		if (ImGui::RadioButton("List", myCurrentRenderMode == RenderMode::List))
		{
			myCurrentRenderMode = RenderMode::List;
		}
		ImGui::Separator();
		ImGui::Text("Show Hidden");
		ImGui::SameLine();
		ImGui::Checkbox("##ContentBrowserSettingsShowHiddenCheckbox", &myShowHidden);

		/*ImGui::Separator();
		if (ImGui::SliderFloat("CameraDistMul##ContentBrowserSettings", &myCameraDistMul, 0.01f, 10.f))
		{
			RegenerateEntries();
		}
		if (ImGui::SliderFloat("Fov##ContentBrowserSettings", &myCameraFov, 5, 180))
		{
			RegenerateEntries();
		}*/

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	Ref<Firefly::Texture2D> lockIcon = myLockedFlag ? myLockLockedIcon : myLockUnlockedIcon;
	if (ImGui::ImageButton(lockIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize)))
	{
		myLockedFlag = !myLockedFlag;
	}


	//Next line
	if (mySearchString == "")
	{
		ImGui::SameLine();
		Entry* parent = myCurrentEntry;
		std::vector<Entry*> parentsRecursive;
		while (parent != nullptr)
		{
			parentsRecursive.push_back(parent);
			parent = parent->Parent;
		}
		std::reverse(parentsRecursive.begin(), parentsRecursive.end());

		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
		bool first = true;
		for (auto& parentEntry : parentsRecursive)
		{
			if (first)
			{
				first = false;
			}
			else
			{
				ImGui::SameLine();
			}
			if (ImGui::Button(parentEntry->Name.c_str(), { 0,20 }))
			{
				SetCurrentEntry(*parentEntry);
			}

			ImGui::SameLine();
			ImGui::Text("/");
		}
		ImGui::PopStyleColor();
	}

	if (myCurrentRenderMode == RenderMode::List)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - myTopBarIconSize);
		if (ImGui::ImageButton((myShowMeshPreviewOnRight ? myEyeOpenIcon : myEyeClosedIcon)->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize)))
		{
			myShowMeshPreviewOnRight = !myShowMeshPreviewOnRight;
		}
	}
}

void ContentBrowser::DrawSearchBar()
{
	ImGui::Image(mySearchIcon->GetSRV().Get(), ImVec2(myTopBarIconSize, myTopBarIconSize));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
	if (ImGui::InputTextWithHint("##ContentBrowerSearchInput", "Search Content", &mySearchString))
	{
		if (mySearchString != "")
		{
			CollectSearchEntries();
		}
		else
		{
			mySearchEntries.clear();
		}
	}
	ImGui::SameLine();
	ImGui::CalcItemWidth();
	if (ImGui::Button("x"))
	{
		ClearSearchText();
	}

}
void ContentBrowser::ClearSearchText()
{
	mySearchString = "";
	mySearchEntries.clear();
	if (mySelectedEntry)
	{
		if (mySelectedEntry->Parent)
		{
			SetCurrentEntry(*mySelectedEntry->Parent);
		}
	}
}
void ContentBrowser::OpenEntryInBrowser(Entry* aEntry)
{
	if (aEntry->IsDirectory)
	{
		SetCurrentEntry(*aEntry);
	}
	else
	{
		SetCurrentEntry(*aEntry->Parent);
	}
}

void ContentBrowser::OpenEntryInBrowser(const std::filesystem::path& aPath)
{
	auto entry = GetEntryFromPath(aPath);
	OpenEntryInBrowser(entry);
}

void ContentBrowser::DrawFbxImportWindow()
{
	if (myFbxImportData.IsImporting)
	{

		//positiin the window in the middle of the main wuindow
		auto& appWindow = Firefly::Application::Get().GetWindow();
		auto appWindowWidth = appWindow->GetWidth();
		auto appWindowHeight = appWindow->GetHeight();
		auto appWindowPosX = appWindow->GetXPosition();
		auto appWindowPosY = appWindow->GetYPosition();


		std::string popupID = "Import FBX file##Content Browser Import Fbx Window";


		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.3f,0.3f,0.3f,1.f });
		ImGui::SetNextWindowPos(ImVec2(appWindowPosX + appWindowWidth * 0.5f, appWindowPosY + appWindowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::Begin(popupID.c_str(), &myFbxImportData.IsImporting, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			if (myFbxImportData.FromPathAlreadyExisting.size() > 0)
			{
				ImGui::Text("These files already exist!");
				ImGui::Indent();
				for (auto& path : myFbxImportData.FromPathAlreadyExisting)
				{
					ImGui::Text(path.filename().string().c_str());
				}
				ImGui::Unindent();
				ImGui::Text("Do you want to replace these files?");

				if (ImGui::Button("Yes"))
				{
					myFbxImportData.FromPathAlreadyExisting.clear();
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
				{
					myFbxImportData.IsImporting = false;
					myFbxImportData = { };
				}
			}
			else
			{

				ImGui::Text("Paths to import from:");
				ImGui::Indent();
				for (auto& path : myFbxImportData.FromPaths)
				{
					ImGui::Text(path.string().c_str());
				}
				ImGui::Unindent();

				std::string extension = ".ERROR";
				switch (myFbxImportData.FbxTypeToImport)
				{
					case FbxImportData::FbxImportType::Animation:
						extension = ".anim";
						break;
					case FbxImportData::FbxImportType::StaticMesh:
						extension = ".mesh";
						break;
					case FbxImportData::FbxImportType::Skeleton:
						extension = ".skeleton";
						break;

				}

				auto toPath = myFbxImportData.ToDirectoryPath.string() + "\\";
				ImGui::Text("Directory to import to: %s", toPath.c_str());

				ImGui::RadioButton("Animation", reinterpret_cast<int*>(&myFbxImportData.FbxTypeToImport), static_cast<int>(FbxImportData::FbxImportType::Animation));
				ImGui::RadioButton("Mesh", reinterpret_cast<int*>(&myFbxImportData.FbxTypeToImport), static_cast<int>(FbxImportData::FbxImportType::StaticMesh));
				ImGui::RadioButton("Skeleton", reinterpret_cast<int*>(&myFbxImportData.FbxTypeToImport), static_cast<int>(FbxImportData::FbxImportType::Skeleton));

				switch (myFbxImportData.FbxTypeToImport)
				{
					case FbxImportData::FbxImportType::Animation:
						ImGuiUtils::BeginParameters();
						ImGuiUtils::FileParameter("Skeleton", myFbxImportData.SkeletonPath, { ".skeleton" });
						ImGuiUtils::EndParameters();
						break;
					case FbxImportData::FbxImportType::StaticMesh:
						break;
					case FbxImportData::FbxImportType::Skeleton:
						break;
				}
				if (ImGui::Button("Import"))
				{
					for (auto& path : myFbxImportData.FromPaths)
					{

						std::filesystem::path fbxFileToPath = myFbxImportData.ToDirectoryPath.string() + "\\" + path.stem().string() + ".fbx";
						if (std::filesystem::exists(fbxFileToPath))
						{
							std::filesystem::remove(fbxFileToPath);
						}

						std::filesystem::copy_file(path, fbxFileToPath);
						bool existed = false;
						auto binaryFilePath = fbxFileToPath;
						binaryFilePath.replace_extension(extension);
						if (std::filesystem::exists(binaryFilePath))
						{
							std::filesystem::remove(binaryFilePath);
							existed = true;
						}
						switch (myFbxImportData.FbxTypeToImport)
						{
							case FbxImportData::FbxImportType::Animation:
							{
								Firefly::ResourceCache::CompileFBXToAnimation(path, binaryFilePath, myFbxImportData.SkeletonPath);
								break;
							}
							case FbxImportData::FbxImportType::StaticMesh:
							{
								Firefly::ResourceCache::CompileFBXToMesh(path, binaryFilePath);
								break;
							}
							case FbxImportData::FbxImportType::Skeleton:
							{
								Firefly::ResourceCache::CompileFBXToSkeleton(path, binaryFilePath);
								break;
							}
						}
						if (existed)
						{
							if (Firefly::ResourceCache::IsAssetInCache(binaryFilePath))
							{
								Firefly::ResourceCache::ReloadAsset(binaryFilePath);
							}
						}
					}
					myFbxImportData.IsImporting = false;
					myFbxImportData = { };

					RegenerateEntries();
				}
			}
			ImGui::End();
		}
		if (!myFbxImportData.IsImporting)
		{
			myFbxImportData = { };
		}
		ImGui::PopStyleColor();
	}
}

void ContentBrowser::DrawTextureImportWindow()
{
	if (!myTextureImportData.IsImporting)
	{
		return;
	}
	//positiin the window in the middle of the main wuindow
	auto& appWindow = Firefly::Application::Get().GetWindow();
	auto appWindowWidth = appWindow->GetWidth();
	auto appWindowHeight = appWindow->GetHeight();
	auto appWindowPosX = appWindow->GetXPosition();
	auto appWindowPosY = appWindow->GetYPosition();


	std::string popupID = "Import PNG/TGA file##Content Browser Import PNG/TGA Window";


	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.3f,0.3f,0.3f,1.f });
	ImGui::SetNextWindowPos(ImVec2(appWindowPosX + appWindowWidth * 0.5f, appWindowPosY + appWindowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin(popupID.c_str(), &myTextureImportData.IsImporting, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
	{
		if (myTextureImportData.PathAlreadyExisting.size() > 0)
		{
			ImGui::Text("These files already exist!");
			ImGui::Indent();
			for (auto& path : myTextureImportData.PathAlreadyExisting)
			{
				ImGui::Text(path.filename().string().c_str());
			}
			ImGui::Unindent();
			ImGui::Text("Do you want to replace these files?");

			if (ImGui::Button("Yes"))
			{
				myTextureImportData.PathAlreadyExisting.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("No"))
			{
				myTextureImportData.IsImporting = false;
				myTextureImportData = { };
			}
		}
		else
		{

			ImGui::Text("Paths to import from:");
			ImGui::Indent();
			for (auto& path : myTextureImportData.FromPaths)
			{
				ImGui::Text(path.string().c_str());
			}
			ImGui::Unindent();

			std::string extension = ".dds";

			auto toPath = myTextureImportData.ToDirectoryPath.string() + "\\";
			ImGui::Text("Directory to import to: %s", toPath.c_str());

			if (ImGui::RadioButton("BC7 SRGB (for albedo)", myTextureImportData.IsSrgb))
			{
				myTextureImportData.IsSrgb = true;
			}
			if (ImGui::RadioButton("BC7 Normal (for Material map, Normal map etc)", !myTextureImportData.IsSrgb))
			{
				myTextureImportData.IsSrgb = false;
			}
			if (ImGui::Button("Import"))
			{
				for (auto& path : myTextureImportData.FromPaths)
				{
					auto targetPath = toPath + path.stem().string() + ".dds";
					bool existed = false;
					if (std::filesystem::exists(targetPath))
					{
						std::filesystem::remove(targetPath);
						existed = true;
					}


					if (path.extension() == ".tga")
					{
						std::string error;
						if (!LLL::ImageConverter::ConvertTGAToDDS(path, targetPath, myTextureImportData.IsSrgb ? LLL::DDSFormat::BC7UNormSRGB : LLL::DDSFormat::BC7UNorm, error))
						{
							ImGuiUtils::NotifyErrorLocal(error.c_str());
						}
					}
					else if (path.extension() == ".png")
					{
						std::string error;
						if (!LLL::ImageConverter::ConvertPNGToDDS(path, targetPath, myTextureImportData.IsSrgb ? LLL::DDSFormat::BC7UNormSRGB : LLL::DDSFormat::BC7UNorm, error))
						{
							ImGuiUtils::NotifyErrorLocal(error.c_str());
						}
					}

					if (existed)
					{
						if (Firefly::ResourceCache::IsAssetInCache(targetPath))
						{
							Firefly::ResourceCache::ReloadAsset(targetPath);
						}
					}
				}
				myTextureImportData.IsImporting = false;
				myTextureImportData = { };
				RegenerateEntries();
			}
		}
	}
	if (!myTextureImportData.IsImporting)
	{
		myTextureImportData = { };
	}
	ImGui::End();
	ImGui::PopStyleColor();

}

void ContentBrowser::OnOpenEntry(Entry& aEntry)
{
	if (aEntry.IsDirectory)
	{
		SetCurrentEntry(aEntry);
	}
	else
	{
		auto type = GetEntryTypeFromExtension(aEntry.Extension);

		switch (type)
		{
			case EntryType::Prefab:
			{
				PrefabAssetOpenForEditEvent ev(Firefly::ResourceCache::GetAsset<Firefly::Prefab>(aEntry.Path, true)->GetPrefabID());
				Firefly::Application::Get().OnEvent(ev);
				break;
			}
			case EntryType::Scene:
			{
				EditorLoadSceneEvent ev(aEntry.Path);
				Firefly::Application::Get().OnEvent(ev);
				break;
			}
			case EntryType::Animator:
			{
				auto window = EditorLayer::GetOrCreateWindow<AnimatorWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetAnimator(aEntry.Path);
				break;
			}
			case EntryType::Material:
			{
				auto window = EditorLayer::GetOrCreateWindow<MaterialEditorWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetMaterial(aEntry.Path);
				break;
			}
			case EntryType::Emitter:
			{
				auto window = EditorLayer::GetOrCreateWindow<ParticleEditorWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetEmitter(aEntry.Path);
				break;
			}
			case EntryType::AvatarMask:
			{
				auto window = EditorLayer::GetOrCreateWindow<AvatarMaskEditorWindow>();
				window->SetOpen(true);
				window->SetAvatarMask(aEntry.Path.string());
				break;
			}
			case EntryType::BlendSpace:
			{
				auto window = EditorLayer::GetOrCreateWindow<BlendSpaceEditorWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetBlendSpace(aEntry.Path);
				break;
			}
			case EntryType::Pipeline:
			{
				auto window = EditorLayer::GetOrCreateWindow<PipelineWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetPipeline(aEntry.Path);
				break;
			}
			case EntryType::VisualScript:
			{
				auto window = EditorLayer::GetOrCreateWindow<VisualScriptingWindow>();
				window->SetOpen(true);
				window->SetFocused();
				window->OpenVisualScript(aEntry.Path);
				break;
			}
			case EntryType::Animation:
			{
				auto window = EditorLayer::GetOrCreateWindow<AnimationEditor>();
				window->SetOpen(true);
				window->SetFocused();
				window->SetAnimation(aEntry.Path.string());
				break;
			}
			case EntryType::VoiceLineData:
			{
				auto window = EditorLayer::GetOrCreateWindow<VoiceLineDataEditor>();
				window->SetOpen(true);
				window->SetFocused();
				window->Load(aEntry.Path.string());
				break;
			}
			default:
				ShellExecute(0, 0, aEntry.Path.wstring().c_str(), 0, 0, SW_SHOW);
				break;
		}

	}
}



void ContentBrowser::SelectEntry(const std::filesystem::path& aPath)
{
	auto entry = GetEntryFromPath(aPath);
	if (entry)
	{
		OpenEntryInBrowser(entry);
		mySelectedEntry = entry;
	}
	else
	{
		ImGuiUtils::NotifyWarningLocal("Could not select Entry, Have you moved the file?");
	}
}

void ContentBrowser::SetSearchString(std::string aString)
{
	mySearchString = aString;
	CollectSearchEntries();
}

void ContentBrowser::DrawMiddleBar()
{
	auto color = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
	color.x += 0.05f;
	color.y += 0.05f;
	color.z += 0.05f;
	color.w = 1;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, color);



	auto cursorPos = ImGui::GetCursorPos();
	ImGui::BeginChild("##ContentMiddleBarSourcePanel", ImVec2(myMiddleBarResizeValue, 0), true);

	DrawDirectoryHierarchy();
	for (int i = 0; i < 10; i++)
	{
		ImGui::Spacing();
	}
	ImGui::EndChild();
	ImGui::SetCursorPos(ImVec2(cursorPos.x + myMiddleBarResizeValue, cursorPos.y));
	ResizeMiddleBar();
	ImGui::SetCursorPos(ImVec2(cursorPos.x + myMiddleBarRightChildBeginX, cursorPos.y));
	ImGui::BeginChild("##ContentMiddleBarChildEnt", ImVec2(0, ImGui::GetContentRegionAvail().y - 20));
	myContentWindowPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
	myContentWindowSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
	if (ImGui::BeginPopupContextWindow())
	{
		OnRightClickEmptySpace();
		ImGui::EndPopup();
	}

	if (myCurrentRenderingEntry)
	{
		auto texture = Firefly::Renderer::GetSceneFrameBuffer(myRenderID)->GetTextures2D(0);

		Firefly::GraphicsContext::Context()->CopyResource(texture.Get(), texture.Get());

		D3D11_TEXTURE2D_DESC desc;
		texture.Get()->GetDesc(&desc);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		Firefly::GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, tex.GetAddressOf());

		Firefly::GraphicsContext::Context()->CopyResource(tex.Get(), texture.Get());

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		Firefly::Renderer::GetSceneFrameBuffer(myRenderID)->GetColorAttachment(0)->GetDesc(&srvDesc);

		if (myCurrentRenderingEntry->TextureSRV)
		{
			myCurrentRenderingEntry->TextureSRV.Reset();
		}
		Firefly::GraphicsContext::Device()->CreateShaderResourceView(tex.Get(), &srvDesc, myCurrentRenderingEntry->TextureSRV.GetAddressOf());
		tex.Reset();
		myCurrentRenderingEntry = nullptr;
	}
	//ImGui::TableNextColumn();
	if (mySearchString == "")
	{
		DrawEntries(myCurrentEntry->Children);
	}
	else
	{
		DrawEntries(mySearchEntries);
	}
	for (int i = 0; i < 10; i++)
	{
		ImGui::Spacing();
	}
	ImGui::EndChild();



	ImGui::PopStyleColor();
}
void ContentBrowser::ResizeMiddleBar()
{
	ImGuiUtils::ResizeWidget(("AnimatorResizeWidget" + std::to_string(myId)), myMiddleBarResizeValue);
	myMiddleBarRightChildBeginX = myMiddleBarResizeValue + ImGui::GetItemRectSize().x;
}
void ContentBrowser::DrawDirectoryHierarchy()
{
	if (ImGui::CollapsingHeader("Favorites##SourcePanelFavoritesTree",
		ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TreePush();
		if (myFavorites.size() > 0)
		{
			DrawFavoritesRecursive(*myRootEntry);
		}
		else
		{
			ImGui::Text("No Favorites... (V)__|*,,,*|__(V)");
		}
		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Content##SourcePanelContentTree",
		ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TreePush();
		DrawDirectoryHierarchyRecursive(*myRootEntry);
		ImGui::TreePop();
	}
}
void ContentBrowser::DrawFavoritesRecursive(Entry& aEntry)
{
	if (aEntry.IsFavorite)
	{
		ImGui::Image(myStarIcon->GetSRV().Get(), ImVec2(15, 15));
		ImGui::SameLine();
		if (ImGui::Selectable(aEntry.Name.c_str(), myCurrentEntry == &aEntry))
		{
			SetCurrentEntry(aEntry);
		}
		CheckEntryInteractionItem(aEntry, false);
	}
	for (auto& child : aEntry.Children)
	{
		DrawFavoritesRecursive(*child);
	}
}
void ContentBrowser::DrawDirectoryHierarchyRecursive(Entry& aEntry)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	if (!aEntry.Parent) // if entry is root (Assets folder)
	{
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
	}
	//count how many of children are directories
	int childDirCount = 0;
	bool childIsSelected = false;
	for (auto child : aEntry.Children)
	{
		if (child->IsDirectory)
		{
			childDirCount++;
		}
	}
	if (childDirCount == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	ImGui::Image(myDirectoryIcon->GetSRV().Get(), ImVec2(15, 15));
	bool selected = &aEntry == myCurrentEntry;
	if (selected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;

	}

	auto id = aEntry.Name + "##" + aEntry.Path.string();
	ImGui::SameLine();
	bool isOpen = ImGui::TreeNodeEx(id.c_str(), flags);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
		{
			std::filesystem::path path = (const char*)payload->Data;
			MoveEntry(path, aEntry);
			ImGui::EndDragDropTarget();
			return;
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginDragDropSource())
	{
		SetEntryAsPayload(aEntry);
		ImGui::EndDragDropSource();
	}
	auto min = ImGui::GetCurrentContext()->LastItemData.Rect.Min;
	min.x += 25; // offset to ignore arrow, magic number dont touch
	if (ImGui::IsMouseHoveringRect(min, ImGui::GetCurrentContext()->LastItemData.Rect.Max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		SetCurrentEntry(aEntry);
	}
	if (ImGui::BeginPopupContextItem())
	{
		OnRightClickEntry(aEntry);
		ImGui::EndPopup();
	}
	if (isOpen)
	{
		if (childDirCount > 0)
		{
			ImGui::TreePush(id.c_str());
			for (auto entry : aEntry.Children)
			{
				if (entry->IsDirectory)
				{
					DrawDirectoryHierarchyRecursive(*entry);
				}
			}
			ImGui::TreePop();
		}
	}
}
void ContentBrowser::DrawEntryTile(Entry& aEntry)
{
	const auto startCursorPos = ImGui::GetCursorScreenPos();
	ImRect tileRect = ImRect({ startCursorPos.x, startCursorPos.y }, { startCursorPos.x + myTileSize , startCursorPos.y + myTileSize * 1.5f });
	const bool tileHovered = ImGui::IsMouseHoveringRect(tileRect.Min, tileRect.Max);
	const float imageSize = myTileSize * 0.95f;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	auto type = GetEntryTypeFromExtension(aEntry.Extension);
	if (aEntry.Extension == ".dds")
	{
		srv = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(aEntry.Path)->GetSRV();
	}
	else if (type == EntryType::Mesh || type == EntryType::Animation || type == EntryType::Skeleton || type == EntryType::Prefab /*|| type == EntryType::Material*/)
	{
		if (!aEntry.Asset)
		{
			switch (type)
			{
				case EntryType::Mesh:
					aEntry.Asset = Firefly::ResourceCache::GetAsset<Firefly::Mesh>(aEntry.Path);
					break;
				case EntryType::Animation:
					aEntry.Asset = Firefly::ResourceCache::GetAsset<Firefly::Animation>(aEntry.Path);
					break;
				case EntryType::Skeleton:
					aEntry.Asset = Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(aEntry.Path);
					break;
				case EntryType::Prefab:
					aEntry.Asset = Firefly::ResourceCache::GetAsset<Firefly::Prefab>(aEntry.Path);
					break;
			}
			return;
		}

		if (aEntry.TextureSRV)
		{
			srv = aEntry.TextureSRV;
		}
		//if we dont have a srv we need to render one frame of the picture
		bool render = (aEntry.Asset->IsLoaded() && !myCurrentRenderingEntry && !aEntry.TextureSRV);
		render |= (aEntry.Asset->IsLoaded() && type == EntryType::Animation && tileHovered);
		if (type == EntryType::Prefab)
		{
			auto prefabAsset = std::reinterpret_pointer_cast<Firefly::Prefab>(aEntry.Asset);
			//loop through all components and find all meshes
			for (auto ent : prefabAsset->GetEntities())
			{
				if (ent->HasComponent<Firefly::MeshComponent>())
				{
					auto meshComponent = ent->GetComponent<Firefly::MeshComponent>().lock();
					if (meshComponent->GetMesh())
					{
						if (!meshComponent->GetMeshPath().empty())
						{
							meshComponent->LoadAssets();
						}
						else
						{
							render = render && meshComponent->GetMesh()->IsLoaded();
						}
					}
					for (auto mat : meshComponent->GetMaterials())
					{
						if (mat)
						{
							render = render && mat->IsLoaded();
						}
					}
				}
				else if (ent->HasComponent<Firefly::AnimatedMeshComponent>())
				{
					auto meshComponent = ent->GetComponent<Firefly::AnimatedMeshComponent>().lock();
					if (meshComponent->GetMesh())
					{
						if (!meshComponent->GetMeshPath().empty())
						{
							meshComponent->LoadAssets();
						}
						else
						{
							render = render && meshComponent->GetMesh()->IsLoaded();
						}
					}
					for (auto mat : meshComponent->GetMaterials())
					{
						if (mat)
						{
							render = render && mat->IsLoaded();
						}
					}
				}

			}
		}
		if (render)
		{
			Firefly::Renderer::BeginScene(myRenderID);
			auto gridActive = Firefly::Renderer::GetGridActive();
			Firefly::Renderer::SetGridActive(false);

			//Environment Light
			Firefly::EnvironmentData environmentData;
			environmentData.EnvironmentMap = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("FireflyEngine/Defaults/skansen_cubemap.dds");
			environmentData.Intensity = 1.3f;
			Firefly::Renderer::Submit(environmentData);
			//

			myCamera->SetSize(imageSize, imageSize);
			Firefly::Renderer::SubmitActiveCamera(myCamera);

			Utils::Vector3f cameraOffset;
			std::vector< std::vector<Firefly::SubMesh>*> submeshes;

			switch (type)
			{
				case EntryType::Mesh:
				{
					auto mesh = std::reinterpret_pointer_cast<Firefly::Mesh>(aEntry.Asset);
					submeshes.push_back(&mesh->GetSubMeshes());

					for (size_t i = 0; i < mesh->GetSubMeshes().size(); ++i)
					{
						Firefly::MeshSubmitInfo info;
						info.Mesh = &mesh->GetSubMeshes()[i];
						Firefly::Renderer::Submit(info);
					}
					break;
				}
				case EntryType::Skeleton:
				{
					auto mesh = std::reinterpret_pointer_cast<Firefly::AnimatedMesh>(aEntry.Asset);
					submeshes.push_back(&mesh->GetSubMeshes());

					for (size_t i = 0; i < mesh->GetSubMeshes().size(); ++i)
					{
						Firefly::MeshSubmitInfo info;
						info.Mesh = &mesh->GetSubMeshes()[i];
						Firefly::Renderer::Submit(info);
					}

					break;
				}
				case EntryType::Animation:
				{
					auto animation = std::reinterpret_pointer_cast<Firefly::Animation>(aEntry.Asset);
					Firefly::Frame animationFrame;
					if (tileHovered)
					{
						myAnimationTime += Utils::Timer::GetUnscaledDeltaTime();
						while (myAnimationTime > animation->GetDuration())
						{
							myAnimationTime -= animation->GetDuration();
						}
						animationFrame = animation->GetFrame(myAnimationTime, true);
					}
					else
					{
						animationFrame = animation->GetFrame(animation->GetDuration() / 2.f, true);
					}
					auto mesh = animation->GetAnimatedMesh();
					submeshes.push_back(&mesh->GetSubMeshes());


					std::vector<Utils::Matrix4f> matrices;
					animationFrame.CalculateTransforms(animation->GetAnimatedMesh()->GetSkeleton(), matrices);
					cameraOffset.x = matrices.front()(1, 4);
					cameraOffset.y = matrices.front()(2, 4);
					cameraOffset.z = matrices.front()(3, 4);
					for (size_t i = 0; i < animation->GetAnimatedMesh()->GetSubMeshes().size(); ++i)
					{
						Firefly::MeshSubmitInfo info;
						auto& subMesh = (animation->GetAnimatedMesh()->GetSubMeshes()[i]);
						info.Mesh = &subMesh;
						info.SetBoneTransforms(matrices);
						Firefly::Renderer::Submit(info);
					}

					break;
				}

				case EntryType::Prefab:
				{
					try
					{

						Firefly::Scene scene;
						scene.SetLoaded(true);
						auto prefabAsset = std::reinterpret_pointer_cast<Firefly::Prefab>(aEntry.Asset);
						scene.Instantiate(prefabAsset);
						scene.OnRuntimeStart();
						//loop through all components and find all meshes
						for (auto ent : prefabAsset->GetEntities())
						{
							if (ent->HasComponent<Firefly::MeshComponent>())
							{
								auto meshComp = ent->GetComponent<Firefly::MeshComponent>();
								auto meshPath = meshComp.lock()->GetMeshPath();
								if (!meshPath.empty())
								{
									auto mesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>(meshPath, true);
									if (mesh && mesh->GetSubMeshes().size() > 0)
									{
										submeshes.push_back(&mesh->GetSubMeshes());
									}
								}
							}

							if (ent->HasComponent<Firefly::AnimatedMeshComponent>())
							{
								auto meshComp = ent->GetComponent<Firefly::AnimatedMeshComponent>();
								auto meshPath = meshComp.lock()->GetMeshPath();
								if (!meshPath.empty())
								{
									auto mesh = Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(meshPath, true);
									if (mesh->GetSubMeshes().size() > 0)
									{
										submeshes.push_back(&mesh->GetSubMeshes());
									}
								}
							}
						}
						Firefly::AppRenderEvent ev;
						scene.OnEvent(ev);
					}
					catch (...)
					{
						LOGERROR("Failed to load prefab \"{}\" for the content browser", aEntry.Asset->GetPath().string().c_str());
						submeshes.clear();
						aEntry.TextureSRV = myErrorIcon->GetSRV();
					}

					break;
				}

			}

			auto& cameraTransform = myCamera->GetTransform();
			if (submeshes.size() > 0)
			{
				Utils::Vector3f center;
				float radius;

				CalcCameraPositionData(submeshes, center, radius);
				cameraTransform.SetPosition(center + cameraTransform.GetBackward() * radius * myCameraDistMul + cameraOffset);
				myCamera->SetFov(myCameraFov);
				myCamera->UpdateFrustum();

				//Directional Light
				Firefly::DirLightPacket directionalLightPacket;
				directionalLightPacket.ColorAndIntensity = { 1, 1, 1, 1 }; // why not in separate variables
				directionalLightPacket.Direction = { 1, 1, 0 , 0 };// why 4?
				directionalLightPacket.Direction.Normalize();

				//why should i have to do this
				float offset = 4500.f;
				directionalLightPacket.ViewMatrix = Utils::Matrix4f::CreateLookAt((cameraTransform.GetPosition() + Utils::Vector3f(0, 0, 1) * offset) + Utils::Vec4ToVec3(directionalLightPacket.Direction), (cameraTransform.GetPosition() + Utils::Vector3f(0, 0, 1) * offset) - Utils::Vec4ToVec3(directionalLightPacket.Direction), { 0,1,0 });
				directionalLightPacket.ProjMatrix = Utils::Matrix4f::CreateProjectionMatrixOrthographic(10000, 10000, 1.f, 12000.f);
				directionalLightPacket.dirLightInfo.x = static_cast<uint32_t>(true);
				directionalLightPacket.dirLightInfo.y = static_cast<uint32_t>(true);
				Firefly::Renderer::Submit(directionalLightPacket, Firefly::ShadowResolutions::res4098);
				//
			}



			Firefly::Renderer::EndScene();
			//aEntry.TextureSRV = Firefly::Renderer::GetSceneFrameBuffer(myRenderID)->GetColorAttachment(0);




			myCurrentRenderingEntry = &aEntry;
		}
	}

	if (!srv)
	{
		srv = GetIconFromExtension(aEntry.Extension)->GetSRV();
	}

	//Icon
	bool selected = mySelectedEntry == &aEntry;

	ImVec4 color = { 0.3f, 0.3f, 0.3f, 1.f };
	if (selected)
	{
		color = { 0.3f,0.4f,1.f,1.f };
	}
	if (tileHovered)
	{
		color = { color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, 1.f };
	}
	ImGui::GetWindowDrawList()->AddRect(tileRect.Min, { tileRect.Max.x, tileRect.Min.y + myTileSize }, ImGui::GetColorU32(color), 5.f, ImDrawFlags_RoundCornersTop, 2.f);
	ImRect textRect = ImRect({ startCursorPos.x, startCursorPos.y + myTileSize }, tileRect.Max);
	ImGui::GetWindowDrawList()->AddRectFilled(textRect.Min, textRect.Max, ImGui::GetColorU32(color), 5.f, ImDrawFlags_RoundCornersBottom);
	ImGui::GetWindowDrawList()->AddText({ startCursorPos.x + 2,startCursorPos.y + tileRect.GetHeight() - ImGui::CalcTextSize("").y - 2 }, ImGui::GetColorU32({ 0.7f,0.7f,0.7f,1 }),
		ImGuiUtils::AddDotsIfMaxSize(GetDisplayNameFromExtension(aEntry.Extension), myTileSize).c_str());
	ImGui::SetCursorScreenPos({ startCursorPos.x + myTileSize * 0.025f,startCursorPos.y + myTileSize * 0.025f });

	ImGui::Image(srv.Get(), ImVec2(static_cast<float>(imageSize), static_cast<float>(imageSize)));
	if (aEntry.IsFavorite)
	{
		ImGui::GetWindowDrawList()->AddImageQuad(myStarIcon->GetSRV().Get(),
			startCursorPos,
			ImVec2(startCursorPos.x + myTileSize / 4, startCursorPos.y),
			ImVec2(startCursorPos.x + myTileSize / 4, startCursorPos.y + myTileSize / 4),
			ImVec2(startCursorPos.x, startCursorPos.y + myTileSize / 4));
	}

	//

	auto textStartCursorPos = ImGui::GetCursorScreenPos();
	//Text
	if (myRenamingEntry != &aEntry)
	{
		auto displayText = ImGuiUtils::AddDotsIfMaxSize(aEntry.Name, myTileSize);
		auto textSize = ImGui::CalcTextSize(displayText.c_str());
		auto centeredPos = 0;
		ImGui::SetCursorScreenPos({ startCursorPos.x + myTileSize * 0.05f,startCursorPos.y + myTileSize * 1 });
		ImGui::Text(displayText.c_str());
	}
	else
	{
		RenameInputText(aEntry);
	}
	ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x , startCursorPos.y + myTileSize * 1.5f });

	//
	auto endCursorPos = ImGui::GetCursorScreenPos();
	static float textClickTimer = 0;
	static Entry* clickedEntryText = nullptr;

	bool mouseHoveringText = ImGui::IsMouseHoveringRect(textRect.Min, textRect.Max);

	if (mouseHoveringText)
	{
		ImGui::BeginTooltip();
		ImGui::SetTooltip((aEntry.Name + aEntry.Extension).c_str());
		ImGui::EndTooltip();
	}


	ImGui::SetCursorScreenPos(startCursorPos);
	if (ImGui::InvisibleButton(("MainClickable" + aEntry.Path.string()).c_str(), { static_cast<float>(myTileSize),  endCursorPos.y - startCursorPos.y }))
	{
		//if hovering text
		if (mouseHoveringText)
		{
			textClickTimer = 0;
			clickedEntryText = &aEntry;
		}
		mySelectedEntry = &aEntry;
	}
	ImGui::SetCursorScreenPos(endCursorPos);
	CheckEntryInteractionItem(aEntry, true);

	if (clickedEntryText == &aEntry)
	{
		if (mouseHoveringText)
		{
			textClickTimer += Utils::Timer::GetDeltaTime();
			if (textClickTimer >= 0.5f)
			{
				BeginRename(aEntry);
				textClickTimer = 0;
				clickedEntryText = nullptr;
			}
		}
		else
		{
			textClickTimer = 0;
			clickedEntryText = nullptr;
		}
	}


	auto height = endCursorPos.y - startCursorPos.y;



}

void ContentBrowser::DrawEntryList(Entry& aEntry)
{
	std::shared_ptr<Firefly::Texture2D> texture = GetIconFromExtension(aEntry.Extension);
	auto textHeight = ImGui::CalcTextSize("").y;
	auto startDrawPos = ImGui::GetCursorPos();
	ImGui::Image(texture->GetSRV().Get(), ImVec2(textHeight, textHeight));
	ImGui::SameLine();
	//rename
	if (myRenamingEntry != &aEntry)
	{
		auto text = aEntry.Name + aEntry.Extension;
		if (ImGui::Selectable(text.c_str(), mySelectedEntry == &aEntry))
		{
			mySelectedEntry = &aEntry;
		}
		const auto textSize = ImGui::CalcTextSize(text.c_str());

		const auto entryXPos = textSize.x + 50;
		if (entryXPos > myFurthestToTheRightListEntryX)
		{
			myFurthestToTheRightListEntryX = entryXPos;
		}
		CheckEntryInteractionItem(aEntry, true);
		if (aEntry.IsFavorite)
		{
			ImGui::SameLine();
			ImGui::Image(myStarIcon->GetSRV().Get(), ImVec2(textHeight, textHeight));
		}
	}
	else
	{
		RenameInputText(aEntry);
	}

}

void ContentBrowser::CheckEntryInteractionItem(Entry& aEntry, bool aCanDragFlag)
{
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			OnOpenEntry(aEntry);
		}
	}
	if (aCanDragFlag)
	{
		if (ImGui::BeginDragDropSource())
		{
			SetEntryAsPayload(aEntry);
			ImGui::EndDragDropSource();
		}
	}
	if (aEntry.IsDirectory)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
			{
				std::filesystem::path path = (const char*)payload->Data;
				MoveEntry(path, aEntry);
				ImGui::EndDragDropTarget();
				return;
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (ImGui::BeginPopupContextItem())
	{
		mySelectedEntry = &aEntry;
		OnRightClickEntry(aEntry);
		ImGui::EndPopup();
	}
}

void ContentBrowser::RenameInputText(Entry& aEntry)
{
	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		EndRename(aEntry.Path);
	}
	ImGui::SetKeyboardFocusHere();
	bool renamed = ImGui::InputText("##renamingContentBrowser", &myRenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
	renamed = renamed || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemHovered();

	if (renamed)
	{
		auto newPath = aEntry.Path.parent_path().string() + "\\" + myRenameBuffer + aEntry.Extension;
		if (!std::filesystem::exists(newPath))
		{
			std::filesystem::rename(aEntry.Path, newPath);
			RegenerateEntries();
		}
		else
		{
			ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Entry with path:\"%s\"Already exists, entry was not renamed",newPath.c_str() });
		}
		EndRename(newPath);
	}
}

void ContentBrowser::DrawEntries(std::vector<std::shared_ptr<Entry>> someEntries)
{
	if (myCurrentRenderMode == RenderMode::Tiles)
	{
		int leftPadding = 10;
		auto windowSize = ImGui::GetContentRegionAvail();
		windowSize.x -= leftPadding;
		int itemWidth = myTileSize + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().CellPadding.x;
		int count = static_cast<int>(std::floor(windowSize.x / itemWidth));
		/*count--;*/
		if (count <= 0)
		{
			count = 1;
		}
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + leftPadding);
		if (ImGui::BeginTable("##ContentBrowserTable", count, ImGuiTableFlags_SizingFixedFit))
		{
			for (auto& entry : someEntries)
			{
				if (!myShowHidden)
				{
					if (GetEntryTypeFromExtension(entry->Extension) == EntryType::FBX)
					{
						continue;
					}
				}
				ImGui::TableNextColumn();
				ImGui::PushID(entry->Path.c_str());
				DrawEntryTile(*entry);
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	else if (myCurrentRenderMode == RenderMode::List)
	{
		myFurthestToTheRightListEntryX = 0;
		for (auto& entry : someEntries)
		{
			if (!myShowHidden)
			{
				if (GetEntryTypeFromExtension(entry->Extension) == EntryType::FBX)
				{
					continue;
				}
			}

			ImGui::PushID(entry->Path.c_str());
			DrawEntryList(*entry);
			ImGui::PopID();
		}
		auto cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ myFurthestToTheRightListEntryX, 0 });
		if (GetSelectedEntry())
		{
			if (GetSelectedEntry()->TextureSRV)
			{
				ImGui::Image(GetSelectedEntry()->TextureSRV.Get(), { ImGui::GetWindowSize().x - myFurthestToTheRightListEntryX, ImGui::GetWindowSize().y });
			}
		}
	}
}

void ContentBrowser::SetEntryAsPayload(Entry& aEntry)
{
	ImGui::SetDragDropPayload("FILE", aEntry.Path.string().c_str(), aEntry.Path.string().size() + 1);
}


void ContentBrowser::DrawBottomBar()
{
	auto cursorPos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(cursorPos.x + myMiddleBarRightChildBeginX, cursorPos.y));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4, 0.4, 0.4, 1));
	ImGui::BeginChild("##ContentBrowserBottomBar", ImVec2(0, 0), false,
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImGui::PopStyleColor();

	if (mySelectedEntry)
	{
		ImGui::Text(mySelectedEntry->Path.string().c_str());
	}
	ImGui::EndChild();
}

void ContentBrowser::OnImGui()
{
	DrawFbxImportWindow();
	DrawTextureImportWindow();
	if (myCreateWindowFlag)
	{
		UpdateCreateWindow();
	}
	myWindowMin = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
	myWindowMax = { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight() };

	if (myIsCompiling)
	{
		std::string desc = "DO NOT TURN OFF THE ENGINE!";
		auto textSize = ImGui::CalcTextSize(desc.c_str());
		ImGui::OpenPopup("Compiling FBX files...");
		ImGui::BeginPopupModal("Compiling FBX files...", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::ProgressBar(static_cast<float>(myCurrentCompileCount) / myTotalCompileCount);
		ImGui::Text(myCompilingPath.string().c_str());
		ImGui::Text(desc.c_str());

		ImGui::EndPopup();
		if (myCompilingDone)
		{
			myIsCompiling = false;
			myCompilingDonePopupFlag = true;
			RegenerateEntries();
		}
	}
	else if (myCompilingDonePopupFlag)
	{
		ImGui::OpenPopup("Compiling FBX files done!");
		ImGui::BeginPopupModal("Compiling FBX files done!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Compiling done!");
		if (ImGui::Button("Ok"))
		{
			ImGui::CloseCurrentPopup();
			myCompilingDonePopupFlag = false;
		}
		ImGui::EndPopup();
	}

	DrawTopBar();

	DrawMiddleBar();




	if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
	{
		mySelectedEntry = nullptr;
	}



	DrawBottomBar();



	//should be last to make the outline draw over everything else
	AcceptEntityToCreatePrefab();
}

void ContentBrowser::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<SaveEditorSettingsEvent>([this](SaveEditorSettingsEvent& aEvent)
		{
			auto& json = aEvent.GetJson();
			auto& contentBrowserJson = json["ContentBrowser"];

			contentBrowserJson["TileSize"] = myTileSize;
			contentBrowserJson["RenderMode"] = static_cast<int>(myCurrentRenderMode);
			contentBrowserJson["MiddleBarResizeValue"] = myMiddleBarResizeValue;
			auto& favoritesJson = contentBrowserJson["Favorites"];
			for (int i = 0; i < myFavorites.size(); i++)
			{
				favoritesJson[i] = myFavorites[i];
			}
			return false;
		});

	dispatcher.Dispatch<LoadEditorSettingsEvent>([this](LoadEditorSettingsEvent& aEvent)
		{
			auto& json = aEvent.GetJson();
			if (json.contains("ContentBrowser"))
			{
				auto& contentBrowserJson = json["ContentBrowser"];
				if (contentBrowserJson.contains("TileSize"))
				{
					myTileSize = contentBrowserJson["TileSize"];
				}
				if (contentBrowserJson.contains("RenderMode"))
				{
					myCurrentRenderMode = static_cast<RenderMode>(contentBrowserJson["RenderMode"]);
				}
				if (contentBrowserJson.contains("MiddleBarResizeValue"))
				{
					myMiddleBarResizeValue = contentBrowserJson["MiddleBarResizeValue"];
				}
				if (contentBrowserJson.contains("Favorites"))
				{
					auto& favoritesJson = contentBrowserJson["Favorites"];
					for (auto& favorite : favoritesJson)
					{
						myFavorites.push_back(favorite);
					}
				}
			}
			RegenerateEntries();
			return false;
		});

	if (!myLockedFlag)
	{
		dispatcher.Dispatch<SearchInContentBrowserEvent>([this](SearchInContentBrowserEvent& aEvent)
			{
				mySearchString = aEvent.GetSearchString();
				CollectSearchEntries();
				return false;
			});

		dispatcher.Dispatch<SelectInContentBrowserEvent>([this](SelectInContentBrowserEvent& aEvent)
			{
				SelectEntry(aEvent.GetPath());
				return false;
			});
	}

}

Entry* ContentBrowser::GetSelectedEntry()
{
	return mySelectedEntry;
}

void ContentBrowser::WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DROPFILES)
	{
		HDROP hDrop = (HDROP)wParam;
		UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (UINT i = 0; i < nFiles; ++i)
		{
			char szFileName[MAX_PATH];
			DragQueryFileA(hDrop, i, szFileName, MAX_PATH);
			POINT dropPoint;
			DragQueryPoint(hDrop, &dropPoint);
			const auto& window = Firefly::Application::Get().GetWindow();
			const Utils::Vector2f appWindowPos = { static_cast<float>(window->GetXPosition()), static_cast<float>(window->GetYPosition()) };
			auto localPos = myContentWindowPos - appWindowPos;


			if (dropPoint.x < (localPos + myContentWindowSize).x && dropPoint.x > localPos.x && dropPoint.y < (localPos + myContentWindowSize).y && dropPoint.y > localPos.y)
			{
				std::string path = szFileName;
				auto filename = path.substr(path.find_last_of("\\") + 1);
				std::string newPath = myCurrentEntry->Path.string() + "\\" + filename;
				//get the extension
				auto extension = filename.substr(filename.find_last_of("."));
				if (extension == ".fbx")
				{
					myFbxImportData.IsImporting = true;
					myFbxImportData.FromPaths.push_back(path);
					myFbxImportData.ToDirectoryPath = myCurrentEntry->Path.string();
					auto targetPath = myFbxImportData.ToDirectoryPath.string() + "\\" + std::filesystem::path(path).filename().string();
					if (std::filesystem::exists(targetPath))
					{
						myFbxImportData.FromPathAlreadyExisting.push_back(targetPath);
					}
				}
				else if (extension == ".png" || extension == ".tga")
				{
					myTextureImportData.FromPaths.push_back(path);
					auto targetPath = newPath;
					targetPath.replace(targetPath.find_last_of("."), targetPath.length(), ".dds");
					if (std::filesystem::exists(targetPath))
					{
						myTextureImportData.PathAlreadyExisting.push_back(targetPath);
					}
					myTextureImportData.IsImporting = true;
					myTextureImportData.ToDirectoryPath = myCurrentEntry->Path.string();
				}
				else
				{
					//Texture already exists in folder, want to replace and reload all materials?
					if (std::filesystem::exists(newPath) && extension == ".dds")
					{
						ImGuiUtils::OpenYesNoPopupModal("Replace texture \"" + filename + "\"?",
							"That texture already exists! \"" + filename + "\". Do you want to replace it? Note: This can only be reverted with Perforce!",
							//Yes
							[=]()
							{
								std::filesystem::remove_all(newPath);

								bool succeded = CopyFileA(path.c_str(), (std::filesystem::current_path().string() + "\\" + newPath).c_str(), true);
								if (succeded)
								{
									RegenerateEntries();

									if (Firefly::ResourceCache::IsAssetInCache(newPath))
									{
										Firefly::ResourceCache::ReloadAsset(newPath);
									}

									ImGuiUtils::NotifyInfo("{} was replaced.", newPath);
								}
								else
								{
									ImGuiUtils::NotifyError("Error replacing {}! Revert the old one back through Perforce.", newPath);
								}
							},
							//No
							[=]()
							{
								ImGuiUtils::NotifyWarning("{} was not replaced", newPath);
							});
					}
					else
					{
						int existsCount = 0;
						std::string testPath = newPath;
						while (std::filesystem::exists(testPath))
						{
							testPath = newPath;
							auto it = newPath.find_last_of(".");
							testPath.insert(it, std::to_string(existsCount));
							existsCount++;
						}
						newPath = testPath;
						newPath = std::filesystem::current_path().string() + "\\" + newPath;

						bool succeded = CopyFileA(path.c_str(),
							(newPath).c_str(),
							true);
						if (succeded)
						{
							RegenerateEntries();
						}
					}

				}
			}
			else
			{
				ImGuiUtils::NotifyError("You can only drop files in the content browser window! File was not added to directory!");
			}
		}
		DragFinish(hDrop);
	}
}

std::vector<Entry*> ContentBrowser::GetAllEntriesOfTypes(std::vector<EntryType> someTypes)
{
	auto allEntries = GetChildrenRecursive(*myRootEntry);
	std::vector<Entry*> entries;
	for (auto& entry : allEntries)
	{
		if (std::find(someTypes.begin(), someTypes.end(), GetEntryTypeFromExtension(entry->Extension)) != someTypes.end())
		{
			entries.push_back(entry.get());
		}
	}
	return entries;
}

void ContentBrowser::BeginRename(Entry& aEntry)
{
	myRenamingEntry = &aEntry;
	myRenameBuffer = aEntry.Name;
	myRenamingFlag = true;
}

void ContentBrowser::EndRename(const std::filesystem::path& aNewPath)
{
	if (myRenamingEntry->Extension == ".ffs")
	{
		const std::string newName = myRenameBuffer;
		auto info = Firefly::ShaderImporter::LoadDataFromFile(aNewPath);
		auto oldInfo = info;
		MaterialEditorWindow::Reset();
		std::filesystem::rename(info.pixelShaderPath, info.pixelShaderPath.parent_path().string() + "\\" + newName + ".hlsl");
		info.pixelShaderPath = info.pixelShaderPath.parent_path().string() + "\\" + newName + ".hlsl";
		info.shaderKey = newName;
		info.shaderKeyAnimation = newName + "Animation";
		Firefly::ShaderImporter::UpdateFileFromData(aNewPath, info);
		Firefly::ShaderImporter::RegistryKeys(info);
		//Firefly::ShaderImporter::RemoveKeys(oldInfo);

	}
	myRenamingFlag = false;
	myRenameBuffer = "";
	myRenamingEntry = nullptr;
}

void ContentBrowser::AcceptEntityToCreatePrefab()
{

	if (const ImGuiPayload* payload = ImGuiUtils::DragDropWindow("ENTITY"))
	{
		Ptr<Firefly::Entity> entity = *((Ptr<Firefly::Entity>*)payload->Data);

		std::string path = myCurrentEntry->Path.string() + "\\" + entity.lock()->GetName() + ".prefab";
		Firefly::SerializationUtils::SaveEntityAsPrefab(entity, path);
		RegenerateEntries();
	}
}

void ContentBrowser::OnRightClickEmptySpace()
{
	if (ImGui::MenuItem("Create Pipeline"))
	{
		OpenCreateWindow("Create shader Pipeline", EntryType::Pipeline, myCurrentEntry->Path.string() + "\\");

		RegenerateEntries();

	}
	if (ImGui::MenuItem("Create Material"))
	{
		std::filesystem::path materialPath = myCurrentEntry->Path.string() + "\\" + "newMaterial.mat";
		int i = 0;
		while (std::filesystem::exists(materialPath))
		{
			materialPath.remove_filename();
			materialPath += "newMaterial" + std::to_string(i) + ".mat";
			i++;
		}
		std::ifstream fin("FireflyEngine/Defaults/Default.mat");
		std::ofstream fout(materialPath);
		fout << fin.rdbuf();
		RegenerateEntries();

		BeginRename(*GetEntryFromPath(materialPath));

	}
	if (ImGui::MenuItem("Create Folder"))
	{
		std::filesystem::path folderPath = myCurrentEntry->Path.string() + "\\" + "New Folder";
		int i = 0;
		while (std::filesystem::exists(folderPath))
		{
			folderPath.remove_filename();
			folderPath += "New Folder" + std::to_string(i);
			i++;
		}

		std::filesystem::create_directory(folderPath);
		RegenerateEntries();
		BeginRename(*GetEntryFromPath(folderPath));
	}
	if (ImGui::MenuItem("Create BlendSpace"))
	{
		myCreateWindowBlendSpaceToCreate = CreateRef<Firefly::BlendSpace>();
		OpenCreateWindow("Create Blend Space", EntryType::BlendSpace, myCurrentEntry->Path.string() + "\\");
	}
	if (ImGui::MenuItem("Create Avatar Mask"))
	{
		myCreateWindowAvatarMaskToCreate = CreateRef<Firefly::AvatarMask>();
		OpenCreateWindow("Create Avatar Mask", EntryType::AvatarMask, myCurrentEntry->Path.string() + "\\");
	}
	if (ImGui::MenuItem("Refresh"))
	{
		RegenerateEntries();
	}


}

void ContentBrowser::OnRightClickEntry(Entry& aEntry)
{
	if (aEntry.Name == "Primitives" && aEntry.Parent->Name == "Primitives")
	{
		return;
	}
	if (ImGui::MenuItem("Open"))
	{
		OnOpenEntry(aEntry);
	}
	if (ImGui::MenuItem("Rename"))
	{
		BeginRename(aEntry);
	}
	if (ImGui::MenuItem("Show In Explorer"))
	{
		std::string command = "explorer.exe /select,";
		command += aEntry.Path.string();
		system(command.c_str());
	}
	if (aEntry.IsDirectory)
	{

		if (!aEntry.IsFavorite)
		{
			if (ImGui::MenuItem("Favorite"))
			{
				SetAsFavorite(aEntry);
			}
		}
		else
		{
			//remove
			if (ImGui::MenuItem("Unfavorite"))
			{
				RemoveFromFavorites(aEntry);
			}
		}

	}
	else
	{
		if (aEntry.Extension != ".ffs" && aEntry.Extension != ".prefab")
		{
			if (ImGui::MenuItem("Duplicate"))
			{
				DuplicateEntry(aEntry);
			}
		}

		/*if (aEntry.Extension == ".fbx")
		{
			if (ImGui::MenuItem("Reimport"))
			{
				Firefly::ResourceCache::CompileBinaryFBX(aEntry.Path);
				RegenerateEntries();
			}
		}*/

	}
	if (ImGui::MenuItem("Delete"))
	{
		ImGuiUtils::OpenYesNoPopupModal("About to delete \"" + aEntry.Name + "\"!",
			"Are you sure you want to delete entry: \"" + aEntry.Path.string() + "\"? This action CANNOT be undone! The file will not appear in the recycle bin!",
			[&]()
			{
				if (aEntry.Extension == ".mesh" || aEntry.Extension == ".anim" || aEntry.Extension == ".skeleton")
				{
					auto fbxPath = aEntry.Path;
					fbxPath.replace_extension(".fbx");
					std::filesystem::remove_all(fbxPath);
				}
				Firefly::ResourceCache::UnloadAsset(aEntry.Path);
				std::filesystem::remove_all(aEntry.Path);
				RegenerateEntries();
			},
			[]() {});
	}
}

void ContentBrowser::SetAsFavorite(Entry& aEntry)
{
	myFavorites.push_back(aEntry.Path);
	aEntry.IsFavorite = true;
}

void ContentBrowser::RemoveFromFavorites(Entry& aEntry)
{
	myFavorites.erase(std::remove(myFavorites.begin(), myFavorites.end(), aEntry.Path), myFavorites.end());
	aEntry.IsFavorite = false;
}

void ContentBrowser::DuplicateEntry(Entry& aEntry)
{
	auto pathAsStr = aEntry.Path.string();
	auto it = pathAsStr.find_last_of('.');
	int i = 0;
	auto destination = pathAsStr;
	while (std::filesystem::exists(destination))
	{
		destination = pathAsStr;
		destination.insert(it, " - copy" + std::to_string(i));
		i++;
	}
	std::filesystem::copy_file(aEntry.Path, destination);
	RegenerateEntries();
}

void ContentBrowser::MoveEntry(std::filesystem::path aPath, Entry& aToEntry)
{
	std::filesystem::path newPath = aToEntry.Path / aPath.filename();
	if (!std::filesystem::exists(newPath))
	{
		//make sure we didnt move into ourself or our children

		auto movingName = aPath.stem();
		auto toPathNames = GetEntryNamesFromPath(aToEntry.Path);

		bool valid = true;
		for (int i = 0; i < toPathNames.size(); i++)
		{
			if (movingName == toPathNames[i])
			{
				valid = false;
				break;
			}
		}

		if (valid)
		{
			std::filesystem::rename(aPath, newPath);
			RegenerateEntries();
		}
		else
		{
			ImGuiToast tos(ImGuiToastType_Error, 3000, "Cannot move Entry into itself or into any of it's children!", newPath.string().c_str());
			tos.windowPos = { myContentWindowPos.x, myContentWindowPos.y };
			tos.windowSize = { myContentWindowSize.x, myContentWindowSize.y + 20 };
			ImGui::InsertNotification(tos);
		}
	}
	else
	{
		ImGuiToast tos(ImGuiToastType_Error, 3000, "Entry with path:\n\"%s\"\nAlready exists, entry was not moved!", newPath.string().c_str());
		tos.windowPos = { myContentWindowPos.x, myContentWindowPos.y };
		tos.windowSize = { myContentWindowSize.x, myContentWindowSize.y + 20 };
		ImGui::InsertNotification(tos);
	}
}

void ContentBrowser::CalcCameraPositionData(std::vector< std::vector<Firefly::SubMesh>*>& someSubMeshes, Utils::Vector3f& aCenter, float& aRadius)
{
	int vertCount = 0;
	Utils::Vector3f min, max;
	for (int i = 0; i < someSubMeshes.size(); i++)
	{
		for (auto& submesh : *someSubMeshes[i])
		{
			vertCount += submesh.GetVerticesPositions().size();
			for (int j = 0; j < submesh.GetVerticesPositions().size(); j++)
			{
				const auto& vert = submesh.GetVerticesPositions()[j];
				if (vert.x > max.x)
				{
					max.x = vert.x;
				}
				if (vert.x < min.x)
				{
					min.x = vert.x;
				}
				if (vert.y > max.y)
				{
					max.y = vert.y;
				}
				if (vert.y < min.y)
				{
					min.y = vert.y;
				}
				if (vert.z > max.z)
				{
					max.z = vert.z;
				}
				if (vert.z < min.z)
				{
					min.z = vert.z;
				}
			}
		}
	}
	aCenter = (max + min) / 2.f;

	float furthestDistanceSqr = 0;
	for (int i = 0; i < someSubMeshes.size(); i++)
	{
		for (auto& submesh : *someSubMeshes[i])
		{
			for (int j = 0; j < submesh.GetVerticesPositions().size(); j++)
			{
				auto distance = (submesh.GetVerticesPositions()[j] - aCenter).LengthSqr();
				if (distance > furthestDistanceSqr)
				{
					furthestDistanceSqr = distance;
				}
			}
		}
	}
	aRadius = sqrt(furthestDistanceSqr);
}

void ContentBrowser::CollectSearchEntries()
{
	//find all entries that contain the search string
	mySearchEntries.clear();
	auto children = GetChildrenRecursive(*myRootEntry);
	std::string searchStringLower = mySearchString;
	std::transform(searchStringLower.begin(), searchStringLower.end(), searchStringLower.begin(), [](unsigned char c) { return std::tolower(c); });
	for (auto entry : children)
	{
		std::string nameLower = entry->Name;
		std::string extensionLower = entry->Extension;

		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c) { return std::tolower(c); });
		std::transform(extensionLower.begin(), extensionLower.end(), extensionLower.begin(), [](unsigned char c) { return std::tolower(c); });


		bool shouldAdd = false;
		auto offset = searchStringLower.find(std::string("t:"));
		//only search for name before t:
		auto nameSearch = searchStringLower;
		if (offset != std::string::npos)
		{
			nameSearch = searchStringLower.substr(0, offset);
		}
		//remove all trailing Spaces
		nameSearch = nameSearch.substr(0, nameSearch.find_last_not_of(' ') + 1);
		if (nameLower.find(nameSearch) != std::string::npos || nameSearch == "")
		{
			shouldAdd = true;
		}
		if (offset != std::string::npos)
		{
			auto extensionSearch = searchStringLower.substr(offset + 2, searchStringLower.size() - offset - 2);
			if (extensionSearch[0] != '.')
			{
				extensionSearch = "." + extensionSearch;
			}
			if (extensionLower.find(extensionSearch) != std::string::npos)
			{
				shouldAdd = shouldAdd && true;
			}
			else
			{
				shouldAdd = false;
			}
		}
		if (shouldAdd)
		{
			mySearchEntries.push_back(entry);
		}
	}
}

void ContentBrowser::OnPressBackButton()
{
	auto entry = myPrevOpenEntriesStack.back();
	myForwardEntryStack.push_back(myCurrentEntry);
	myCurrentEntry = entry;
	myPrevOpenEntriesStack.pop_back();
}

void ContentBrowser::OnPressForwardButton()
{
	auto entry = myForwardEntryStack.back();
	myPrevOpenEntriesStack.push_back(myCurrentEntry);
	myCurrentEntry = entry;
	myForwardEntryStack.pop_back();
}


std::vector<std::shared_ptr<Entry>> ContentBrowser::GetChildrenRecursive(Entry& aEntry)
{
	std::vector<std::shared_ptr<Entry>> children;
	for (auto& entry : aEntry.Children)
	{
		children.push_back(entry);
		auto childrenRecursive = GetChildrenRecursive(*entry);
		for (auto child : childrenRecursive)
		{
			children.push_back(child);
		}
	}
	return children;
}

EntryType ContentBrowser::GetEntryTypeFromExtension(std::string aExtension)
{
	std::string extension = aExtension;
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
	EntryType returnVal = EntryType::Unknown;
	if (extension == "")
	{
		returnVal = EntryType::Directory;
	}
	else if (extension == ".mesh")
	{
		returnVal = EntryType::Mesh;
	}
	else if (extension == ".dds")
	{
		returnVal = EntryType::Texture;
	}
	else if (extension == ".mat")
	{
		returnVal = EntryType::Material;
	}
	else if (extension == ".prefab")
	{
		returnVal = EntryType::Prefab;
	}
	else if (extension == ".ttf")
	{
		returnVal = EntryType::Font;
	}
	else if (extension == ".emitter")
	{
		returnVal = EntryType::Emitter;
	}
	else if (extension == ".animator")
	{
		returnVal = EntryType::Animator;
	}
	else if (extension == ".scene")
	{
		returnVal = EntryType::Scene;
	}
	else if (extension == ".ogg" || extension == ".wav" || extension == ".mp3")
	{
		returnVal = EntryType::Audio;
	}
	else if (extension == ".anim")
	{
		returnVal = EntryType::Animation;
	}
	else if (extension == ".skeleton")
	{
		returnVal = EntryType::Skeleton;
	}
	else if (extension == ".fbx")
	{
		returnVal = EntryType::FBX;
	}
	else if (extension == ".bank")
	{
		returnVal = EntryType::SoundBank;
	}
	else if (extension == ".blend")
	{
		returnVal = EntryType::BlendSpace;
	}
	else if (extension == ".ffpl")
	{
		returnVal = EntryType::Pipeline;
	}
	else if (extension == ".mask")
	{
		returnVal = EntryType::AvatarMask;
	}
	else if (extension == ".flow")
	{
		returnVal = EntryType::VisualScript;
	}
	else if (extension == ".state")
	{
		returnVal = EntryType::GameplayStateMachine;
	}
	else if (extension == ".vld")
	{
		returnVal = EntryType::VoiceLineData;
	}

	return returnVal;
}

std::string ContentBrowser::GetExtensionFromEntryType(EntryType aType)
{
	std::string returnVal = "";
	switch (aType)
	{
		case EntryType::Mesh:
			returnVal = ".mesh";
			break;
		case EntryType::Texture:
			returnVal = ".dds";
			break;
		case EntryType::Material:
			returnVal = ".mat";
			break;
		case EntryType::Prefab:
			returnVal = ".prefab";
			break;
		case EntryType::Font:
			returnVal = ".ttf";
			break;
		case EntryType::Emitter:
			returnVal = ".emitter";
			break;
		case EntryType::Animator:
			returnVal = ".animator";
			break;
		case EntryType::Scene:
			returnVal = ".scene";
			break;
		case EntryType::Audio:
			returnVal = ".ogg";
			break;
		case EntryType::Animation:
			returnVal = ".anim";
			break;
		case EntryType::Skeleton:
			returnVal = ".skeleton";
			break;
		case EntryType::FBX:
			returnVal = ".fbx";
			break;
		case EntryType::SoundBank:
			returnVal = ".bank";
			break;
		case EntryType::BlendSpace:
			returnVal = ".blend";
			break;
		case EntryType::Pipeline:
			returnVal = ".ffpl";
			break;
		case EntryType::AvatarMask:
			returnVal = ".mask";
			break;
		case EntryType::VisualScript:
			returnVal = ".flow";
			break;
		case EntryType::GameplayStateMachine:
			returnVal = ".state";
			break;
		case EntryType::VoiceLineData:
			returnVal = ".vld";
			break;
	}
	return returnVal;
}

std::shared_ptr<Firefly::Texture2D> ContentBrowser::GetIconFromExtension(std::string aExtension)
{
	return GetIconFromEntryType(GetEntryTypeFromExtension(aExtension));
}

std::shared_ptr<Firefly::Texture2D> ContentBrowser::GetIconFromEntryType(EntryType aType)
{
	std::shared_ptr<Firefly::Texture2D> texture = myFileIcon;
	switch (aType)
	{
		case EntryType::Unknown:
			texture = myFileIcon;
			break;
		case EntryType::Directory:
			texture = myDirectoryIcon;
			break;
		case EntryType::Animator:
			texture = myAnimatorIcon;
			break;
		case EntryType::Animation:
			texture = myAnimationIcon;
			break;
		case EntryType::Mesh:
			texture = myMeshIcon;
			break;
		case EntryType::Skeleton:
			texture = mySkeletonIcon;
			break;
		case EntryType::Material:
			texture = myMaterialIcon;
			break;
		case EntryType::Texture:
			texture = myTextureIcon;
			break;
		case EntryType::Pipeline:
			texture = myPipelineIcon;
			break;
		case EntryType::Prefab:
			texture = myPrefabIcon;
			break;
		case EntryType::Font:
			texture = myFontIcon;
			break;
		case EntryType::Emitter:
			texture = myEmitterIcon;
			break;
		case EntryType::Scene:
			texture = mySceneIcon;
			break;
		case EntryType::Audio:
			texture = myAudioIcon;
			break;
		case EntryType::FBX:
			texture = myFBXIcon;
			break;
		case EntryType::SoundBank:
			texture = mySoundBankIcon;
			break;
		case EntryType::BlendSpace:
			texture = myBlendspaceIcon;
			break;
		case EntryType::AvatarMask:
			texture = myAvatarMaskIcon;
			break;
		case EntryType::VisualScript:
			texture = myVisualScriptIcon;
			break;
		case EntryType::GameplayStateMachine:
			texture = myGameplayStateMachineIcon;
			break;
		case EntryType::VoiceLineData:
			texture = myVoiceLineDataIcon;
			break;
	}
	return texture;
}

std::string ContentBrowser::GetDisplayNameFromEntryType(EntryType aType)
{
	std::string returnVal = "";
	switch (aType)
	{
		case EntryType::Unknown:
			returnVal = "Unknown";
			break;
		case EntryType::Directory:
			returnVal = "Directory";
			break;
		case EntryType::Animator:
			returnVal = "Animator";
			break;
		case EntryType::Animation:
			returnVal = "Animation";
			break;
		case EntryType::Mesh:
			returnVal = "Static Mesh";
			break;
		case EntryType::Skeleton:
			returnVal = "Skeleton";
			break;
		case EntryType::Material:
			returnVal = "Material";
			break;
		case EntryType::Texture:
			returnVal = "Texture";
			break;
		case EntryType::Pipeline:
			returnVal = "Pipeline";
			break;
		case EntryType::Prefab:
			returnVal = "Prefab";
			break;
		case EntryType::Font:
			returnVal = "Font";
			break;
		case EntryType::Emitter:
			returnVal = "Emitter";
			break;
		case EntryType::Scene:
			returnVal = "Scene";
			break;
		case EntryType::Audio:
			returnVal = "Audio File";
			break;
		case EntryType::FBX:
			returnVal = "FBX";
			break;
		case EntryType::SoundBank:
			returnVal = "Sound Bank";
			break;
		case EntryType::BlendSpace:
			returnVal = "Blendspace";
			break;
		case EntryType::VisualScript:
			returnVal = "Visual Script";
			break;
		case EntryType::AvatarMask:
			returnVal = "Avatar Mask";
			break;
	}
	return returnVal;
}

std::string ContentBrowser::GetDisplayNameFromExtension(const std::string& aExtension)
{
	return GetDisplayNameFromEntryType(GetEntryTypeFromExtension(aExtension));
}

void ContentBrowser::OpenCreateWindow(const std::string& aName, EntryType aType, const std::filesystem::path& aDirectoryPath)
{
	myCreateWindowName = aName;
	myCreateWindowAssetType = aType;
	myCreateWindowDirectoryPath = aDirectoryPath;
	myCreateWindowFlag = true;
}

void ContentBrowser::UpdateCreateWindow()
{
	ImGui::OpenPopup((myCreateWindowName + "##Content Browser Create Window").c_str());
	if (ImGui::BeginPopupModal((myCreateWindowName + "##Content Browser Create Window").c_str(), &myCreateWindowFlag))
	{
		ImGui::Text("Name:");
		ImGui::SameLine();
		ImGui::InputText("##Content Browser Create Window Asset Name", &myCreateWindowAssetName);
		ImGui::SameLine();
		ImGui::Text(GetExtensionFromEntryType(myCreateWindowAssetType).c_str());
		if (myCreateWindowAssetType == EntryType::BlendSpace)
		{
			auto& blendSpace = myCreateWindowBlendSpaceToCreate;
			ImGuiUtils::BeginParameters();
			ImGuiUtils::FileParameter("Skeleton", blendSpace->myMeshPath, ".skeleton");
			ImGuiUtils::EndParameters();
		}
		if (myCreateWindowAssetType == EntryType::AvatarMask)
		{
			auto& avatarMask = myCreateWindowAvatarMaskToCreate;
			ImGuiUtils::BeginParameters();
			if (ImGuiUtils::FileParameter("Skeleton", myCreateWindowSkeletonPath, ".skeleton"))
			{
				avatarMask->SetAnimatedMesh(Firefly::ResourceCache::GetAsset<Firefly::AnimatedMesh>(myCreateWindowSkeletonPath));
			}
			ImGuiUtils::EndParameters();
		}
		else if (myCreateWindowAssetType == EntryType::Pipeline)
		{
			ImGuiUtils::BeginParameters();

			if (myCurrentPipelineChoice == 0 || myCurrentPipelineChoice == 3)
			{
				ImGuiUtils::Parameter("Geometry shader", myWantGeometryStage);
			}
			else
			{
				myWantGeometryStage = false;
			}
			ImGuiUtils::Parameter("Pixel shader", myWantPixelStage);
			ImGuiUtils::EnumParameter("Pipeline Type", myCurrentPipelineChoice, {"Graphics", "PostProcess", "Decal", "Particle"});
			ImGuiUtils::EndParameters();
		}

		if (ImGui::Button("Create##ContentBrowserCreateWindowCreateButton"))
		{
			bool allowedToCreate = true;
			if (myCreateWindowAssetName == "")
			{
				ImGuiUtils::NotifyErrorLocal("The asset must have a name!");
				allowedToCreate = false;
			}
			std::filesystem::path finalPath = myCreateWindowDirectoryPath.string() + myCreateWindowAssetName + GetExtensionFromEntryType(myCreateWindowAssetType);
			if (std::filesystem::exists(finalPath))
			{
				ImGuiUtils::NotifyErrorLocal("An asset with that path and name already exists!");
				allowedToCreate = false;
			}
			if (myCreateWindowAssetType == EntryType::BlendSpace)
			{
				if (myCreateWindowBlendSpaceToCreate->myMeshPath == "")
				{
					ImGuiUtils::NotifyErrorLocal("Blendspaces must have a skeleton assigned!");
					allowedToCreate = false;
				}
			}
			else if (myCreateWindowAssetType == EntryType::AvatarMask)
			{
				if (!myCreateWindowAvatarMaskToCreate->GetAnimatedMesh())
				{
					ImGuiUtils::NotifyErrorLocal("Avatar masks must have a skeleton assigned!");
					allowedToCreate = false;
				}
			}
			if (allowedToCreate)
			{
				if (myCreateWindowAssetType == EntryType::BlendSpace)
				{
					myCreateWindowBlendSpaceToCreate->SaveTo(finalPath);
				}
				else if (myCreateWindowAssetType == EntryType::Pipeline)
				{
					auto currentPipelineChoice = static_cast<Firefly::PipelineType>(myCurrentPipelineChoice);
					Firefly::GraphicsPipelineInfo pipelineInfo{};
					pipelineInfo.GenerateHash = true;
					pipelineInfo.IsDeferred = false;
					pipelineInfo.Name = myCreateWindowAssetName;
					pipelineInfo.Path = finalPath;
					pipelineInfo.ShaderStages.clear();
					pipelineInfo.ShaderStages.emplace_back(Firefly::ShaderType::Vertex);
					pipelineInfo.PipeType = currentPipelineChoice;
					if (myWantGeometryStage)
					{
						pipelineInfo.ShaderStages.emplace_back(Firefly::ShaderType::Geometry);
					}
					if (myWantPixelStage)
					{
						pipelineInfo.ShaderStages.emplace_back(Firefly::ShaderType::Pixel);
					}

					std::filesystem::path customPipelinepath = R"(FireflyEngine\Pipelines\Custom)";
					customPipelinepath /= myCreateWindowAssetName;
					if (!std::filesystem::is_directory(customPipelinepath))
					{
						std::filesystem::create_directories(customPipelinepath);
					}

					auto vertexPath = customPipelinepath;
					auto vertexName = myCreateWindowAssetName + "_vs.hlsl";
					vertexPath /= (vertexName);
					pipelineInfo.shaderPaths.emplace_back(vertexPath.string());

					auto path = Firefly::ShaderLibrary::GetPathToTemplate(myCurrentPipelineChoice, Firefly::ShaderType::Vertex);

					std::ifstream fin(path);
					std::ofstream fout(vertexPath);
					fout << fin.rdbuf();

					if (myWantGeometryStage)
					{
						auto geoPath = customPipelinepath;
						auto geoName = myCreateWindowAssetName + "_gs.hlsl";
						geoPath /= (geoName);
						pipelineInfo.shaderPaths.emplace_back(geoPath.string());

						auto path = Firefly::ShaderLibrary::GetPathToTemplate(myCurrentPipelineChoice, Firefly::ShaderType::Geometry);

						std::ifstream fin(path);
						std::ofstream fout(geoPath);
						fout << fin.rdbuf();
					}

					if (myWantPixelStage)
					{
						auto pixelPath = customPipelinepath;
						auto pixelName = myCreateWindowAssetName + "_ps.hlsl";
						pixelPath /= (pixelName);
						pipelineInfo.shaderPaths.emplace_back(pixelPath.string());

						auto path = Firefly::ShaderLibrary::GetPathToTemplate(myCurrentPipelineChoice, Firefly::ShaderType::Pixel);

						std::ifstream fin(path);
						std::ofstream fout(pixelPath);
						fout << fin.rdbuf();
					}

					Firefly::PipelineLibrary::Add(pipelineInfo);
					Firefly::PipelineLibrary::Get(pipelineInfo.Hash).Cache(finalPath);
				}
				else if (myCreateWindowAssetType == EntryType::AvatarMask)
				{
					myCreateWindowAvatarMaskToCreate->SaveTo(finalPath);
				}
				myCreateWindowFlag = false;
			}
			RegenerateEntries(); // kinda unneccecary, should just add it without regenerating all
		}
		ImGui::SameLine();
		//cancel
		if (ImGui::Button("Cancel##ContentBrowserCreateWindowCancelButton"))
		{
			myCreateWindowFlag = false;
		}
		ImGui::EndPopup();
	}
	if (!myCreateWindowFlag)
	{
		if (myCreateWindowAssetType == EntryType::BlendSpace)
		{
			myCreateWindowBlendSpaceToCreate.reset();
		}
	}
}
