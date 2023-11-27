#pragma once
#pragma once

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"


namespace Firefly
{
	class Animation;
	class AnimatedMeshComponent;
	class AnimationPlayerComponent : public Component
	{
	public:
		AnimationPlayerComponent();
		~AnimationPlayerComponent() = default;

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		static std::string GetFactoryName() { return "AnimationPlayerComponent"; }
		static Ref<Component> Create() { return CreateRef<AnimationPlayerComponent>(); }

		void SetAnimation(std::string aMeshPath);

		void SetAnimationSpeed(const float& aScale) { myAnimationSpeed = aScale; }

		bool GetIsDonePlaying() { return myIsDonePlaying; }

		float GetCurrentAnimationDuration();
		float GetAnimationTime() { return myCurrentAnimationTime; }
		void SetLoop(bool aFlag);

	private:
		void LoadAnim();
		std::string myAnimationPath;
		float myCurrentAnimationTime;
		int myCurrentAnimationFrame = 0;;
		float myAnimationSpeed;
		bool myPlayingFlag;
		bool myIsDonePlaying;
		bool myIsLooping;

		Ref<Animation> myAnim;
		Ptr<AnimatedMeshComponent> myAnimatedMeshComp;

	};
}