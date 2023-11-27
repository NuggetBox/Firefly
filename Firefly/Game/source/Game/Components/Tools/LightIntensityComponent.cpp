#include "Gamepch.h"
#include "LightIntensityComponent.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Components/Lights/PointLightComponent.h"
#include "Firefly/Components/Lights/SpotLightComponent.h"
#include "Firefly/Components/Lights/DirectionalLightComponent.h"
#include "Utils/Timer.h"


REGISTER_COMPONENT(LightIntensityComponent);

LightIntensityComponent::LightIntensityComponent()
	: Component("LightIntensityComponent")
{
	EditorVariable("Min Value", Firefly::ParameterType::Float, &myMinValue);
	EditorVariable("Max Value", Firefly::ParameterType::Float, &myMaxValue);
	EditorVariable("Max Timer", Firefly::ParameterType::Float, &myMaxTime);
}

void LightIntensityComponent::Initialize()
{
	myPointLight = myEntity->GetComponent<Firefly::PointLightComponent>();
	mySpotLight = myEntity->GetComponent<Firefly::SpotLightComponent>();
	myDirectionalLight = myEntity->GetComponent<Firefly::DirectionalLightComponent>();
}

void LightIntensityComponent::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<Firefly::AppUpdateEvent>([&](Firefly::AppUpdateEvent& e)
		{
			if (!e.GetIsInPlayMode()) 
			{
				return false;
			}

			if (myTimeDirection)
			{
				myCurrentTime -= Utils::Timer::GetDeltaTime();
			}
			else
			{
				myCurrentTime += Utils::Timer::GetDeltaTime();
			}

			const float lerpValue = Utils::Lerp(myMinValue, myMaxValue, myCurrentTime / myMaxTime);
			const float lerpValue2 = Utils::Lerp(myMinValue2, myMaxValue2, myCurrentTime / myMaxTime);
			///LOGINFO("{}", lerpValue);
			if (myCurrentTime >= myMaxTime)
			{
				myTimeDirection = !myTimeDirection;
				myCurrentTime = myMaxTime; 
			}
			else if (myCurrentTime <= 0)
			{
				myTimeDirection = !myTimeDirection;
				myCurrentTime = 0;
			}
			if (!myPointLight.expired())
			{
				myPointLight.lock()->SetIntensity(lerpValue);
				myPointLight.lock()->SetRadius(lerpValue);
			}
			if (!mySpotLight.expired())
			{
				mySpotLight.lock()->SetIntensity(lerpValue);
				mySpotLight.lock()->SetIntensity(lerpValue);
			}
			if (!myDirectionalLight.expired())
			{
				myDirectionalLight.lock()->SetIntensity(lerpValue);
			}
			return false;
		});
}