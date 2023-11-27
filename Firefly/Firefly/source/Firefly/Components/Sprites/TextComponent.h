#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Font;
	struct TextInfo;
}
class TextComponent : public Firefly::Component
{
public:
	TextComponent();
	~TextComponent() = default;

	void Initialize() override;
	void SetText(const std::string& aText);
	void SetActive(bool aBool);

	void OnEvent(Firefly::Event& aEvent) override;
	void Fade(bool aIn = true, float aTime = 1.f);
	void TypeWrite(float aTime = 1.f);

	static std::string GetFactoryName() { return "TextComponent"; }
	static Ref<Component> Create() { return CreateRef<TextComponent>(); }

private:
	void CalcSizeX();

	Ref<Firefly::Font> myFont;
	Utils::Vector4f myColor;
	Utils::Vector3f myOffset;
	std::string myFontPath;
	std::string myText;
	bool my3DFlag;
	bool myLookAtCameraFlag;
	bool myUseLocalPosFlag;
	bool IsActive;
	bool myIgnoreDepth = false;
	float myMaxLerp;
	float myCurLerp;
	float myTextWrapSize;
	float mySizeX = 0;
	bool myFadeInFlag;
	bool myCenterXFlag;
	bool myRightFlag;

	bool myTypeWriterFlag;
	std::string myCurTypeWriterText;
	float myTimeToWrite;
	float myCurTypeWriteTime;
	int myTextIndex;
};

