#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Texture2D;
	struct BillboardInfo;
	class ParticleEmitter;
	class ParticleEmitterTemplate;

	class ParticleEmitterComponent : public Component
	{
	public:
		ParticleEmitterComponent();
		~ParticleEmitterComponent() override = default;

		void Initialize() override;
		void OnEvent(Event& aEvent) override;

		static std::string GetFactoryName() { return "ParticleEmitterComponent"; }
		static Ref<Component> Create() { return CreateRef<ParticleEmitterComponent>(); }
		void SetEmitterTemplatePath(const std::string& aPath) { myEmitterTemplatePath = aPath; }

		const Ref<ParticleEmitter>& GetEmitter() { return myParticleEmitter; }

	private:
		bool LoadAnimatedMesh();
		void Load(bool aLoadNow = false);
		void LoadDebugBillboard(bool aLoadNow = true);

		void DrawDebugBillboard();

		Ref<ParticleEmitterTemplate> myEmitterTemplate;
		Ref<ParticleEmitter> myParticleEmitter;
		std::string myEmitterTemplatePath;

		Ptr<Entity> myAnimatedMeshEntity;

		Ref<Texture2D> myDebugBillboard;
		Ref<BillboardInfo> myDebugBillboardInfo;

		bool myEmitOnStart = true;
		bool myPrewarm = false;
		bool myShouldRotate = true;
		bool myShouldScale = true;

		bool myEmitterLoaded = false;
		bool myAnimatedMeshLoaded = false;
	};
}
