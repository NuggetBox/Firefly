#include "FFpch.h"
#include "UIElement.h"

#include "UIButton.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Utils/InputHandler.h"
#include "Firefly/Components/Sprites/Sprite3DComponent.h"
#include "Firefly/Components/Sprites/TextComponent.h"
#include "Firefly/Physics/PhysicsScene.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/Renderer.h"
#include "Utils/Math/Ray.hpp"
#include "Utils/Math/Intersection.hpp"

REGISTER_COMPONENT(UIElement);


UIElement::UIElement() : Component("UIElement")
{
	Is2D = false;
	myUseCameraForward = false;
	IsActive = true;
	hasCustomBounds = false;
	myInsideBoundsFlag3D = false;

	EditorVariable("Is 2D", Firefly::ParameterType::Bool, &Is2D);
	EditorVariable("Is Active", Firefly::ParameterType::Bool, &IsActive);
	EditorVariable("Use Camera Forward", Firefly::ParameterType::Bool, &myUseCameraForward);
	EditorVariable("Custom Boudns", Firefly::ParameterType::Bool, &hasCustomBounds);
	EditorVariable("Bounds", Firefly::ParameterType::Vec4, &myCustomBounds);

}

void UIElement::Initialize()
{
	Component::Initialize();

	auto pos = myEntity->GetTransform().GetPosition();
	myPos = Utils::Vector2f(pos.x, pos.y);
}

void UIElement::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>(BIND_EVENT_FN(UIElement::OnUpdateEvent));
	dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](Firefly::EntityPropertyUpdatedEvent& aEvent)
		{
			mySprite2D = myEntity->GetComponent<Sprite2DComponent>();
			if (mySprite2D.lock() != nullptr)
				mySprite2D.lock()->SetActive(IsActive);

			mySprite3D = myEntity->GetComponent<Sprite3DComponent>();
			if (mySprite3D.lock() != nullptr)
				mySprite3D.lock()->SetActive(IsActive);

			auto pos = myEntity->GetTransform().GetPosition();
			myPos = Utils::Vector2f(pos.x, pos.y);
			return false;
		});
}

bool UIElement::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
{
	if (!Is2D)
	{
		auto rb = myEntity->GetComponent<Firefly::RigidbodyComponent>();
		if (!rb.expired())
		{
			rb.lock()->Teleport(myEntity->GetTransform().GetPosition());
			rb.lock()->SetRotation(myEntity->GetTransform().GetQuaternion());
		}
	}
	else
	{
		auto sprite = GetComponent<Sprite2DComponent>();
		if (!sprite.expired() && sprite.lock()->GetUseLocalPos())
		{
			auto pos = myEntity->GetTransform().GetLocalPosition();
			myPos = { pos.x, pos.y };
		}
	}
	return false;
}

bool UIElement::IsInsideBounds(Utils::Vector2f aPointToCompare)
{
	if (myEntity == nullptr)
		return false;

	if (Is2D)
	{
		auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;

		if (aPointToCompare.x > viewSize.x * 2 || aPointToCompare.x < 0 ||
			aPointToCompare.y > viewSize.y * 2 || aPointToCompare.y < 0)
			return false;

		Utils::Vector2f entityPos = myPos * 0.01f;
		entityPos.x *= viewSize.x;
		entityPos.y *= viewSize.y;
		entityPos.x += viewSize.x;
		entityPos.y *= -1;
		entityPos.y += viewSize.y;

		Utils::Vector2f horiBounds = { entityPos.x - (myBounds.x * 0.5f), entityPos.x + (myBounds.x * 0.5f) };
		Utils::Vector2f vertBounds = { entityPos.y - (myBounds.y * 0.5f), entityPos.y + (myBounds.y * 0.5f) };
		myHoriBound = vertBounds;

		if (hasCustomBounds)
		{
			auto bound = myCustomBounds * 0.01f;
			horiBounds = Utils::Vector2f(entityPos.x - (bound.x * viewSize.x), entityPos.x + (bound.y * viewSize.x));
			vertBounds = Utils::Vector2f(entityPos.y - (bound.z * viewSize.y), entityPos.y + (bound.w * viewSize.y));
		}
		LOGINFO("Mouse Pos: {}, {}\nBounds: {}, {}, {}, {}", aPointToCompare.x, aPointToCompare.y, horiBounds.x, horiBounds.y, vertBounds.x, vertBounds.y);
		if (aPointToCompare.x > horiBounds.x && aPointToCompare.x < horiBounds.y &&
			aPointToCompare.y > vertBounds.x && aPointToCompare.y < vertBounds.y)
		{
			myMouseInsideRatio.x = (aPointToCompare.x - horiBounds.x) / (horiBounds.y - horiBounds.x);
			myMouseInsideRatio.y = (aPointToCompare.y - vertBounds.y) / (vertBounds.x - vertBounds.y);

			return true;
		}
	}
	else
	{
		auto windowSize = Utils::InputHandler::GetWindowSize();
		auto cam = Firefly::Renderer::GetActiveCamera();
		Utils::Vector3f dir = cam->ScreenPosToWorldDirection(aPointToCompare, windowSize);
		if (myUseCameraForward)
			dir = cam->GetTransform().GetForward();
		if (Firefly::Application::Get().GetIsInPlayMode())
		{
			auto results = Firefly::SceneManager::Get().GetPhysicsScene()->RaycastAll(
				cam->GetTransform().GetPosition(), dir, 500.f, 2);
			for (int i = 0; i < results.size(); i++)
			{
				auto ent = results[i].HitEntity;

				if (!ent.expired())
				{
					auto hitName = ent.lock()->GetName();
					auto hitID = ent.lock()->GetID();
					auto name = myEntity->GetName();
					auto ID = myEntity->GetID();
					if (i < results.size())
					{
						Firefly::Renderer::SubmitDebugSphere(results[i].Position, 1.f);
					}

					auto sprite = myEntity->GetComponent<Sprite3DComponent>();
					if (!sprite.expired())
					{
						auto left = myEntity->GetTransform().GetPosition() + myEntity->GetTransform().GetLeft() * (sprite.lock()->GetSize().x * 0.5f);
						auto right = myEntity->GetTransform().GetPosition() + myEntity->GetTransform().GetRight() * (sprite.lock()->GetSize().x * 0.5f);
						Firefly::Renderer::SubmitDebugSphere(left, 1.f);
						Firefly::Renderer::SubmitDebugSphere(right, 1.f);
						auto fullLength = (right - left).Length();
						Utils::Vector3f outPos;
						if (i < results.size())
							outPos = results[i].Position;
						auto relativeLength = (results[i].Position - left).Length();
						myMouseInsideRatio.x = relativeLength / fullLength;
					}

					if (hitID == ID)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void UIElement::SetBounds(const Utils::Vector2f& aBounds)
{
	myBounds = aBounds;
}

void UIElement::SetBounds(const Utils::Vector2f& aBounds, const Utils::Vector2f& aPos)
{
	myPos = aPos;
	myBounds = aBounds;
}

void UIElement::SetBounds(const Vector4f& aBounds, const Utils::Vector2f& aPos)
{
	myPos = aPos;
	myCustomBounds = aBounds;
}

void UIElement::SetActive(bool aBool)
{
	mySprite2D = myEntity->GetComponent<Sprite2DComponent>();
	if (mySprite2D.lock() != nullptr)
		mySprite2D.lock()->SetActive(aBool);

	mySprite3D = myEntity->GetComponent<Sprite3DComponent>();
	if (mySprite3D.lock() != nullptr)
		mySprite3D.lock()->SetActive(aBool);

	auto text = myEntity->GetComponent<TextComponent>();
	if (!text.expired())
	{
		text.lock()->SetActive(aBool);
	}

	IsActive = aBool;

	for (auto child : myEntity->GetChildren())
	{
		auto childElement = child.lock()->GetComponent<UIElement>();
		if (!childElement.expired())
		{
			childElement.lock()->SetActive(aBool);
		}
	}
}

const Utils::Vector2f& UIElement::GetBounds() const
{
	return myBounds;
}

const Utils::Vector4f& UIElement::GetCustomBounds() const
{
	return myCustomBounds;
}

const Utils::Vector2f& UIElement::GetMouseRatio() const
{
	return myMouseInsideRatio;
}

const Utils::Vector2f& UIElement::GetHoriBound() const
{
	return myHoriBound;
}
