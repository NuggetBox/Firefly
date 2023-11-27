#pragma once

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"


namespace Firefly
{
	class AnimatedMeshComponent;
	class FollowBoneComponent : public Component
	{
	public:
		FollowBoneComponent();
		~FollowBoneComponent() = default;

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		static std::string GetFactoryName() { return "FollowBoneComponent"; }
		static Ref<Component> Create() { return CreateRef<FollowBoneComponent>(); }

		void SetEntityToFollow(Ptr<Entity> aEntity) { myEntityToFollowBoneOf = aEntity; }

	private:
		Ptr<Entity> myEntityToFollowBoneOf;

		std::string myNameOfBoneToFollow;

		Utils::BasicTransform myLinkedTransform;

		Utils::Vector3f myOffsetPosition;
		Utils::Vector3f myOffsetRotation;
		Utils::Vector3f myOffsetScale;

		bool myLinkPosition = true;
		bool myLinkRotation = true;
		bool myLinkScale = false;
		bool myUseForward = false;
	};
}