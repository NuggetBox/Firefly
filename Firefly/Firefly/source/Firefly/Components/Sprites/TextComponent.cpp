#include "FFpch.h"
#include "TextComponent.h"

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

using namespace Firefly;
using namespace Utils;

REGISTER_COMPONENT(TextComponent);

TextComponent::TextComponent() : Component("TextComponent")
{
	myTextWrapSize = 0;
	myFadeInFlag = true;
	myText = "";
	myColor = { 1, 1, 1, 1 };
	myFontPath = "";
	myUseLocalPosFlag = false;
	IsActive = true;
	myCenterXFlag = false;
	myRightFlag = false;
	myTypeWriterFlag = false;

	myCurTypeWriteTime = 0.f;
	myTimeToWrite = 0.f;

	EditorVariable("Is Active", ParameterType::Bool, &IsActive);
	EditorVariable("Look at Camera", ParameterType::Bool, &myLookAtCameraFlag);
	EditorVariable("Is 3D", ParameterType::Bool, &my3DFlag);
	EditorVariable("Ignore Depth", ParameterType::Bool, &myIgnoreDepth);
	EditorVariable("Use Local Pos", ParameterType::Bool, &myUseLocalPosFlag);
	EditorVariable("Center X", ParameterType::Bool, &myCenterXFlag);
	EditorVariable("Right X", ParameterType::Bool, &myRightFlag);
	EditorVariable("Offset", ParameterType::Vec3, &myOffset);
	EditorVariable("Color", ParameterType::Color, &myColor);
	EditorVariable("Font Path", ParameterType::File, &myFontPath, ".ttf");
	EditorVariable("Text", ParameterType::String, &myText);
	EditorVariable("Text Wrap Size", ParameterType::Float, &myTextWrapSize);
	EditorVariable("Type Writer", ParameterType::Bool, &myTypeWriterFlag);
}

void TextComponent::Initialize()
{
	myFont = Firefly::ResourceCache::GetAsset<Firefly::Font>(myFontPath, true);
	myCurLerp = 1.f;
	myMaxLerp = 0.f;

	CalcSizeX();
}

void TextComponent::SetText(const std::string& aText)
{
	myText = aText;
}

void TextComponent::SetActive(bool aBool)
{
	IsActive = aBool;
}

void TextComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::AppRenderEvent>([&](Firefly::AppRenderEvent& aEvent)
		{
			if (!myFont)
				myFont = Firefly::ResourceCache::GetAsset<Firefly::Font>(myFontPath, true);

			if (myFont && !myFontPath.empty())
			{
				Vector3f entPos;
				if (myUseLocalPosFlag)
				{
					entPos = myEntity->GetTransform().GetLocalPosition();
				}
				else
				{
					entPos = myEntity->GetTransform().GetPosition();
				}
				
				auto mod = 1.f;
				auto quarter = 1.f;
				if (!my3DFlag)
				{
					mod = 0.01f;
					quarter = 2.f;
				}
				Vector3f pos = entPos * mod;

				if (my3DFlag)
				{
					pos = entPos * mod + myOffset - myEntity->GetTransform().GetRight() * mySizeX * myEntity->GetTransform().GetScale().x / (2.f * quarter);
				}
				else
				{
					auto sizeMod = 50.f;
					if (myRightFlag)
						sizeMod = 22.5f;
					pos = (entPos + myOffset) * mod - Utils::Vector3f(mySizeX / sizeMod, 0, 0);
				}

				auto scale = myEntity->GetTransform().GetScale();
				Firefly::TextInfo info{};
				info.Position = pos;
				constexpr float ratio = 0.5625f; // 16:9 ratio basically 1080/1920 or 720/1280
				info.Size = { scale.x, scale.y };
				if (!my3DFlag)
					info.Size.x *= ratio;
				info.TextWrapSize = myTextWrapSize;
				info.Font = myFont;
				if (myTypeWriterFlag && Application::Get().GetIsInPlayMode())
				{
					info.Text = myCurTypeWriterText;
				}
				else
				{
					info.Text = myText;
				}
				info.Is3D = my3DFlag;
				info.IgnoreDepth = myIgnoreDepth;

				if (myLookAtCameraFlag)
				{
					Transform trans;
					trans.SetPosition(myEntity->GetTransform().GetPosition());
					auto cam = Firefly::Renderer::GetActiveCamera();
					auto camBackwards = cam->GetTransform().GetBackward();
					auto noYFarCameraPos = Utils::Vector3f(camBackwards.x, 0, camBackwards.z);
					auto desiredDir = (cam->GetTransform().GetPosition() - myEntity->GetTransform().GetPosition()).GetNormalized();
					myEntity->GetTransform().SetRotation(Utils::Quaternion::CreateLookRotation(desiredDir));
					myEntity->GetTransform().AddLocalRotation({ 0,180,0 });
					if (my3DFlag)
						info.Rotation = myEntity->GetTransform().GetQuaternion();
				}
				else
				{
					if (my3DFlag)
						info.Rotation = myEntity->GetTransform().GetQuaternion();
				}

				info.Color = myColor;
				if (IsActive)
				{
					Firefly::Renderer::Submit(info);
				}
			}
			return false;
		});
	dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
		{
			if (myCurLerp < myMaxLerp)
			{
				float targetAlpha = 0.f;
				if (myFadeInFlag)
				{
					targetAlpha = 1.f;
				}

				myCurLerp += Utils::Timer::GetUnscaledDeltaTime();
				auto curAlpha = Utils::EaseInOut(targetAlpha + (1 - targetAlpha) - targetAlpha, targetAlpha, myCurLerp / myMaxLerp);

				myColor.w = curAlpha;
			}
			else
			{
				if (myCurTypeWriteTime < myTimeToWrite && myTypeWriterFlag)
				{
					myCurTypeWriteTime += Utils::Timer::GetUnscaledDeltaTime();

					auto charPerSecond = myTimeToWrite / myText.size();

					int index = 0;

					myCurTypeWriterText = "";

					for (float i = 0.f; i < myCurTypeWriteTime && index < myText.size(); i += charPerSecond)
					{
						myCurTypeWriterText += myText[index];
						index++;
					}
				}
			}

			return false;
		});
}

void TextComponent::Fade(bool aIn, float aTime)
{
	myFadeInFlag = aIn;
	myMaxLerp = aTime;
	myCurLerp = 0;
}

void TextComponent::TypeWrite(float aTime)
{
	myTimeToWrite = aTime;
	myCurTypeWriteTime = 0.f;
	myCurTypeWriterText = "";
	myTextIndex = 0;
}

void TextComponent::CalcSizeX()
{
	if (myCenterXFlag)
	{
		//calculateTextSize
		auto fontGeo = myFont->GetFontData().FontGeometry;
		const auto& metrics = fontGeo.getMetrics();
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		for (int i = 0; i < myText.size(); i++)
		{
			const auto character = myText[i];
			const auto glyph = fontGeo.getGlyph(character);
			if (glyph)
			{
				double advance = glyph->getAdvance();
				fontGeo.getAdvance(advance, character, myText[i + 1]);
				mySizeX += advance * fsScale;
				if (myTextWrapSize > 0 && mySizeX > myTextWrapSize)
				{
					mySizeX = myTextWrapSize;
					break;
				}
			}
		}
	}
	else if (myRightFlag)
	{
		auto fontGeo = myFont->GetFontData().FontGeometry;
		const auto& metrics = fontGeo.getMetrics();
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		for (int i = 0; i < myText.size(); i++)
		{
			const auto character = myText[i];
			const auto glyph = fontGeo.getGlyph(character);
			if (glyph)
			{
				double advance = glyph->getAdvance();
				double space = 0.0;
				fontGeo.getAdvance(space, character, myText[i + 1]);
				mySizeX += (advance)+(space * (3.0 / 4.0));
				if (myTextWrapSize > 0 && mySizeX > myTextWrapSize)
				{
					mySizeX = myTextWrapSize;
					break;
				}
			}
		}
	}
}