#pragma once
#include "Editor/UndoSystem/UndoCommand.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Scene;
}


class EntityReorderCommand : public UndoCommand
{

public:
	EntityReorderCommand(uint32_t indexToRemoveAt, uint32_t indexToInsertAt,
		Ptr<Firefly::Entity> aParentEntity, int aChildIndex, Firefly::Scene* aFromScene, Firefly::Scene* aToScene);
	// Inherited via UndoCommand
	virtual void Execute() override;
	virtual void Undo() override;

private:
	uint32_t myIndexToRemoveAt;
	uint32_t myIndexToInsertAt;

	Firefly::Scene* myFromScene;
	Firefly::Scene* myToScene;

	Ptr<Firefly::Entity> myParent;
	int myChildIndex;

	Ptr<Firefly::Entity> myPreviousParent;
	int myPreviousChildIndex;
	bool myInsertIsBiggerThanRemoveIndexFlag = false;




};