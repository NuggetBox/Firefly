 #pragma once

#include "Editor/Windows/EditorWindow.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Scene;
}

class HierarchyWindow : public EditorWindow
{
public:
	HierarchyWindow();
	virtual ~HierarchyWindow() = default;

	void OnImGui() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<HierarchyWindow>(); }
	static std::string GetFactoryName() { return "Hierarchy"; }
	std::string GetName() const override { return GetFactoryName(); }

	void ScrollToSelected();
	void OnDropEntityOnEntity(Ptr<Firefly::Entity> aDroppedEntity, Ptr<Firefly::Entity> aTargetEntity);

private:
	void DrawEntity(Ptr<Firefly::Entity> aEntity, int& aIndexInScene, int aParentIndexInScene); //int should be a reference for recursion
	bool CheckIfCanReorderEntity(Ptr<Firefly::Entity> aEntity);
	void OnDropEntityBetweenEntities(Ptr<Firefly::Entity> aDroppedEntity, int aTargetBetweenIndex);

	Ptr<Firefly::Entity> DoCreateEntity(Ptr<Firefly::Entity> aParentEntity);

	std::vector<Ptr<Firefly::Entity>> myMovingEntitiesWithMoveTo;


	bool myDragging;
	bool myEntityDeletedFlag;

	bool myExpandAll;
	bool myCollapseAll;
	bool myOpenAllParents;
	bool myAlreadyAutoScrolled;
	bool mySearchComponent = false;

	int myBetweenIndex = 0;

	struct BetweenRect
	{
		BetweenRect() = default;
		BetweenRect(ImVec2 aMin, ImVec2 aMax, int aInsertIndex, int aParentIndex, Firefly::Scene* aScene, bool aOpenFlag)
			: Min(aMin), Max(aMax), InsertIndex(aInsertIndex), Scene(aScene), OpenFlag(aOpenFlag), ParentIndex(aParentIndex)
		{
		}

		ImVec2 Min;
		ImVec2 Max;
		int InsertIndex;
		Firefly::Scene* Scene;
		int ParentIndex = -1;

		bool OpenFlag;
	};

	std::vector<BetweenRect> myBetweenRects;

	std::string mySearchString;
};