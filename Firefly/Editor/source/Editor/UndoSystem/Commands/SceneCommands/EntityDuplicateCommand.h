#pragma once
#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Scene;
}
class EntityDuplicateCommand : public UndoCommand
{
public:
	EntityDuplicateCommand(Ptr<Firefly::Entity> aEntity, bool aNewIDFlag);

	Ptr<Firefly::Entity> GetDuplicatedEntity() { return myDuplicatedEntity; }

	void Execute() override;
	void Undo() override;

private:
	std::vector<Ptr<Firefly::Entity>> myPrevSelectedEntities;
	Ptr<Firefly::Entity> myEntityToDuplicate;
	Ref<Firefly::Entity> myDuplicatedEntity;

	std::vector<Ref<Firefly::Entity>> myDuplicatedChildren;
	std::vector<uint64_t> myDuplicatedChildrenParentIDs;
	int myDuplicatedAtIndex;

	Firefly::Scene* myParentScene;

	bool myNewIDFlag;
};
