#pragma once
#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Scene;
}

class EntityCreateCommand : public UndoCommand
{
public:
	EntityCreateCommand(Ptr<Firefly::Entity> aParentEntity, Firefly::Scene* aScene, bool isPrefabEditorChild = false, bool aSelectOnSpawn = true);

	Ptr<Firefly::Entity> GetCreatedEntity() const { return myCreatedEntity; }

	void Execute() override;
	void Undo() override;

private:
	Firefly::Scene* myScene;
	Ref<Firefly::Entity> myCreatedEntity;
	Ptr<Firefly::Entity> myStartParent;

	bool mySelectOnSpawn;
	bool myIsPrefabEditorChild;
};