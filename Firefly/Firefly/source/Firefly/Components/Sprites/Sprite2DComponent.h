#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class AppUpdateEvent;
	struct Sprite2DInfo;
	struct Sprite3DInfo;
	class Texture2D;
}

class Sprite2DComponent : public Firefly::Component
{
public:
	Sprite2DComponent();
	~Sprite2DComponent() override = default;

	void EarlyInitialize() override;
	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;

	void Expand(float aMultiplier);
	bool GetUseLocalPos() { return UseLocalPos; }

	void SetActive(bool aBool);
	Utils::Vector2f* GetUV();
	std::shared_ptr<Firefly::Sprite2DInfo> GetInfo() { return myInfo; };
	static std::string GetFactoryName() { return "Sprite2DComponent"; }
	static Ref<Component> Create() { return CreateRef<Sprite2DComponent>(); }
	void SetColor(const Utils::Vector4f& aColor);
	void SetAlpha(float aAlpha);
	Ref<Firefly::Texture2D> GetTexture();
	void UpdateInfoUVs();
	void Fade(bool aFadeIn = true, float aTime = 1.f);

private:
	std::shared_ptr<Firefly::Sprite2DInfo> myInfo;
	Utils::Vector2f mySize;
	Utils::Vector2f myExpandedSize;
	Utils::Vector4f myColor;
	Utils::Vector2f myUV[4];
	std::string mySpritePath;

	float myExpandMultiplier;
	bool myExpand = false;
	bool IsActive = false;
	bool UseLocalPos = false;
	bool myIgnoreDepth = false;
	bool myOverrideFade = false;
	bool myPermaTransparent = false;

	bool myFadeInFlag = false;
	float myMaxLerp;
	float myCurLerp;
};

inline void Sprite2DComponent::Expand(float aMultiplier)
{
	myExpand = true;
	myExpandMultiplier = aMultiplier;
}

