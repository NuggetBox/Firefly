#include "FFpch.h"
#include "PhysicsTestComponent.h"
#include "Firefly/Physics/PhysicsImplementation.h"
#include "Firefly/Physics/PhysicsUtils.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Rendering/Renderer.h"
#include <Firefly/Components/Physics/CharacterControllerComponent.h>
#include <Utils/InputHandler.h>
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"
#include <Utils/Timer.h>
#include <Firefly/Event/EntityEvents.h>

namespace Firefly
{

	REGISTER_COMPONENT(PhysicsTestComponent);

	PhysicsTestComponent::PhysicsTestComponent() : Component("PhysicsTestComponent")
	{
	}

	void PhysicsTestComponent::Initialize()
	{
	}

	void Firefly::PhysicsTestComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppFixedUpdateEvent>(BIND_EVENT_FN(PhysicsTestComponent::OnFixedUpdateEvent));
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(PhysicsTestComponent::OnUpdateEvent));
		dispatcher.Dispatch<EntityOnTriggerEnterEvent>([](EntityOnTriggerEnterEvent& eEvent)
			{
				DeleteEntity(eEvent.GetCollidedEntity());
				return true;
			});
	}

	bool PhysicsTestComponent::OnUpdateEvent(Firefly::AppUpdateEvent& aEvent)
	{
		

		return true;
	}

	bool Firefly::PhysicsTestComponent::OnFixedUpdateEvent(Firefly::AppFixedUpdateEvent& aEvent)
	{

		if (!aEvent.GetIsInPlayMode())
		{
			return true;
		}

		auto cct = myEntity->GetComponent<CharacterControllerComponent>().lock();

		if (cct)
		{

				if (Utils::InputHandler::GetKeyHeld('W'))
				{
					cct->Move({ 10,0,0 });
				}
				if (Utils::InputHandler::GetKeyHeld('S'))
				{
					cct->Move({ -10,0,0 });
				}
				if (Utils::InputHandler::GetKeyHeld('A'))
				{
					cct->Move({ 0,0,10 });
				}
				if (Utils::InputHandler::GetKeyHeld('D'))
				{
					cct->Move({ 0,0,-10 });
				}
				if (Utils::InputHandler::GetKeyDown(VK_SPACE))
				{
					cct->Jump(2);
				}

		}
		return true;
	}



	bool Firefly::PhysicsTestComponent::OnPlayEvent(EditorPlayEvent& aEvent)
	{
		return false;
	}
}
