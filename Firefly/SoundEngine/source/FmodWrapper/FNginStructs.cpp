#include "SoundEngine.pch.h"

#include "FNginStructs.h"

#include <string>

SoundEventInstanceHandle SoundEventInstanceHandle::InvalidHandle = { "", -1 };

SoundEventInstanceHandle::SoundEventInstanceHandle(const std::string& anEvent, int aHandleId)
{
	if (aHandleId >= 0)
	{
		myInstance = aHandleId;
		myEventName = anEvent;
	}
}

const int& SoundEventInstanceHandle::GetId() const
{
	return myInstance;
}

const std::string& SoundEventInstanceHandle::GetEvent() const
{
	return myEventName;
}

bool SoundEventInstanceHandle::IsValid() const
{
	return !myEventName.empty() && myInstance >= 0;
}