#include "FFpch.h"
#include "UIButton.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Components/Sprites/Sprite3DComponent.h"
#include "Firefly/Components/Sprites/TextComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Utils/InputHandler.h"
#include "Utils/InputMap.h"

#include "FmodWrapper/AudioManager.h"

REGISTER_COMPONENT(UIButton);

UIButton::UIButton() : Component("UIButton")
{
	my3DFlag = false;
	myEnumNames.push_back("Default");
	myEnumNames.push_back("Respawn");
	myHoverSizeMultiplier = 1.f;
	isHovering = false;
	newFrame = false;
	myHoverEffectFlag = false;
	myShouldLockInput = false;
	myType = UIButtonPressEvent::UIButtonEventType::Default;

	EditorVariable("3D", Firefly::ParameterType::Bool, &my3DFlag);
	EditorVariable("Has Hover Effect", Firefly::ParameterType::Bool, &myHoverEffectFlag);
	EditorVariable("Hover Multiplier", Firefly::ParameterType::Float, &myHoverSizeMultiplier);
	EditorVariable("Event Type", Firefly::ParameterType::Enum, &myType, myEnumNames);
	EditorVariable("Hover Ent", Firefly::ParameterType::Entity, &myHoverEnt);
	EditorVariable("Lock Input", Firefly::ParameterType::Bool, &myShouldLockInput);

}

void UIButton::Initialize()
{
	myElement = myEntity->GetComponent<UIElement>();
	newFrame = false;

	if (myHoverEnt.lock())
	{
		auto button = myHoverEnt.lock()->GetComponent<UIElement>();
		if (button.lock())
			button.lock()->SetActive(false);
	}
	anyButtonClicked = false;
}

void UIButton::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(UIButton::OnUpdateEvent));
}

bool UIButton::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
{
	if (!newFrame)
	{
		newFrame = true;
		return false;
	}

	Utils::Vector2f pos = { Utils::InputHandler::GetMouseRelativeXPos(), Utils::InputHandler::GetMouseRelativeYPos() };
	Utils::Vector2f mousePos = { static_cast<float>(pos.x), static_cast<float>(pos.y) };

	myElement = myEntity->GetComponent<UIElement>();
	if (myElement.lock())
	{
		if (myElement.lock()->IsInsideBounds(mousePos))
		{
			auto sprite = myEntity->GetComponent<Sprite2DComponent>();
			if (!isHovering)
			{
				AudioManager::PlayEventOneShot(EVENT_UI_Hover);
				if (myHoverEffectFlag)
				{
					if (myHoverEnt.lock())
					{
						auto button = myHoverEnt.lock()->GetComponent<UIElement>();
						if (button.lock())
							button.lock()->SetActive(true);
						if (!sprite.expired())
							sprite.lock()->GetInfo()->Color.w = 0;
						auto textComp = myHoverEnt.lock()->GetComponent<TextComponent>();
						if(!textComp.expired())
						{
							textComp.lock()->TypeWrite();
						}
					}
				}
				isHovering = true;
			}
			if (sprite.lock())
			{
				sprite.lock()->Expand(myHoverSizeMultiplier);
			}
			if (Utils::InputHandler::GetLeftClickDown() || InputMap::IsKeybindDown(InputMap::Keybinds::Interact))
			{
				AudioManager::PlayEventOneShot(EVENT_UI_Click);

				if(anyButtonClicked)
					return false;

				UIButtonPressEvent buttonPress(myType);

				Firefly::Application::Get().OnEvent(buttonPress);

				CallFunction();

				if (!myHoverEnt.expired())
				{
					auto hover = myHoverEnt.lock()->GetComponent<UIElement>();
					if (hover.lock())
						hover.lock()->SetActive(false);
					if (!sprite.expired())
						sprite.lock()->GetInfo()->Color.w = 1;
				}

				newFrame = false;
				if (myShouldLockInput)
					anyButtonClicked = true;

				LOGINFO("Button Pressed");
			}
		}
		else
		{
			auto sprite = myEntity->GetComponent<Sprite2DComponent>();
			if (isHovering)
			{
				if (myHoverEffectFlag)
				{
					if (myHoverEnt.lock())
					{
						auto button = myHoverEnt.lock()->GetComponent<UIElement>();
						if (button.lock())
							button.lock()->SetActive(false);
					}
					if (!sprite.expired())
						sprite.lock()->GetInfo()->Color.w = 1;
				}
				isHovering = false;
			}
			if (sprite.lock())
			{
				sprite.lock()->Expand(1.f);
			}
		}
	}

	return false;
}

void UIButton::BindFunction(std::function<void()> aFunction)
{
	myFunctionToFire = aFunction;
}

void UIButton::CallFunction()
{
	if (myFunctionToFire)
	{
		myFunctionToFire();
	}
}

void UIButton::NewFrame()
{
	newFrame = false;
}
