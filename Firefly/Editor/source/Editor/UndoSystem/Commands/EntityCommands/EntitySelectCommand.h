#pragma once

#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	class Component;
}
class EntitySelectCommand : public UndoCommand
{
public:
	EntitySelectCommand(const Ptr<Firefly::Entity>& aTargetEntity, bool aAddSelected = false);

	void Execute() override;
	void Undo() override;

private:
	std::vector<Ptr<Firefly::Entity>> myPreviousSelected;
	Ptr<Firefly::Entity> myTargetEntity;
	bool myAddSelected = false;
};