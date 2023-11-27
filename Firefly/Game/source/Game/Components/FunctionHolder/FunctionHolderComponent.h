#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"

namespace Firefly
{
	class AppUpdateEvent;
}

class FunctionHolderComponent : public Firefly::Component
{
public:
	FunctionHolderComponent();
	FunctionHolderComponent(const std::string& aName);
	~FunctionHolderComponent() = default;
	void Initialize() override;
	virtual void CallFunction();
	virtual void CallFunction(void* aArg);

	static std::string GetFactoryName() { return "FunctionHolderComponent"; }
	static Ref<Component> Create() { return CreateRef<FunctionHolderComponent>(); }

	void OnEvent(Firefly::Event& aEvent) override;

protected:
	Ptr<Firefly::Entity> myChain;
};

