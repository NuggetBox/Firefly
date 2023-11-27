#include "FFpch.h"
#include "FontImporter.h"
namespace Firefly
{
	Ref<Font> FontImporter::ImportFont(const std::filesystem::path& aPath)
	{
		Ref<Font> newFont = Font::Create(aPath);
		newFont->SetPath(aPath);
		return newFont;
	}
	bool FontImporter::ImportFont(Ref<Font> aFont)
	{
		aFont->Load(aFont->GetPath());
		return true;
	}
}