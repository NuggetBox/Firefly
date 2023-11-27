#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include <Firefly/Components/UI/UIElement.h>
#include "Firefly/Rendering/Renderer.h"

class UISceneLoaderConnector : public Firefly::Component
{
public:
	UISceneLoaderConnector();
	~UISceneLoaderConnector() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;

	static std::string GetFactoryName() { return "UISceneLoaderConnector"; }
	static Ref<Component> Create() { return CreateRef<UISceneLoaderConnector>(); }

private:
	Ptr<Firefly::Entity> myButtonEntity;
	std::string myPath;
};

