#pragma once
#include <string>
#include <algorithm>
#include <cctype>

namespace Utils
{
	inline std::string ToLower(const std::string& aString)
	{
		std::string lowerCase = aString;
		std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), [](unsigned char c) { return std::tolower(c); });
		return lowerCase;
	}

}