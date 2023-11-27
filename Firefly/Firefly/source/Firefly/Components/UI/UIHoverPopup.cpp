#include "FFpch.h"
#include "UIHoverPopup.h"

#include "UIButton.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Components/Sprites/Sprite3DComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Utils/InputHandler.h"
#include "Firefly/Event/EntityEvents.h"

REGISTER_COMPONENT(UIHoverPopup);

UIHoverPopup::UIHoverPopup() : Component("UIHoverPopup")
{
	myPopUpEntity = Ptr<Firefly::Entity>();

	using namespace Firefly;

	EditorVariable("In World", ParameterType::Bool, &myInWorldFlag);
	EditorVariable("PopUp Entity", ParameterType::Entity, &myPopUpEntity);
}

void UIHoverPopup::Initialize()
{
	Component::Initialize();

	std::shared_ptr<UIElement> element(myEntity->GetOrCreateComponent<UIElement>());
	myElement = element;
	myCounter = 0;
}

void UIHoverPopup::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(UIHoverPopup::OnUpdateEvent));
	dispatcher.Dispatch<Firefly::EntityOnTriggerEnterEvent>([&](Firefly::EntityOnTriggerEnterEvent& e)
		{
			auto element = myPopUpEntity;
			if (!element.expired())
			{
				//element.lock()->SetActive(true);
				auto weakSprite = element.lock()->GetComponent<Sprite3DComponent>();
				if(!weakSprite.expired())
				{
					auto sprite = weakSprite.lock();
					sprite->Fade(true, 0.2f);
				}
			}
			LOGINFO("pop in");
			return false;
		});
	dispatcher.Dispatch<Firefly::EntityOnTriggerExitEvent>([&](Firefly::EntityOnTriggerExitEvent& e)
		{
			auto element = myPopUpEntity;
			if (!element.expired())
			{
				//element.lock()->SetActive(false);
				auto weakSprite = element.lock()->GetComponent<Sprite3DComponent>();
				if (!weakSprite.expired())
				{
					auto sprite = weakSprite.lock();
					sprite->Fade(false, 0.2f);
				}
			}

			LOGINFO("Pop out");
			return false;
		});
}

bool UIHoverPopup::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
{
	Utils::Vector2f mousePos = { Utils::InputHandler::GetMouseRelativeXPos(), Utils::InputHandler::GetMouseRelativeYPos() };

	//LOGINFO("Mouse Pos: {}, {}", mousePos.x, mousePos.y);

	myElement = myEntity->GetComponent<UIElement>();
	if (myElement.lock())
	{
		if(!myInWorldFlag)
		{
			if (myElement.lock()->IsInsideBounds(mousePos))
			{
				myCounter = 0;
				if (myPopUpEntity.lock() != nullptr)
				{
					//add that the entity will set its position depending on where the mouse is relative to the screen.
					//eg. if mouse is bottom left section of the window it will set its
					//position so the bottom left corner will be at the mouse position (-half height and -half width)
					//Second thoughts after testing, maybe not, have it hard stuck to where it should be
					std::shared_ptr<UIElement> element = myPopUpEntity.lock()->GetComponent<UIElement>().lock();
					if (element)
						element->SetActive(true);
					//LOGINFO("Mouse is hovering over, popup opened!");
				}
			}
			else if (myCounter == 0)
			{
				myCounter++;
				if (myPopUpEntity.lock() != nullptr)
				{
					std::shared_ptr<UIElement> element = myPopUpEntity.lock()->GetComponent<UIElement>().lock();
					if (element)
						element->SetActive(false);
					//LOGINFO("Mouse exited the bounds, popup closed!");
				}
			}
		}
	}

	return false;
}
