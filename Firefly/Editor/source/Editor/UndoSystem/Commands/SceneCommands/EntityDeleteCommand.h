#pragma once

#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Scene;
}
class EntityDeleteCommand : public UndoCommand
{

public:
	EntityDeleteCommand(Ptr<Firefly::Entity> aEntity);
	
	virtual void Execute() override;
	virtual void Undo() override;

private:
	Ref<Firefly::Entity> myDeleteEntity;
	Ref<Firefly::Entity> myPrevParent;
	std::vector<Ref<Firefly::Entity>> myDeletedChildren;
	std::vector<uint64_t> myDeletedChildrenParentIDs;
	Firefly::Scene* myParentScene;
	int myDeleteIndex;
};