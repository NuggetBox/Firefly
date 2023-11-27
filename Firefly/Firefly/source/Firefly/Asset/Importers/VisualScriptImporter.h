#pragma once
#include "Firefly/Asset/VisualScriptAsset.h"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	//:skull:
	class VisualScriptImporter
	{
	public:
		bool ImportVisualScript(Ref<VisualScriptAsset> aAsset);
	};
}