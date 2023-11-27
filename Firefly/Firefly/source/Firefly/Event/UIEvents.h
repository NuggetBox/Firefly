#pragma once
#include "Firefly/Event/Event.h"
#include <sstream>
#include <array>

class UIButtonPressEvent : public Firefly::Event
{
public:
	enum class UIButtonEventType
	{
		Default,
		Respawn
	};
	UIButtonPressEvent(UIButtonEventType aType) : myType(aType) {}
	~UIButtonPressEvent() {}

	UIButtonEventType GetType() const { return myType; };

	EVENT_CLASS_TYPE(UIButtonPress)

private:
	UIButtonEventType myType;
};

