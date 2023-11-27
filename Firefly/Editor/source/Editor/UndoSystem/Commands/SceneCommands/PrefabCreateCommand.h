#pragma once

#include "Editor/UndoSystem/UndoHandler.h"

#include <Firefly/Asset/Prefab.h>

namespace Firefly
{
	class Entity;
	class Scene;
}

class PrefabCreateCommand : public UndoCommand
{
public:
	PrefabCreateCommand(Ref<Firefly::Prefab> aPrefab, Ptr<Firefly::Entity> aParentEntity, Firefly::Scene* aScene, bool aSelectOnSpawn = true);

	Ptr<Firefly::Entity> GetCreatedEntity() const { return myCreatedEntity; }

	void Execute() override;
	void Undo() override;

private:
	Ref<Firefly::Prefab> myPrefab;
	Ptr<Firefly::Entity> myStartParent;
	Firefly::Scene* myScene;

	std::vector<Ref<Firefly::Entity>> myDuplicatedChildren;// needs to be a ref because it should save even when the entity is deleted
	std::vector<uint64_t> myDuplicatedChildrenParentIDs;

	Ref<Firefly::Entity> myCreatedEntity; // needs to be a ref because it should save even when the entity is deleted
	bool mySelectOnSpawn;
};