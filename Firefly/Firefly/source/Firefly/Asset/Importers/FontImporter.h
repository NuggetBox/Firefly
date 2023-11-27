#pragma once
#include "Firefly/Rendering/Font/FireflyFont.h"
namespace Firefly
{
	class FontImporter
	{
	public:
		Ref<Font> ImportFont(const std::filesystem::path& aPath);
		bool ImportFont(Ref<Font> aFont);
		
	};
}
