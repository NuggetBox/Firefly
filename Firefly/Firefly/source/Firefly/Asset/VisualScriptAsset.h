#pragma once
#include "Asset.h"
#include "Firefly/Core/Core.h"

class ScriptGraph;

namespace Firefly
{
	struct VisualScriptAsset : public Asset
	{
		AssetType GetAssetType() const override { return AssetType::VisualScript; }
		static AssetType GetStaticType() { return AssetType::VisualScript; }

		//TODO: Setup another structure for visual script asset. This should be a template with POD, and the graph should not exist here
		Ref<ScriptGraph> VisualScriptGraph;
	};
}