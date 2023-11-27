#pragma once
#include "Firefly/Event/Event.h"

class TogglePauseEvent : public Firefly::Event
{
public:
	TogglePauseEvent(bool aAltTab = false) { myAltTab = aAltTab; }
	~TogglePauseEvent() {}

	bool GetIsAltTab() { return myAltTab; }

	EVENT_CLASS_TYPE(TogglePause);

private:
	bool myAltTab = false;
};