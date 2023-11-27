#pragma once
#include "Firefly/Event/Event.h"

class PrefabAssetOpenForEditEvent : public Firefly::Event
{
public:
	PrefabAssetOpenForEditEvent(const uint64_t& aPrefabID) { myPrefabID = aPrefabID; }
	~PrefabAssetOpenForEditEvent() {}

	uint64_t GetPrefabID() { return myPrefabID; }

	EVENT_CLASS_TYPE(PrefabAssetOpenForEdit);
	
private:
	uint64_t myPrefabID;
};

class PrefabEditorBackButtonPressedEvent : public Firefly::Event
{
public:
	PrefabEditorBackButtonPressedEvent() {}
	~PrefabEditorBackButtonPressedEvent() {}

	EVENT_CLASS_TYPE(PrefabEditorBackButtonPressed);
};

//on save editor settings
class SaveEditorSettingsEvent : public Firefly::Event
{
public:
	SaveEditorSettingsEvent(nlohmann::json& aJson) : myJson(aJson) {}
	~SaveEditorSettingsEvent() {}

	nlohmann::json& GetJson() { return myJson; }

	EVENT_CLASS_TYPE(SaveEditorSettings);

private:
	nlohmann::json& myJson;
	
};


//on load editor settings
class LoadEditorSettingsEvent : public Firefly::Event
{
public:
	LoadEditorSettingsEvent(nlohmann::json& aJson) : myJson(aJson) {}
	~LoadEditorSettingsEvent() {}

	nlohmann::json& GetJson() { return myJson; }

	EVENT_CLASS_TYPE(LoadEditorSettings);
	
private:
	nlohmann::json& myJson;
};

//called when we want to load a Scene in the editor
class EditorLoadSceneEvent : public Firefly::Event
{
public:
	EditorLoadSceneEvent(const std::filesystem::path& aPath) { myPath = aPath; }
	~EditorLoadSceneEvent() {}

	const std::filesystem::path& GetPath() const { return myPath; }

	EVENT_CLASS_TYPE(EditorLoadScene);

private:
	std::filesystem::path myPath;
};

class SelectInContentBrowserEvent : public Firefly::Event
{
public:
	SelectInContentBrowserEvent(const std::filesystem::path& aPath) { myPath = aPath; }
	~SelectInContentBrowserEvent() {}

	const std::filesystem::path& GetPath() const { return myPath; }

	EVENT_CLASS_TYPE(SelectInContentBrowser);
	
private:
	std::filesystem::path myPath;
};

class SearchInContentBrowserEvent : public Firefly::Event
{
public:
	SearchInContentBrowserEvent(const std::string& aSearchString) { mySearchString = aSearchString; }
	~SearchInContentBrowserEvent() {}

	const std::string& GetSearchString() const { return mySearchString; }

	EVENT_CLASS_TYPE(SearchInContentBrowser);
	
private:
	std::string mySearchString;
};

class EntitySelectedEvent : public Firefly::Event
{
public:
	EntitySelectedEvent() {}
	~EntitySelectedEvent() {}

	EVENT_CLASS_TYPE(EntitySelected);
};