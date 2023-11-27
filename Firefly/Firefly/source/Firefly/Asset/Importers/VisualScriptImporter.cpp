#include "FFpch.h"
#include "VisualScriptImporter.h"
#include "MuninScriptGraph.h"
#include "nlohmann/json.hpp"

namespace Firefly
{
	bool VisualScriptImporter::ImportVisualScript(Ref<VisualScriptAsset> aAsset)
	{
		const std::filesystem::path path = aAsset->GetPath();

		std::ifstream file(path);

		if (!file.is_open())
		{
			LOGERROR("Couldn't open visual script file: {}", path.string());
			return false;
		}

		const std::string graphData = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();

		if (ScriptGraphSchema::DeserializeScriptGraph(aAsset->VisualScriptGraph, graphData))
		{
			LOGINFO("Loaded Visual Script Graph " + path.filename().string() + " successfully");
			return true;
		}
		else
		{
			LOGERROR("Failed to deserialize visual script file: {}", path.string());
			return false;
		}
	}
}