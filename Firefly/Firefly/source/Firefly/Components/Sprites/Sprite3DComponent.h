#pragma once
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class AppUpdateEvent;
	struct Sprite3DInfo;
	struct Sprite2DInfo;
	class Texture2D;
}

class Sprite3DComponent : public Firefly::Component
{
public:
	Sprite3DComponent();
	~Sprite3DComponent() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;

	const Utils::Vector2f& GetSize() const;

	void SetActive(bool aBool);
	void SetOverridePosition(bool aBool);
	Utils::Transform& GetLookingAtCameraTransform();
	bool IsLookingAtCamera() const { return isLookingAtCamera; }

	void Fade(bool aFadeIn = true, float aTime = 1.f);
	bool HasFinishedFade() const;

	void SetSize(Utils::Vector2f aSize);
	void SetColor(Utils::Vector4f aColor);
	Utils::Vector2f* GetUV();

	std::shared_ptr<Firefly::Sprite3DInfo> GetInfo() { return myInfo; };
	static std::string GetFactoryName() { return "Sprite3DComponent"; }
	static Ref<Component> Create() { return CreateRef<Sprite3DComponent>(); }


	Ref<Firefly::Texture2D> GetTexture();
	void UpdateInfoUVs();

private:
	std::shared_ptr<Firefly::Sprite3DInfo> myInfo;
	Utils::Vector2f mySize;
	Utils::Vector4f myColor;
	Utils::Vector2f myUV[4];
	std::string mySpritePath;
	Utils::Transform myLookingAtCameraTransform;
	bool IsActive;
	bool OverridePosition;
	bool isLookingAtCamera;
	bool myUseWorldRotation;

	bool myIgnoreDepth = false;
	bool myFadeInFlag;
	float myMaxLerp;
	float myCurLerp;
};

