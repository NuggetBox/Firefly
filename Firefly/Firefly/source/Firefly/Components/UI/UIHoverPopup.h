#pragma once
#include "Firefly/Components/UI/UIElement.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class AppUpdateEvent;
}

class UIHoverPopup : public Firefly::Component
{
public:
	UIHoverPopup();
	~UIHoverPopup() = default;

	void Initialize() override;
	void OnEvent(Firefly::Event& aEvent) override;

	bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

	static std::string GetFactoryName() { return "UIHoverPopup"; }
	static Ref<Component> Create() { return CreateRef<UIHoverPopup>(); }

private:
	Ptr<UIElement> myElement;
	Ptr<Firefly::Entity> myPopUpEntity;
	int myCounter;
	bool myInWorldFlag;
};

