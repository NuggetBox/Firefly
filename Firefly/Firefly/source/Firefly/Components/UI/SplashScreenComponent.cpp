#include "FFpch.h"
#include "SplashScreenComponent.h"

#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Utils/InputHandler.h"
#include "Utils/Timer.h"
#include "../../Application/Application.h"


REGISTER_COMPONENT(SplashScreenComponent);

using namespace Firefly;
SplashScreenComponent::SplashScreenComponent() : Component("SplashScreenComponent")
{
	myTimer = 0;

	EditorVariable("TGA Logo", ParameterType::Entity, &myTGALogo);
	EditorVariable("FMOD Logo", ParameterType::Entity, &myFMODLogo);
	EditorVariable("Engine Logo", ParameterType::Entity, &myEngineLogo);
	EditorVariable("Group Logo", ParameterType::Entity, &myGroupLogo);
	EditorVariable("Amount Of Show Time", ParameterType::Float, &myAmountOfShowTime);
	EditorVariable("Transition Time", ParameterType::Float, &myTransitionTime);
}

void SplashScreenComponent::Initialize()
{
	myTimer = 0;
	mySplash = SplashScreen::Entry;
#ifdef FF_SHIPIT
	/*myFuture = std::async(std::launch::async, [this]() {
		std::vector<Ref<Firefly::Scene>> scenes;
		scenes.push_back(SceneManager::Get().GetLoadScene("Assets/Scenes/MainMenuOffice.scene"));
		scenes.push_back(SceneManager::Get().GetLoadScene("Assets/Scenes/MainMenuDocks.scene"));

		while (!myFinished)
		{

		}

		SceneManager::Get().SetCurrentScenes(scenes);
		});*/
	if (!myTGALogo.expired())
	{
		auto sprite = myTGALogo.lock()->GetComponent<Sprite2DComponent>();
		if (!sprite.expired())
		{
			myTGALogoInfo = sprite.lock()->GetInfo();
			myTGALogoInfo->Color.w = 0.f;
			sprite.lock()->SetActive(true);
		}
	}
	if (!myFMODLogo.expired())
	{
		auto sprite = myFMODLogo.lock()->GetComponent<Sprite2DComponent>();
		if (!sprite.expired())
		{
			myFMODLogoInfo = sprite.lock()->GetInfo();
			myFMODLogoInfo->Color.w = 0.f;
			sprite.lock()->SetActive(true);
		}
	}
	if (!myEngineLogo.expired())
	{
		auto sprite = myEngineLogo.lock()->GetComponent<Sprite2DComponent>();
		if (!sprite.expired())
		{
			mySoundEventLogo = AudioManager::CreateEventInstance(EVENT_Music_Logo);
			myEngineLogoInfo = sprite.lock()->GetInfo();
			myEngineLogoInfo->Color.w = 0.f;
			sprite.lock()->SetActive(true);
		}
	}
	if (!myGroupLogo.expired())
	{
		auto sprite = myGroupLogo.lock()->GetComponent<Sprite2DComponent>();
		if (!sprite.expired())
		{
			myGroupLogoInfo = sprite.lock()->GetInfo();
			myGroupLogoInfo->Color.w = 0.f;
			sprite.lock()->SetActive(true);
		}
	}
#endif
}

void SplashScreenComponent::OnEvent(Firefly::Event& aEvent)
{
	EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<EditorPlayEvent>([&](EditorPlayEvent& e)
		{
			if (!myTGALogo.expired())
			{
				auto sprite = myTGALogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myTGALogoInfo = sprite.lock()->GetInfo();
					myTGALogoInfo->Color.w = 0.f;
				}
			}
			if (!myFMODLogo.expired())
			{
				auto sprite = myFMODLogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myFMODLogoInfo = sprite.lock()->GetInfo();
					myFMODLogoInfo->Color.w = 0.f;
				}
			}
			if (!myEngineLogo.expired())
			{
				auto sprite = myEngineLogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myEngineLogoInfo = sprite.lock()->GetInfo();
					myEngineLogoInfo->Color.w = 0.f;
				}
			}
			if (!myGroupLogo.expired())
			{
				auto sprite = myGroupLogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myGroupLogoInfo = sprite.lock()->GetInfo();
					myGroupLogoInfo->Color.w = 0.f;
				}
			}
			return false;
		});
	dispatcher.Dispatch<EditorStopEvent>([&](EditorStopEvent& e)
		{
			if (!myTGALogo.expired())
			{
				auto sprite = myTGALogo.lock()->GetComponent<Sprite2DComponent>();
				if (sprite.expired())
				{
					myTGALogoInfo = sprite.lock()->GetInfo();
					myTGALogoInfo->Color.w = 0.f;
				}
			}
			if (!myFMODLogo.expired())
			{
				auto sprite = myFMODLogo.lock()->GetComponent<Sprite2DComponent>();
				if (sprite.expired())
				{
					myFMODLogoInfo = sprite.lock()->GetInfo();
					myFMODLogoInfo->Color.w = 0.f;
				}
			}
			if (!myEngineLogo.expired())
			{
				auto sprite = myEngineLogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myEngineLogoInfo = sprite.lock()->GetInfo();
					myEngineLogoInfo->Color.w = 0.f;
				}
			}
			if (!myGroupLogo.expired())
			{
				auto sprite = myGroupLogo.lock()->GetComponent<Sprite2DComponent>();
				if (!sprite.expired())
				{
					myGroupLogoInfo = sprite.lock()->GetInfo();
					myGroupLogoInfo->Color.w = 0.f;
				}
			}
			return false;
		});
	dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
		{
			if (Firefly::Application::Get().GetIsInPlayMode())
			{
				float dT = Utils::Timer::GetDeltaTime();
				myTimer += dT;
				if (!myTGALogoInfo || !myEngineLogoInfo || !myGroupLogoInfo)
					return false;
				switch (mySplash)
				{
					case SplashScreen::Entry:
					{
						if (myTimer >= myTransitionTime)
						{
							mySplash = SplashScreen::TGALogo;
							myTimer = 0;
						}
						break;
					}
					case SplashScreen::TGALogo:
					{
						SplashItUp(myTGALogoInfo, SplashScreen::FModLogo);

						break;
					}
					case SplashScreen::FModLogo:
					{
						SplashItUp(myFMODLogoInfo, SplashScreen::EngineLogo);

						break;
					}
					case SplashScreen::EngineLogo:
					{
						SplashItUp(myEngineLogoInfo, SplashScreen::GroupLogo);

						break;
					}
					case SplashScreen::GroupLogo:
					{
						SplashItUp(myGroupLogoInfo, SplashScreen::LoadScene);

						break;
					}
					case SplashScreen::LoadScene:
					{
						if (myTimer >= 0.5f)
						{
							/*myFinished = true;
							myFuture.wait();*/
							std::vector<Ref<Firefly::Scene>> scenes;
							SceneManager::Get().LoadScene("Assets/Scenes/MainMenuOffice.scene");
							SceneManager::Get().LoadSceneAdd("Assets/Scenes/MainMenuDocks.scene");

							//SceneManager::Get().SetCurrentScenes(scenes);
						}
						break;
					}
				}
			}

			return false;
		});
}

void SplashScreenComponent::SplashItUp(Ref<Firefly::Sprite2DInfo> aInfo, SplashScreen aSplashToShow)
{
	float ratio = Utils::Lerp(0.f, 1.f, myTimer / myTransitionTime);
	aInfo->Color.w = ratio;
	if (myTimer >= myTransitionTime)
		aInfo->Color.w = 1.f;

	if (aSplashToShow == SplashScreen::GroupLogo && !logoSoundPlayed)
	{

		//AudioManager::PlayEvent(mySoundEventLogo);
		logoSoundPlayed = true;
	}

	if (myTimer >= myAmountOfShowTime - myTransitionTime)
	{
		float ratio = Utils::Lerp(1.f, 0.f,
			(myTimer - myAmountOfShowTime + myTransitionTime) / myTransitionTime);
		float control = (myTimer - myAmountOfShowTime + myTransitionTime);
		float control1 = myTransitionTime;
		//LOGINFO("Ratio: {}\nControl: {}\nControl1: {}", ratio, control, control1);
		aInfo->Color.w = ratio;
	}

	if (myTimer >= myAmountOfShowTime || Utils::InputHandler::GetKeyDown(VK_ESCAPE))
	{
		if (aSplashToShow == SplashScreen::GroupLogo)
		{
			//AudioManager::StopEvent(mySoundEventLogo);
		}

		aInfo->Color.w = 0.f;
		mySplash = aSplashToShow;
		myTimer = 0;
	}
}
