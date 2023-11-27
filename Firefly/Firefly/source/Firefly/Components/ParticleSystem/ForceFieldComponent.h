#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/ParticleSystem/EmitterSettings.h"

namespace Firefly
{
	class ForceFieldComponent : public Component
	{
	public:
		ForceFieldComponent();

		void Initialize() override;
		void OnEvent(Event& aEvent) override;

		static std::string GetFactoryName() { return "ForceFieldComponent"; }
		static Ref<Component> Create() { return CreateRef<ForceFieldComponent>(); }

		const ForceField& GetForceField() const;

	private:
		void DrawDebugLinesAndBillboard();

		ForceField myForceField;

		Ref<Texture2D> myForceFieldBillboard;
		BillboardInfo myForceFieldBillboardInfo;
	};
}