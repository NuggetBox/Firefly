#include "FFpch.h"
#include "FollowBoneComponent.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "Firefly/Components/Animation/AnimatorComponent.h"


namespace Firefly
{
	REGISTER_COMPONENT(FollowBoneComponent);

	Firefly::FollowBoneComponent::FollowBoneComponent()
		: Component("FollowBoneComponent")
	{
		myOffsetPosition = { 0,0,0 };
		myOffsetRotation = { 0,0,0 };
		myOffsetScale = { 1,1,1 };
		EditorVariable("Entity to follow bone of", ParameterType::Entity, &myEntityToFollowBoneOf);
		EditorVariable("Name of bone to follow", ParameterType::String, &myNameOfBoneToFollow);
		EditorVariable("Position Offset", ParameterType::Vec3, &myOffsetPosition);
		EditorVariable("Rotation Offset", ParameterType::Vec3, &myOffsetRotation);
		EditorVariable("Scale Mod", ParameterType::Vec3, &myOffsetScale);
		
		EditorVariable("Link Position", ParameterType::Bool, &myLinkPosition);
		EditorVariable("Link Rotation", ParameterType::Bool, &myLinkRotation);
		EditorVariable("Link Scale", ParameterType::Bool, &myLinkScale);
		EditorVariable("Forward-ish", ParameterType::Bool, &myUseForward);
	}

	void Firefly::FollowBoneComponent::Initialize()
	{
		if (!myEntityToFollowBoneOf.expired())
		{
			if (!myNameOfBoneToFollow.empty())
			{
				auto ent = myEntityToFollowBoneOf.lock();
				if (ent->HasComponent<AnimatorComponent>())
				{
					auto animComp = ent->GetComponent<AnimatorComponent>().lock();
					animComp->LinkTransformToBone(myLinkedTransform, myNameOfBoneToFollow);
				}
			}
		}
	}

	void Firefly::FollowBoneComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e) -> bool
			{
				if (myEntityToFollowBoneOf.expired())
				{
					return false;
				}

				if (myLinkPosition)
				{
					Utils::Vec3 pos = myLinkedTransform.GetPosition();
					if (myUseForward)
					{
						pos += myLinkedTransform.GetForward() * myOffsetPosition;
					}
					else
					{
						pos += myOffsetPosition;
					}
					myEntity->GetTransform().SetPosition(pos);
				}
				if (myLinkRotation)
				{
					myEntity->GetTransform().SetRotation(myLinkedTransform.GetQuaternion()  * (myOffsetRotation.LengthSqr() > 0 ? Utils::Quaternion::CreateFromEulerAngles(myOffsetRotation) : Utils::Quaternion()));
				}
				if (myLinkScale)
				{
					myEntity->GetTransform().SetScale(myLinkedTransform.GetScale() * myOffsetScale);
				}
				return false;
			});
	}

}