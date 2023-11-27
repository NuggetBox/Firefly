#pragma once
#include "Firefly/Core/Core.h"
#include <vector>
namespace Firefly
{
	class Scene;
	class Mesh;
}

namespace Utils
{
	class Transform;
}

namespace Firefly
{
	class FBXExporter
	{
	public:
		void ExportScene(Ref<Scene> aScene, const std::filesystem::path& aExportPath);
		void ExportMeshPackage(std::vector<std::pair<Ref<Mesh>, Utils::Transform>> aMeshes, const std::filesystem::path& aExportPath);
	};
}
