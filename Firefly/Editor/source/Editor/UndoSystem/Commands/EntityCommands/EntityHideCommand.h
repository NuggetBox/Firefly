#pragma once

#include "Editor/UndoSystem/UndoHandler.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
}

class EntityHideCommand : public UndoCommand
{
public:
	EntityHideCommand(const Ptr<Firefly::Entity>& aTargetEntity, bool aHide = true);

	void Execute() override;
	void Undo() override;

private:
	Ptr<Firefly::Entity> myTargetEntity;
	bool myHide = false;
};