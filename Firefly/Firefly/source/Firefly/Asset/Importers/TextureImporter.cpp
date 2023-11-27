#include "FFpch.h"
#include "TextureImporter.h"
namespace Firefly
{
	bool TextureImporter::ImportTexture(Ref<Texture2D> aTexture)
	{
		aTexture->Init(aTexture->GetPath(), true);
		return true;
	}
}