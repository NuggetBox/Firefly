#include "Trigger.h"

Trigger::Trigger(bool aValue) : myValue(aValue)
{

}

bool Trigger::Get() const
{
	const bool returnValue = myValue;
	const_cast<bool&>(myValue) = false;
	return returnValue;
}

void Trigger::Set(bool aValue)
{
	myValue = aValue;
}