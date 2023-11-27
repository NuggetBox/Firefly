#pragma once
#include <string>
namespace Firefly
{
	enum class EntityTag
	{
		None,
		Door,
		COUNT
	};

	inline std::string EntityTagToString(EntityTag aTag)
	{
		switch (aTag)
		{
			case EntityTag::None:
				return "None";
			case EntityTag::Door:
				return "Door";
			default:
				return "None";
		}
	}

	inline EntityTag StringToEntityTag(const std::string& aString)
	{

		if (aString == "None")
		{
			return EntityTag::None;
		}
		else if (aString == "Door")
		{
			return EntityTag::Door;
		}
		else
		{
			return EntityTag::None;
		}
	}
}