#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Utils/TimerManager.h"

class VFXComponent : public Firefly::Component
{
public:
	VFXComponent();
	~VFXComponent() override;

	void Initialize() override;

	static std::string GetFactoryName() { return "VFXComponent"; }
	static Ref<Firefly::Component> Create() { return CreateRef<VFXComponent>(); }

private:
	float myTime = 1;
	bool myShouldBlend = true;

	int myTimerID = -1;
};