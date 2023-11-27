#pragma once
#include "Firefly/Core/Layer/Layer.h"

namespace Firefly
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const std::string& aName = "ImGuiLayer");
		void OnAttach() override;
		void OnUpdate() override;
		void OnDetach() override;
	};
}