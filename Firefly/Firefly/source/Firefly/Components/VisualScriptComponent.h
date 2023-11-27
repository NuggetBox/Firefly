#pragma once
#include <typeindex>

#include "Firefly/Asset/VisualScriptAsset.h"
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Event/EntityEvents.h"

namespace Firefly
{
	class EntityPropertyUpdatedEvent;

	class VisualScriptComponent : public Component
	{
	public:
		VisualScriptComponent();
		~VisualScriptComponent() override = default;

		void EarlyInitialize() override;
		void Initialize() override;
		void OnEvent(Event& aEvent) override;

		static std::string GetFactoryName() { return "VisualScriptComponent"; }
		static Ref<Component> Create() { return CreateRef<VisualScriptComponent>(); }

		void RunBeginPlay();

	private:
		bool OnUpdate(AppUpdateEvent& aEvent);
		bool OnFixedUpdate(AppFixedUpdateEvent& aEvent);
		bool OnPropertyUpdatedEvent(EntityPropertyUpdatedEvent& aEvent);
		bool OnVisualScriptUpdated(VisualScriptUpdatedEvent& aEvent);
		bool OnCollisionEnter(EntityOnCollisionEnterEvent& aEvent);
		bool OnCollisionExit(EntityOnCollisionExitEvent& aEvent);
		bool OnTriggerEnter(EntityOnTriggerEnterEvent& aEvent);
		bool OnTriggerExit(EntityOnTriggerExitEvent& aEvent);

		ParameterType MuninGraphFriendlyNameToParameterType(const std::string& aMuninGraphFriendlyName) const;

		void LoadScriptVariables();
		void SetScriptVariables();
		void SubscribeToEvents();

		void LoadVisualScript();

		Ref<VisualScriptAsset> myVisualScriptAsset;
		std::string myVisualScriptPath;

		std::vector<std::string> myHiddenVariableNames;
		std::vector<std::string> myHiddenVariableTypes;
		std::vector<int> myHiddenVariableData;
		std::vector<std::string> myHiddenStrings;
		std::vector<Ptr<Entity>> myHiddenEntityPtrs;
		int myEditorOnlyVariableCount;

		bool begunPlay = false;
	};
}
