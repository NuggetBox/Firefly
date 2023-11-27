#pragma once
#include "Firefly/Components/UI/UIElement.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Event/ApplicationEvents.h"

class UISlidingBar : public Firefly::Component
{
public:
	enum Corners
	{
		TopLeft = 0,
		TopRight,
		BottomRight,
		BottomLeft
	};

	UISlidingBar();
	~UISlidingBar() = default;

	void Initialize() override;

	void OnEvent(Firefly::Event& aEvent) override;
	bool OnUpdateEvent(Firefly::Event& aEvent);

	/**
	 * \brief Binds current and max values to the slider and it will calculate the delta to crop the slider down to the appropriate ratio eg. enemy health
	 * \param aCurrentValue Self explainatory
	 * \param aMaxValue Self explainatory
	 */
	void BindValue(float& aCurrentValue, float& aMaxValue);
	void BindValue(void* aCurrentValue, void* aMaxValue);

	float GetRatio() const;
	void NewFrame();

	static std::string GetFactoryName() { return "UISlidingBar"; }
	static Ref<Component> Create() { return CreateRef<UISlidingBar>(); }

private:
	void Calculate(float aRatio, float aMod = 1.f);
	void Calculate3D(float aRatio, float aMod = 1.f);

	Ptr<UIElement> myElement;
	Ptr<Firefly::Entity> myFrame;
	bool isLate;
	bool is3D;
	bool myIsClickable;
	bool isVertical;
	bool isLerping;
	bool isHideOnStart;
	float myOldValue;
	float* myCurrentValue;
	float* myMaxValue;
	float myTargetRatio;
	float myCurrentRatio;
	float myOffset;
	bool myClicked;
	int newFrame;
};
