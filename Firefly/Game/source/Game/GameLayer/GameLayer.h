#pragma once
#include "Firefly/Core/Layer/Layer.h"

#include "Game/GameWorld/DebugWorld.h"

namespace Firefly
{
	class GameLayer : public Layer
	{
	public:
		void OnAttach() override;
		void OnUpdate() override;
		void OnDetach() override;

		void OnEvent(Event& aEvent) override;

	private:
		DebugWorld myDebugWorld;
	};
}