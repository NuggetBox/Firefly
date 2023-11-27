#include "FFpch.h"
#include "UICooldownOverlay.h"

#include "Firefly/Application/Application.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Utils/InputHandler.h"

REGISTER_COMPONENT(UICooldownOverlay);

UICooldownOverlay::UICooldownOverlay() : Component("UICooldownOverlay")
{
	myCurrentCD = nullptr;
	myMaxCD = nullptr;
}

void UICooldownOverlay::Initialize()
{
	Component::Initialize();

	std::shared_ptr<UIElement> element(myEntity->GetOrCreateComponent<UIElement>());
	myElement = element;
}

void UICooldownOverlay::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(UICooldownOverlay::OnUpdateEvent));
}

bool UICooldownOverlay::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
{
	//if (myCurrentCD != 0)
	//{
	//	// scales the rect down depending on how much current cd has counted down
	//	//sprite.uv.y = myCurrentCD / myMaxCD;
	//}
	//else
	//{
	//	//dont render?
	//}

	float delta = *myCurrentCD / *myMaxCD;

	//LOGINFO("CurrentCD: {} \n MaxCD: {} \n delta: {}", *myCurrentCD, *myMaxCD, delta);

	return false;
}

void UICooldownOverlay::BindCooldown(float& aCurrentCD, float& aMaxCD)
{
	myCurrentCD = &aCurrentCD;
	myMaxCD = &aMaxCD;
}
