#pragma once
#include "Firefly/Asset/Texture/Texture2D.h"
namespace Firefly
{
	class TextureImporter
	{
	public:
		bool ImportTexture(Ref<Texture2D> aPath);
	};
}