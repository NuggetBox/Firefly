#pragma once
#include "UIButtonEventEnums.h"
#include "Firefly/Components/UI/UIElement.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/UIEvents.h"

namespace Firefly
{
	class AppUpdateEvent;
}

class UIButton : public Firefly::Component
{
public:
	UIButton();
	~UIButton() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

	void BindFunction(std::function<void()> aFunction);
	void CallFunction();
	void NewFrame();

	static std::string GetFactoryName() { return "UIButton"; }
	static Ref<Component> Create() { return CreateRef<UIButton>(); }

private:
	Ptr<UIElement> myElement;
	std::function<void()> myFunctionToFire;

	std::vector<std::string> myEnumNames;
	UIButtonPressEvent::UIButtonEventType myType;
	Ptr<Firefly::Entity> myHoverEnt;

	float myHoverSizeMultiplier;
	bool my3DFlag;
	bool isHovering;
	bool myHoverEffectFlag;
	bool newFrame;
	bool myShouldLockInput;

	static inline bool anyButtonClicked = false;
};

