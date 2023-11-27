#include "EditorPch.h"
#include "TransformCommand.h"

TransformCommand::TransformCommand(const Ptr<Firefly::Entity>& aEntity, 
	const Utils::Vector3f& aPosDiff, const Utils::Quaternion& aRotDiff, const Utils::Vector3f& aScaleDiff)
{
	myEntity = aEntity;
	myPosDiff = aPosDiff;
	myRotDiff = aRotDiff;
	myScaleDiff = aScaleDiff;
}

TransformCommand::TransformCommand(const Ptr<Firefly::Entity>& aEntity, 
	const Utils::Vector3f& aPosDiff, const Utils::Vector3f& someEulerAnglesDiff, const Utils::Vector3f& aScaleDiff)
{
	myEntity = aEntity;
	myPosDiff = aPosDiff;
	myRotDiff = Utils::Quaternion::CreateFromEulerAngles(someEulerAnglesDiff);
	myScaleDiff = aScaleDiff;
}

void TransformCommand::Execute()
{
	if (myEntity.expired())
	{
		LOGERROR("TransformCommand::Execute: Entity has expired!");
		return;
	}
	myEntity.lock()->GetTransform().SetLocalScale(myEntity.lock()->GetTransform().GetLocalScale() * myScaleDiff);
	myEntity.lock()->GetTransform().AddLocalRotation(myRotDiff);
	myEntity.lock()->GetTransform().AddLocalPosition(myPosDiff);

	UpdateModification();
}

void TransformCommand::Undo()
{
	if (myEntity.expired())
	{
		LOGERROR("TransformCommand::Undo: Entity has expired!");
		return;
	}
	myEntity.lock()->GetTransform().AddLocalPosition(-1.0f * myPosDiff);
	myEntity.lock()->GetTransform().AddLocalRotation(myRotDiff.GetInverse());
	myEntity.lock()->GetTransform().SetLocalScale(myEntity.lock()->GetTransform().GetLocalScale() / myScaleDiff);

	UpdateModification();
}

void TransformCommand::UpdateModification()
{
	if (!myEntity.lock()->IsPrefab())
	{
		return;
	}

	constexpr float modificationEpsilon = 0.00001f;

	if (myPosDiff.LengthSqr() > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalPositionModification();
	}

	if (Utils::Abs(myRotDiff.x) > modificationEpsilon ||
		Utils::Abs(myRotDiff.y) > modificationEpsilon ||
		Utils::Abs(myRotDiff.z) > modificationEpsilon ||
		Utils::Abs(1 - myRotDiff.w) > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalRotationModification();
	}

	if ((myScaleDiff - Utils::Vector3f::One()).LengthSqr() > modificationEpsilon)
	{
		myEntity.lock()->UpdateTransformLocalScaleModification();
	}
}