#pragma once
#include "Event.h"

namespace Firefly
{
	class ShaderCompileErrorEvent : public Event
	{
	public:
		ShaderCompileErrorEvent(bool aDidCompile, char* aDataPtr) : myDidCompile(aDidCompile)
		{
			if (aDataPtr)
			{
				myDataPtr = new char[16384];
				strcpy_s(myDataPtr, sizeof(char) * 16384, aDataPtr);
			}
		}

		~ShaderCompileErrorEvent()
		{
			if (myDataPtr)
			{
				delete[] myDataPtr;
				myDataPtr = nullptr;
			}
		}

		bool DidCompile() { return myDidCompile; }
		char* GetErrorPtr() { return myDataPtr; }

		EVENT_CLASS_TYPE(ShaderCompileError);

	private:
		bool myDidCompile = true;
		char* myDataPtr = nullptr;

	};

	class ForceRiseEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(ForceRise);
	};
}