#include "FFpch.h"
#include "Sprite3DComponent.h"

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

REGISTER_COMPONENT(Sprite3DComponent);

Sprite3DComponent::Sprite3DComponent() : Component("Sprite3DComponent")
{
	myFadeInFlag = true;
	myMaxLerp = 0.f;
	myCurLerp = 0.f;

	myInfo = std::make_shared<Firefly::Sprite3DInfo>();
	myInfo->Color = { 0,0,0,1 };
	myInfo->Size = { 0, 0 };

	myInfo->UV[0] = { 0.f, 0.f };
	myInfo->UV[1] = { 1.f, 0.f };
	myInfo->UV[2] = { 1.f, 1.f };
	myInfo->UV[3] = { 0.f, 1.f };

	myUV[0] = { 0.f, 0.f };
	myUV[1] = { 1.f, 0.f };
	myUV[2] = { 1.f, 1.f };
	myUV[3] = { 0.f, 1.f };

	myColor = { 0,0,0,1 };
	mySize = { 0, 0 };

	mySpritePath = "Assets/Textures/ParticleStar.dds";
	myInfo->Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySpritePath);

	IsActive = true;
	isLookingAtCamera = false;
	OverridePosition = false;
	myUseWorldRotation = false;

	EditorVariable("Look at Camera", Firefly::ParameterType::Bool, &isLookingAtCamera);
	EditorVariable("Override Position", Firefly::ParameterType::Bool, &OverridePosition);
	EditorVariable("Ignore Depth", Firefly::ParameterType::Bool, &myIgnoreDepth);
	EditorVariable("Size", Firefly::ParameterType::Vec2, &mySize);
	EditorVariable("Color", Firefly::ParameterType::Color, &myColor);
	EditorVariable("Sprite Path", Firefly::ParameterType::File, &mySpritePath, ".dds");

	EditorVariable("Use World Rotation", Firefly::ParameterType::Bool, &myUseWorldRotation);

	//save UVs but dont show them
	//Hello fabian i´m going to show them so i can copy the values so i have the same rectangle for the same type of sprites
	//That way we can reuse a same size of rectangle and have more consistency
	{
		EditorVariable("UV Top Left", Firefly::ParameterType::Vec2, &myUV[0]);
		//EditorHideVariable();
		EditorVariable("UV Top Right", Firefly::ParameterType::Vec2, &myUV[1]);
		//EditorHideVariable();
		EditorVariable("UV Bottom Right", Firefly::ParameterType::Vec2, &myUV[2]);
		//EditorHideVariable();
		EditorVariable("UV Bottom Left", Firefly::ParameterType::Vec2, &myUV[3]);
		//EditorHideVariable();
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

				myInfo->Size = mySize ;
			}
		}
		});

}

void Sprite3DComponent::Initialize()
{
	Component::Initialize();
	myInfo->Size = mySize;
	myInfo->Color = myColor;
	myInfo->Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySpritePath);

	myInfo->UV[0] = myUV[0];
	myInfo->UV[1] = myUV[1];
	myInfo->UV[2] = myUV[2];
	myInfo->UV[3] = myUV[3];
	myInfo->IgnoreDepth = myIgnoreDepth;
	myMaxLerp = 1.f;
	myCurLerp = myMaxLerp;
}

void Sprite3DComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& aEvent)
		{
			auto viewSize = Utils::InputHandler::GetWindowSize();
			if (!OverridePosition)
				myInfo->Position = myEntity->GetTransform().GetPosition();

			if (isLookingAtCamera)
			{
				myLookingAtCameraTransform = Utils::Transform();
				myLookingAtCameraTransform.SetPosition(myEntity->GetTransform().GetPosition());
				auto cam = Firefly::Renderer::GetActiveCamera();
				auto camBackwards = cam->GetTransform().GetBackward();
				auto noYFarCameraPos = Utils::Vector3f(camBackwards.x, 0, camBackwards.z);
				myLookingAtCameraTransform.LookAt(cam->GetTransform().GetPosition() +
					camBackwards * 10000.f);
				myInfo->Rotation = myLookingAtCameraTransform.GetQuaternion();

			}
			else
			{
				if (!myUseWorldRotation)
					myInfo->Rotation = myEntity->GetTransform().GetLocalQuaternion();
				else
					myInfo->Rotation = myEntity->GetTransform().GetQuaternion();
			}

			if (myCurLerp < myMaxLerp)
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
			if (!IsActive)
				return false;
			Firefly::Renderer::Submit(*myInfo);
			return false;
		});

	dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](Firefly::EntityPropertyUpdatedEvent& aEvent)
		{
			myInfo->Size = mySize;
			myInfo->Color = myColor;
			myInfo->IgnoreDepth = myIgnoreDepth;
			myInfo->Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySpritePath);

			UpdateInfoUVs();

			return false;
		});
}

const Utils::Vector2f& Sprite3DComponent::GetSize() const
{
	return mySize;
}

void Sprite3DComponent::SetActive(bool aBool)
{
	IsActive = aBool;
}

void Sprite3DComponent::SetOverridePosition(bool aBool)
{
	OverridePosition = aBool;
}

Utils::Transform& Sprite3DComponent::GetLookingAtCameraTransform()
{
	return myLookingAtCameraTransform;
}

void Sprite3DComponent::Fade(bool aFadeIn, float aTime)
{
	myFadeInFlag = aFadeIn;
	myMaxLerp = aTime;
	myCurLerp = 0;
}

bool Sprite3DComponent::HasFinishedFade() const
{
	return myCurLerp > myMaxLerp;
}

void Sprite3DComponent::SetSize(Utils::Vector2f aSize)
{
	mySize = aSize;
	myInfo->Size = mySize;
}

void Sprite3DComponent::SetColor(Utils::Vector4f aColor)
{
	myColor = aColor;
	myInfo->Color = myColor;
}

Utils::Vector2f* Sprite3DComponent::GetUV()
{
	return myUV;
}

Ref<Firefly::Texture2D> Sprite3DComponent::GetTexture()
{
	return myInfo->Texture;
}

void Sprite3DComponent::UpdateInfoUVs()
{
	myInfo->UV[0] = myUV[0];
	myInfo->UV[1] = myUV[1];
	myInfo->UV[2] = myUV[2];
	myInfo->UV[3] = myUV[3];
}
