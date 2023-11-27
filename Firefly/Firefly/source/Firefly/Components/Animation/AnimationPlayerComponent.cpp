#include "FFpch.h"
#include "AnimationPlayerComponent.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"

#include "Firefly/Components/Mesh/AnimatedMeshComponent.h"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Animation.h"


#include <Utils/Timer.h>

namespace Firefly
{
	REGISTER_COMPONENT(AnimationPlayerComponent);
	AnimationPlayerComponent::AnimationPlayerComponent()
		: Component("AnimationPlayerComponent")
	{
		myAnimationPath = "";
		myCurrentAnimationTime = 0;
		myAnimationSpeed = 1;
		myPlayingFlag = true;
		myIsLooping = false;

		EditorVariable("Animation Path", ParameterType::File, &myAnimationPath, ".anim");
		EditorVariable("Animation Time", ParameterType::Float, &myCurrentAnimationTime);
		EditorVariable("Animation Frame", ParameterType::Int, &myCurrentAnimationFrame);
		EditorVariable("Play Animation", ParameterType::Bool, &myPlayingFlag);
	}

	void AnimationPlayerComponent::Initialize()
	{
		myAnimatedMeshComp = myEntity->GetComponent<AnimatedMeshComponent>();
		LoadAnim();
	}

	void AnimationPlayerComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
			{
				if (!myAnimatedMeshComp.lock())//TODO: make This dependant on a Component added event
				{
					myAnimatedMeshComp = myEntity->GetComponent<AnimatedMeshComponent>();
				}
				if (myAnim && myAnim->IsLoaded() && myAnimatedMeshComp.lock()->GetMesh()->IsLoaded())
				{
					if (myPlayingFlag)
					{
						myIsDonePlaying = false;
						myCurrentAnimationTime += Utils::Timer::GetDeltaTime() * myAnimationSpeed;
						myCurrentAnimationFrame = myCurrentAnimationTime * myAnim->FramesPerSecond;
					}
					if (myIsLooping)
					{
						if (myCurrentAnimationTime > myAnim->GetDuration() || myCurrentAnimationTime < 0)
						{
							myIsDonePlaying = true;
							myCurrentAnimationTime = 0;
							myCurrentAnimationFrame = myCurrentAnimationTime * myAnim->FramesPerSecond;
						}
					}
					if (myAnimatedMeshComp.lock())
					{
						auto frame = myAnim->GetFrame(myCurrentAnimationTime, true);
						auto& skeleton = myAnimatedMeshComp.lock()->GetMesh()->GetSkeleton();
						std::vector<Utils::Matrix4f> matrices;
						frame.CalculateTransforms(skeleton, matrices);

						myAnimatedMeshComp.lock()->SetCurrentMatrices(matrices);
					}
				}
				return false;
			});

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				if (e.GetParamName() == "Animation Path")
				{
					LoadAnim();
				}
				if (myAnim)
				{
					if (e.GetParamName() == "Animation Frame")
					{
						auto secondsPerFrame = 1 / myAnim->FramesPerSecond;
						myCurrentAnimationTime = myCurrentAnimationFrame * secondsPerFrame;
					}
					if (e.GetParamName() == "Animation Time")
					{
						myCurrentAnimationFrame = myCurrentAnimationTime * myAnim->FramesPerSecond;
					}
				}
				return false;
			});
	}

	void AnimationPlayerComponent::SetAnimation(std::string aMeshPath)
	{
		myAnimationPath = aMeshPath;
		LoadAnim();
	}

	float AnimationPlayerComponent::GetCurrentAnimationDuration()
	{
		return myAnim->GetDuration();
	}

	void AnimationPlayerComponent::SetLoop(bool aFlag)
	{
		myIsLooping = aFlag;
	}

	void AnimationPlayerComponent::LoadAnim()
	{
		myAnim = ResourceCache::GetAsset<Animation>(myAnimationPath);
		if (myAnimatedMeshComp.lock())
		{
			myCurrentAnimationTime = 0;
		}
	}

}