#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Prefab.h"


namespace Firefly
{
	class PrefabImporter
	{
	public:
		PrefabImporter() = default;
		~PrefabImporter() = default;
		
		bool ImportPrefab(Ref<Prefab> aAsset);
		uint64_t ImportID(const std::filesystem::path& aPath);

	private:

	};
}