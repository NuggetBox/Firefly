#include "FFpch.h"
#include "UISlidingBar.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Components/Physics/BoxColliderComponent.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Components/Sprites/Sprite3DComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Utils/InputHandler.h"
#include "Utils/Timer.h"

REGISTER_COMPONENT(UISlidingBar);

UISlidingBar::UISlidingBar() : Component("UISlidingBar")
{
	isLate = true;
	myCurrentValue = nullptr;
	myMaxValue = nullptr;
	myIsClickable = false;
	isVertical = false;
	is3D = false;
	isLerping = false;
	isHideOnStart = false;
	myCurrentRatio = 0;
	myTargetRatio = 0;

	EditorVariable("Late Update", Firefly::ParameterType::Bool, &isLate);
	EditorVariable("Clickable", Firefly::ParameterType::Bool, &myIsClickable);
	EditorVariable("Vertical", Firefly::ParameterType::Bool, &isVertical);
	EditorVariable("3D", Firefly::ParameterType::Bool, &is3D);
	EditorVariable("Lerping", Firefly::ParameterType::Bool, &isLerping);
	EditorVariable("Hide On start", Firefly::ParameterType::Bool, &isHideOnStart);
	EditorVariable("Frame", Firefly::ParameterType::Entity, &myFrame);
}

void UISlidingBar::Initialize()
{
	Component::Initialize();
	myClicked = false;
	myElement = myEntity->GetComponent<UIElement>();

	if (is3D)
	{
		//if (myCurrentValue != nullptr && myMaxValue != nullptr)
		//{
		//	auto ratio = *myCurrentValue / *myMaxValue;
		//	Calculate3D(ratio);

		//	auto weakSprite = myEntity->GetComponent<Sprite3DComponent>();
		//	if (!weakSprite.expired() && weakSprite.lock()->IsLookingAtCamera())
		//	{
		//		auto sprite = weakSprite.lock();
		//		auto trans = sprite->GetLookingAtCameraTransform();
		//		auto info = weakSprite.lock()->GetInfo();
		//		info->Position = trans.GetPosition();
		//		info->Position -= (myOffset * 0.5f) * (trans.GetLeft()).GetNormalized();
		//	}
		//	else
		//	{
		//		auto info = weakSprite.lock()->GetInfo();
		//		info->Position = myEntity->GetTransform().GetPosition();
		//		info->Position += (myOffset * 0.5f) * (myEntity->GetTransform().GetLeft()).GetNormalized();
		//	}
		//}
	}
	else
	{
		if (myCurrentValue != nullptr && myMaxValue != nullptr)
		{
			myTargetRatio = (*myCurrentValue - 0) / (*myMaxValue - 0);
			Calculate(myTargetRatio);
		}
	}
}

void UISlidingBar::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	if (isLate)
	{
		dispatcher.Dispatch<Firefly::AppLateUpdateEvent>(BIND_EVENT_FN(UISlidingBar::OnUpdateEvent));
	}
	else
	{
		dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(UISlidingBar::OnUpdateEvent));
	}
}

bool UISlidingBar::OnUpdateEvent(Firefly::Event& aEvent)
{
	float dTime = Utils::Timer::GetDeltaTime();
	myElement = myEntity->GetComponent<UIElement>();

	if (myElement.lock() == nullptr)
		return false;

	if (newFrame > 0)
	{
		newFrame--;
	}

	if (is3D)
	{
		if (myIsClickable)
		{
			POINT mousePoint = { Utils::InputHandler::GetMouseRelativeXPos(), Utils::InputHandler::GetMouseRelativeYPos() };
			Utils::Vector2f mousePos = { static_cast<float>(mousePoint.x), static_cast<float>(mousePoint.y) };

			if (myElement.lock()->IsInsideBounds(mousePos) || myClicked)
			{
				float ratio = 0;
				if (Utils::InputHandler::GetLeftClickHeld() && newFrame <= 0)
				{
					ratio = Utils::Clamp(myElement.lock()->GetMouseRatio().x, 0.f, 1.f);
					Calculate3D(ratio);
					myClicked = true;
					if (myCurrentValue != nullptr && myMaxValue != nullptr)
					{
						*myCurrentValue = *myMaxValue * ratio;
					}
				}
				else
				{
					myClicked = false;
				}
			}

			if (myCurrentValue != nullptr && myMaxValue != nullptr)
			{
				auto ratio = *myCurrentValue / *myMaxValue;
				Calculate3D(ratio);
			}

			auto weakSprite = myEntity->GetComponent<Sprite3DComponent>();
			if (!weakSprite.expired() && weakSprite.lock()->IsLookingAtCamera())
			{
				auto sprite = weakSprite.lock();
				auto trans = sprite->GetLookingAtCameraTransform();
				auto info = weakSprite.lock()->GetInfo();
				info->Position = trans.GetPosition();
				info->Position -= (myOffset * 0.5f) * (trans.GetLeft()).GetNormalized();
			}
			else
			{
				auto info = weakSprite.lock()->GetInfo();
				info->Position = myEntity->GetTransform().GetPosition();
				info->Position += (myOffset * 0.5f) * (myEntity->GetTransform().GetLeft()).GetNormalized();
			}
			return false;
		}

		if (myCurrentValue != nullptr && myMaxValue != nullptr)
		{
			std::shared_ptr<Sprite3DComponent> sprite = myEntity->GetComponent<Sprite3DComponent>().lock();
			if (!sprite)
				return false;
			std::shared_ptr<Firefly::Sprite3DInfo> info = sprite->GetInfo();
			auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
			auto background = myFrame.lock()->GetComponent<Sprite3DComponent>().lock();

			if (isHideOnStart)
			{
				if (myOldValue == *myCurrentValue)
					info->Color.w = 0.f;
				else
				{
					if (background)
						background->GetInfo()->Color.w = 0.7f;

					info->Color.w = 1.f;
					isHideOnStart = false;
					info->Position.y = myEntity->GetTransform().GetYPosition();
				}
			}

			if (background && myCurrentRatio <= 0.f)
			{
				background->GetInfo()->Color.w = Utils::Lerp(background->GetInfo()->Color.w, 0.f, 5.f * dTime);
			}

			float ratio = Utils::Clamp((*myCurrentValue - 0) / (*myMaxValue - 0), 0.f, 1.f);
			myTargetRatio = (*myCurrentValue - 0) / (*myMaxValue - 0);
			if (isLerping)
				myCurrentRatio = Utils::Lerp(myCurrentRatio, myTargetRatio, 5.f * dTime);
			else
				myCurrentRatio = myTargetRatio;

			Calculate3D(myCurrentRatio);

			auto weakSprite = myEntity->GetComponent<Sprite3DComponent>();
			if (!weakSprite.expired() && weakSprite.lock()->IsLookingAtCamera())
			{
				auto sprite = weakSprite.lock();
				auto trans = sprite->GetLookingAtCameraTransform();
				auto info = weakSprite.lock()->GetInfo();
				info->Position = trans.GetPosition();
				info->Position -= (myOffset * 0.5f) * (trans.GetLeft()).GetNormalized();
			}
			else
			{
				auto info = weakSprite.lock()->GetInfo();
				info->Position = myEntity->GetTransform().GetPosition();
				info->Position += (myOffset * 0.5f) * (myEntity->GetTransform().GetLeft()).GetNormalized();
			}
		}
	}
	else
	{
		// vvv If this is like a audio slider of some sort then its going to be clickable vvv
		if (myIsClickable)
		{
			POINT mousePoint = { Utils::InputHandler::GetMouseRelativeXPos(), Utils::InputHandler::GetMouseRelativeYPos() };
			Utils::Vector2f mousePos = { static_cast<float>(mousePoint.x), static_cast<float>(mousePoint.y) };

			if (myElement.lock()->IsInsideBounds(mousePos))
			{
				float ratio = 0;
				if (Utils::InputHandler::GetLeftClickHeld() && newFrame <= 0)
				{

					if (isVertical)
					{
						ratio = myElement.lock()->GetMouseRatio().y;
						Calculate(ratio);
					}
					else
					{
						ratio = myElement.lock()->GetMouseRatio().x;
						Calculate(ratio);
					}

					if (myCurrentValue != nullptr && myMaxValue != nullptr)
					{
						*myCurrentValue = *myMaxValue * ratio;
					}
				}
			}
			return false;
		}

		if (myCurrentValue != nullptr && myMaxValue != nullptr)
		{
			myTargetRatio = (*myCurrentValue - 0) / (*myMaxValue - 0);
			if (isLerping)
			{
				if (myCurrentRatio >= 0)
					myCurrentRatio = Utils::Lerp(myCurrentRatio, myTargetRatio, 5.f * dTime);
				else
					myCurrentRatio = 0;
				Calculate(myCurrentRatio);
			}
			else
			{
				Calculate(myTargetRatio);
				//LOGINFO("Ratio: {}", myTargetRatio);
			}
		}
	}

	return false;
}

void UISlidingBar::BindValue(void* aCurrentValue, void* aMaxValue)
{
	myCurrentValue = reinterpret_cast<float*>(aCurrentValue);
	myMaxValue = reinterpret_cast<float*>(aMaxValue);
	myOldValue = *reinterpret_cast<float*>(aCurrentValue);
}

void UISlidingBar::BindValue(float& aCurrentValue, float& aMaxValue)
{
	myCurrentValue = &aCurrentValue;
	myMaxValue = &aMaxValue;
	myOldValue = *myCurrentValue;

	//LOGINFO("Audio slider bound\nMax: {}\nCurrent: {}", *myMaxValue, *myCurrentValue);
}

void UISlidingBar::Calculate(float aRatio, float aMod)
{
	std::shared_ptr<Sprite2DComponent> sprite = myEntity->GetComponent<Sprite2DComponent>().lock();
	if (!sprite)
		return;
	std::shared_ptr<Firefly::Sprite2DInfo> info = sprite->GetInfo();
	auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
	auto background = myFrame.lock()->GetComponent<Sprite2DComponent>();

	if (isVertical)
	{
		float uvRatio = Utils::Lerp(sprite->GetUV()[TopLeft].y, sprite->GetUV()[BottomLeft].y, aRatio);

		info->UV[BottomLeft].y = uvRatio;
		info->UV[BottomRight].y = uvRatio;

		float oldPos = 0;
		if (myFrame.lock())
			oldPos = myFrame.lock()->GetTransform().GetLocalYPosition();
		auto oldSize = myElement.lock()->GetBounds().y / viewSize.y;

		info->Size.y = oldSize * aRatio * aMod;
		float offset = (oldSize * aMod - info->Size.y) * 100.f;
		myEntity->GetTransform().SetLocalYPosition(oldPos - offset * 0.5f);
		myElement.lock()->SetBounds(myElement.lock()->GetCustomBounds(), Utils::Vector2f(myEntity->GetTransform().GetXPosition(), oldPos));
	}
	else
	{
		float uvRatio = Utils::Lerp(sprite->GetUV()[TopLeft].x, sprite->GetUV()[TopRight].x, aRatio);

		info->UV[TopRight].x = uvRatio;
		info->UV[BottomRight].x = uvRatio;

		float oldPos = 0;
		if (myFrame.lock())
			oldPos = myFrame.lock()->GetTransform().GetLocalXPosition();
		auto oldSize = myElement.lock()->GetBounds().x / viewSize.x;

		info->Size.x = oldSize * aRatio * aMod;
		float offset = (oldSize * aMod - info->Size.x) * 100.f;
		myEntity->GetTransform().SetLocalXPosition(oldPos - offset * 0.5f);
		myElement.lock()->SetBounds(myElement.lock()->GetCustomBounds(), Utils::Vector2f(oldPos, myEntity->GetTransform().GetYPosition()));
	}
}

void UISlidingBar::Calculate3D(float aRatio, float aMod)
{
	auto sprite = myEntity->GetComponent<Sprite3DComponent>().lock();
	auto info = sprite->GetInfo();
	float uvRatio = Utils::Lerp(sprite->GetUV()[TopLeft].x, sprite->GetUV()[TopRight].x, aRatio);

	info->UV[TopRight].x = uvRatio;
	info->UV[BottomRight].x = uvRatio;

	float oldPos = 0;
	if (myFrame.lock())
		oldPos = myFrame.lock()->GetTransform().GetPosition().x;
	auto oldSize = sprite->GetSize().x;

	info->Size.x = oldSize * aRatio;
	myOffset = (oldSize - info->Size.x);
}

float UISlidingBar::GetRatio() const
{
	return myCurrentRatio;
}

void UISlidingBar::NewFrame()
{
	LOGINFO("New Frame Slider: {}", myEntity->GetID());
	newFrame = 60;
}
