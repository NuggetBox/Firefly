#include "EditorPch.h"
#include "AbsoluteTransformCommand.h"

AbsoluteTransformCommand::AbsoluteTransformCommand(const Ptr<Firefly::Entity>& aEntity,
	const Utils::Vector3f& aPosition, const Utils::Quaternion& aRotation, const Utils::Vector3f& aScale)
{
	myEntity = aEntity;

	myOldPosition = myEntity.lock()->GetTransform().GetLocalPosition();
	myOldRotation = myEntity.lock()->GetTransform().GetLocalQuaternion();
	myOldScale = myEntity.lock()->GetTransform().GetLocalScale();

	myNewPosition = aPosition;
	myNewRotation = aRotation;
	myNewScale = aScale;
}

void AbsoluteTransformCommand::Execute()
{
	if (myEntity.expired())
	{
		LOGERROR("TransformCommand::Execute: Entity has expired!");
		return;
	}

	const auto& entity = myEntity.lock();

	entity->GetTransform().SetLocalPosition(myNewPosition);
	entity->GetTransform().SetLocalRotation(myNewRotation);
	entity->GetTransform().SetLocalScale(myNewScale);

	UpdateModification();
}

void AbsoluteTransformCommand::Undo()
{
	if (myEntity.expired())
	{
		LOGERROR("TransformCommand::Undo: Entity has expired!");
		return;
	}

	const auto& entity = myEntity.lock();

	entity->GetTransform().SetLocalPosition(myOldPosition);
	entity->GetTransform().SetLocalRotation(myOldRotation);
	entity->GetTransform().SetLocalScale(myOldScale);

	UpdateModification();
}

void AbsoluteTransformCommand::UpdateModification()
{
	if (!myEntity.lock()->IsPrefab())
	{
		return;
	}

	constexpr float modificationEpsilon = 0.00001f;

	if ((myNewPosition - myOldPosition).LengthSqr() > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalPositionModification();
	}

	if (Utils::Abs(myNewRotation.x - myOldRotation.x) > modificationEpsilon ||
		Utils::Abs(myNewRotation.y - myOldRotation.y) > modificationEpsilon ||
		Utils::Abs(myNewRotation.z - myOldRotation.z) > modificationEpsilon ||
		Utils::Abs(myNewRotation.w - myOldRotation.w) > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalRotationModification();
	}

	if ((myNewScale - myOldScale).LengthSqr() > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalScaleModification();
	}
}