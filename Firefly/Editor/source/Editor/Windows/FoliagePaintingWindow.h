#pragma once
#include <Firefly/Asset/Prefab.h>
#include <Firefly/Asset/Texture/Texture2D.h>
#include <Firefly/ComponentSystem/Entity.h>

#include "EditorWindow.h"

class FoliagePaintingWindow : public EditorWindow
{
public:
	FoliagePaintingWindow();

	static std::string GetFactoryName() { return "Foliage Painting"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<FoliagePaintingWindow>(); }

	void OnEvent(Firefly::Event& aEvent) override;

protected:
	void OnImGui() override;
	void AcceptPrefabDragDrop();

private:
	std::vector<Ref<Firefly::Prefab>> myFoliagePrefabs;
	std::vector<int> mySelectedIndices;

	Ref<Firefly::Texture2D> myFoliageIcon;

	bool myPaintingActive = false;
	bool myRandomRotations = false;
};