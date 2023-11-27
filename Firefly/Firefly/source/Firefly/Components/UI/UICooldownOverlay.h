#pragma once
#include "Firefly/Components/UI/UIElement.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

class UICooldownOverlay : public Firefly::Component
{
public:
	UICooldownOverlay();
	~UICooldownOverlay() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

	/**
	* \brief Binds the UI CD variables to players without having to send and keep calling functions, binds once and it can keep track if CD changes
	* \param aCurrentCD What you'll be using to count down the cd
	* \param aMaxCD What the max cd is, this is also a reference incase max cd ever changes
	*/
	void BindCooldown(float& aCurrentCD, float& aMaxCD);

	static std::string GetFactoryName() { return "UICooldownOverlay"; }
	static Ref<Component> Create() { return CreateRef<UICooldownOverlay>(); }

private:

	Ptr<UIElement> myElement;
	float* myCurrentCD;
	float* myMaxCD;
};

