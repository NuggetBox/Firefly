#pragma once
#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <windows.h>
#include <iostream>

namespace Firefly
{
	typedef UUID FireflyID;
	typedef FireflyID FFID;
	inline FireflyID CreateFireflyID()
	{
		UUID uuid;
		UuidCreate(&uuid);
		return uuid;
	}
}