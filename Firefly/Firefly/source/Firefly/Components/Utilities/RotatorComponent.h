#pragma once
#include "Firefly/ComponentSystem/Component.h"

namespace Firefly
{
	class RigidbodyComponent;
}

class RotatorComponent : public Firefly::Component
{
public:
	RotatorComponent();
	~RotatorComponent() = default;

	void Initialize() override;
	void OnEvent(Firefly::Event& aEvent) override;

	static std::string GetFactoryName() { return "RotatorComponent"; }
	static Ref<Firefly::Component> Create() { return CreateRef<RotatorComponent>(); }

private:
	Utils::Vec3 myRotationSpeed;
	Ptr<Firefly::RigidbodyComponent> myRigidBody;
};