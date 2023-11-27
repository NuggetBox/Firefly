#pragma once
#include "Editor/UndoSystem/UndoCommand.h"
#include "Firefly/ComponentSystem/Entity.h"

class AbsoluteTransformCommand : public UndoCommand
{
public:
	//Creates a Transform Command with a position, rotation, scale to SET and RESET
	AbsoluteTransformCommand(const Ptr<Firefly::Entity>& aEntity,
		const Utils::Vector3f& aPosition, const Utils::Quaternion& aRotation, const Utils::Vector3f& aScale);

	void Execute() override;
	void Undo() override;

	void UpdateModification();

private:
	Ptr<Firefly::Entity> myEntity;

	Utils::Vector3f myOldPosition;
	Utils::Quaternion myOldRotation;
	Utils::Vector3f myOldScale;

	Utils::Vector3f myNewPosition;
	Utils::Quaternion myNewRotation;
	Utils::Vector3f myNewScale;
};