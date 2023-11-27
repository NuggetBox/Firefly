#pragma once
#include "Editor/UndoSystem/UndoHandler.h"

#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
}

class EntityDeselectAllCommand : public UndoCommand
{
public:
	EntityDeselectAllCommand();

	void Execute() override;
	void Undo() override;

private:
	std::vector<Ptr<Firefly::Entity>> myDeselectedEntities;
};
