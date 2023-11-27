#pragma once
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "Firefly/Rendering/RenderCommands.h"

#include "FmodWrapper/AudioManager.h"

#include <future>

class SplashScreenComponent : public Firefly::Component
{
public:
	enum class SplashScreen
	{
		Entry,
		TGALogo,
		FModLogo,
		EngineLogo,
		GroupLogo,
		LoadScene
	};

	SplashScreenComponent();
	~SplashScreenComponent() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	void SplashItUp(Ref<Firefly::Sprite2DInfo> aInfo, SplashScreen aSplashToShow);

	static std::string GetFactoryName() { return "SplashScreenComponent"; }
	static Ref<Component> Create() { return CreateRef<SplashScreenComponent>(); }
private:
	SplashScreen mySplash;
	Ptr<Firefly::Entity> myTGALogo;
	Ptr<Firefly::Entity> myFMODLogo;
	Ptr<Firefly::Entity> myEngineLogo;
	Ptr<Firefly::Entity> myGroupLogo;
	Ref<Firefly::Sprite2DInfo> myTGALogoInfo;
	Ref<Firefly::Sprite2DInfo> myEngineLogoInfo;
	Ref<Firefly::Sprite2DInfo> myGroupLogoInfo;
	Ref<Firefly::Sprite2DInfo> myFMODLogoInfo;
	float myAmountOfShowTime;
	float myTransitionTime;
	float myTimer;
	bool myFinished = false;

	std::future<void> myFuture;


	SoundEventInstanceHandle mySoundEventLogo;
	bool logoSoundPlayed = false;
};

