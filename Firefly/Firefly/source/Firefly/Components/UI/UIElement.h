#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class AppUpdateEvent;
}

class Sprite3DComponent;
class Sprite2DComponent;

class UIElement : public Firefly::Component
{
public:
	UIElement();
	~UIElement() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnUpdateEvent(Firefly::AppUpdateEvent& aEvent);

	bool IsInsideBounds(Utils::Vector2f aPointToCompare);

	void SetBounds(const Utils::Vector2f& aBounds);
	void SetBounds(const Utils::Vector2f& aBounds, const Utils::Vector2f& aPos);
	void SetBounds(const Vector4f& aBounds, const Utils::Vector2f& aPos);
	void SetActive(bool aBool);

	const Utils::Vector2f& GetBounds() const;
	const Utils::Vector4f& GetCustomBounds() const;
	const Utils::Vector2f& GetMouseRatio() const;
	const Utils::Vector2f& GetHoriBound() const;

	static std::string GetFactoryName() { return "UIElement"; }
	static Ref<Component> Create() { return CreateRef<UIElement>(); }

private:
	bool Is2D;
	bool IsActive;
	bool myInsideBoundsFlag3D;
	bool myUseCameraForward;
	Utils::Vector2f myBounds;
	Utils::Vector2f myPos;
	Ptr<Sprite3DComponent> mySprite3D;
	Ptr<Sprite2DComponent> mySprite2D;

	bool hasCustomBounds;
	Vector4f myCustomBounds;

	Utils::Vector2f myMouseInsideRatio;
	Utils::Vector2f myHoriBound;
};

