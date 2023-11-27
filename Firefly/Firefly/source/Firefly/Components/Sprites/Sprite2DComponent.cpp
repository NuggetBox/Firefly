#include "FFpch.h"
#include "Sprite2DComponent.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Components/UI/UIElement.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/UIEvents.h"
#include "Utils/InputHandler.h"
#include "Firefly/Rendering/Renderer.h"
#include "Utils/Timer.h"

REGISTER_COMPONENT(Sprite2DComponent);

Sprite2DComponent::Sprite2DComponent() : Component("Sprite2DComponent")
{
	myInfo = std::make_shared<Firefly::Sprite2DInfo>();
	myInfo->Color = { 0,0,0,1 };
	myInfo->Size = { 0.f, 0.f };

	myInfo->UV[0] = { 0.f, 0.f };
	myInfo->UV[1] = { 1.f, 0.f };
	myInfo->UV[2] = { 1.f, 1.f };
	myInfo->UV[3] = { 0.f, 1.f };

	myUV[0] = { 0.f, 0.f };
	myUV[1] = { 1.f, 0.f };
	myUV[2] = { 1.f, 1.f };
	myUV[3] = { 0.f, 1.f };

	myColor = { 0,0,0,1 };
	mySize = { 1.f, 1.f };

	IsActive = true;
	UseLocalPos = false;
	myExpand = false;
	myIgnoreDepth = false;
	myExpandMultiplier = 1.f;
	myExpandedSize = mySize;
	myOverrideFade = false;
	myPermaTransparent = false;

	myFadeInFlag = true;
	myMaxLerp = 0.f;
	myCurLerp = 0.f;

	EditorVariable("Use Local Position", Firefly::ParameterType::Bool, &UseLocalPos);
	EditorVariable("Ignore Depth", Firefly::ParameterType::Bool, &myIgnoreDepth);
	EditorVariable("Ignore Fade", Firefly::ParameterType::Bool, &myOverrideFade);
	EditorVariable("Permanent Transparent", Firefly::ParameterType::Bool, &myPermaTransparent);
	EditorVariable("Size", Firefly::ParameterType::Vec2, &mySize);
	EditorVariable("Color", Firefly::ParameterType::Color, &myColor);
	EditorVariable("Sprite Path", Firefly::ParameterType::File, &mySpritePath, ".dds");

	//save UVs but dont show them
	{
		EditorVariable("UV Top Left", Firefly::ParameterType::Vec2, &myUV[0]);
		EditorHideVariable();
		EditorVariable("UV Top Right", Firefly::ParameterType::Vec2, &myUV[1]);
		EditorHideVariable();
		EditorVariable("UV Bottom Right", Firefly::ParameterType::Vec2, &myUV[2]);
		EditorHideVariable();
		EditorVariable("UV Bottom Left", Firefly::ParameterType::Vec2, &myUV[3]);
		EditorHideVariable();
	}


	EditorButton("Set Native Size", [&]() {
		if (myInfo->Texture)
		{
			if (myInfo->Texture->IsLoaded())
			{
				const float uvWidth = myUV[2].x - myUV[0].x;
				const float uvHeight = myUV[2].y - myUV[0].y;
				const float xWidth = myInfo->Texture->GetWidth() * uvWidth;
				const float yHeight = myInfo->Texture->GetHeight() * uvHeight;


				mySize.x = xWidth / (yHeight / mySize.y);
				auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
				float ratio = viewSize.y / viewSize.x;
				myInfo->Size = mySize;
				myInfo->Size.x *= ratio;
			}
		}
		});

}

void Sprite2DComponent::EarlyInitialize()
{
	Component::Initialize();

	myInfo->UV[0] = myUV[0];
	myInfo->UV[1] = myUV[1];
	myInfo->UV[2] = myUV[2];
	myInfo->UV[3] = myUV[3];

	//if (mySize.x <= 0.f)
	//	mySize.x = 1.f;
	//if (mySize.y <= 0.f)
	//	mySize.y = 1.f;

	myInfo->Color = myColor;
	myExpandedSize = mySize;

	if (!mySpritePath.empty())
	{
		myInfo->Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySpritePath);
	}

	auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;

	const float ratio = 0.5625f; // 16:9 ratio basically 1080/1920 or 720/1280
	Utils::Vector3f pos = myEntity->GetTransform().GetLocalPosition();
	//float ratio = viewSize.y / viewSize.x;
	myInfo->Position = Utils::Vector2f(pos.x / viewSize.x * (ratio * 2), pos.y / viewSize.y);

	myInfo->Size = mySize;
	myInfo->Size.x *= ratio;

	auto element = myEntity->GetComponent<UIElement>().lock();
	if (element)
	{
		auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
		element->SetBounds(Utils::Vector2f(mySize.x * viewSize.x * ratio, mySize.y * viewSize.y));
	}
}

void Sprite2DComponent::Initialize()
{

}

void Sprite2DComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& aEvent)
		{
			auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
			Utils::Vector3f pos = myEntity->GetTransform().GetPosition() * 0.01f;
			if (UseLocalPos)
				pos = myEntity->GetTransform().GetLocalPosition() * 0.01f;
			myInfo->Position = Utils::Vector2f(pos.x, pos.y);
			myInfo->Rotation = myEntity->GetTransform().GetLocalQuaternion();
			if (myExpand)
			{
				float ratio = viewSize.y / viewSize.x;
				myExpandedSize = Utils::Vector2f::Lerp(myExpandedSize,
					mySize * myExpandMultiplier, Utils::Timer::GetUnscaledDeltaTime() * 2);
				myInfo->Size = myExpandedSize;
				myInfo->Size.x *= ratio;
			}

			if (myCurLerp < myMaxLerp && !myOverrideFade)
			{
				float targetAlpha = 0.f;
				if (myFadeInFlag)
				{
					targetAlpha = 1.f;
				}

				myCurLerp += Utils::Timer::GetUnscaledDeltaTime();
				auto curAlpha = Utils::EaseInOut(targetAlpha + (1 - targetAlpha) - targetAlpha, targetAlpha, myCurLerp / myMaxLerp);

				myInfo->Color.w = curAlpha;
			}

			return false;
		});

	dispatcher.Dispatch<Firefly::AppRenderEvent>([&](Firefly::AppRenderEvent& aEvent)
		{
			if (!IsActive || myPermaTransparent)
				return false;
			Firefly::Renderer::Submit(*myInfo);
			return false;
		});

	dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](Firefly::EntityPropertyUpdatedEvent& aEvent)
		{
			auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
			float ratio = viewSize.y / viewSize.x;
			//myInfo->position = Utils::Vector3f(myPos.x / viewSize.x, myPos.y / viewSize.y, 0);
			//myEntity->GetTransform().SetPosition(myPos.x, myPos.y, 0);

			//if (mySize.x <= 0.f)
			//	mySize.x = 1.f;
			//if (mySize.y <= 0.f)
			//	mySize.y = 1.f;

			myInfo->Size = mySize;
			myInfo->Size.x *= ratio;
			myExpandedSize = mySize;

			myInfo->Color = (myColor);
			myInfo->IgnoreDepth = myIgnoreDepth;

			if (aEvent.GetParamType() == Firefly::ParameterType::File && !mySpritePath.empty())
			{
				myInfo->Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySpritePath);
			}

			UpdateInfoUVs();

			return false;
		});
}

void Sprite2DComponent::SetActive(bool aBool)
{
	IsActive = aBool;
}

Utils::Vector2f* Sprite2DComponent::GetUV()
{
	return myUV;
}

Ref<Firefly::Texture2D> Sprite2DComponent::GetTexture()
{
	return myInfo->Texture;
}

void Sprite2DComponent::SetColor(const Utils::Vector4f& aColor)
{
	myInfo->Color = aColor;
}

void Sprite2DComponent::SetAlpha(float aAlpha)
{
	myInfo->Color.w = aAlpha;
}

void Sprite2DComponent::UpdateInfoUVs()
{
	auto viewSize = Utils::InputHandler::GetWindowSize() * 0.5f;
	float ratio = viewSize.y / viewSize.x;

	myInfo->UV[0] = myUV[0];
	myInfo->UV[1] = myUV[1];
	myInfo->UV[2] = myUV[2];
	myInfo->UV[3] = myUV[3];

	auto element = myEntity->GetComponent<UIElement>().lock();
	if (element)
	{
		element->SetBounds(Utils::Vector2f(mySize.x * viewSize.x * ratio, mySize.y * viewSize.y));
	}
}

void Sprite2DComponent::Fade(bool aFadeIn, float aTime)
{
	myFadeInFlag = aFadeIn;
	myMaxLerp = aTime;
	myCurLerp = 0;
}


