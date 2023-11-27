#pragma once

class Trigger 
{
public:
	Trigger(bool aValue = true);

	bool Get() const;
	void Set(bool aValue = true);

private:
	bool myValue;
};