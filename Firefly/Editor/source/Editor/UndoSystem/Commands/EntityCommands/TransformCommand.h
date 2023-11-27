#pragma once
#include "Editor/UndoSystem/UndoCommand.h"
#include "Firefly/ComponentSystem/Entity.h"

class TransformCommand : public UndoCommand
{
public:
	//Creates a Transform Command with the Change in position, raw data change of the quaternion, and the multiplied scalediff
	TransformCommand(const Ptr<Firefly::Entity>& aEntity, 
		const Utils::Vector3f& aPosDiff, const Utils::Quaternion& aQuaternionRotDiff, const Utils::Vector3f& aScaleDiff);

	//Creates a Transform Command with the Change in position, Change in euler angles, and the multiplied scalediff
	TransformCommand(const Ptr<Firefly::Entity>& aEntity,
		const Utils::Vector3f& aPosDiff, const Utils::Vector3f& someEulerAnglesDiff, const Utils::Vector3f& aScaleDiff);

	void Execute() override;
	void Undo() override;

	void UpdateModification();

private:
	Ptr<Firefly::Entity> myEntity;

	Utils::Vector3f myPosDiff;
	Utils::Quaternion myRotDiff;
	Utils::Vector3f myScaleDiff; //Multiplication
};