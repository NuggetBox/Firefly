#pragma once
#include "Firefly/Event/Event.h"
namespace Firefly
{
	class Layer
	{
	public:
		Layer(std::string aName = "Layer");
		virtual void OnAttach() {}
		virtual void OnUpdate() {}
		virtual void OnDetach() {}
		virtual void OnEvent(Event& aEvent) {}
		virtual void WindowsMessages(unsigned aMessage, unsigned long long wParam, long long lParam) {}

		inline const std::string& GetName() const { return myDebugName; }

		virtual ~Layer();
	protected:
		std::string myDebugName;
	};
}