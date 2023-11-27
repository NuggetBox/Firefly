#pragma once
#include "Firefly/Asset/Material/MaterialAsset.h"
#include <filesystem>

namespace Firefly
{
	class MaterialImporter
	{
	public:
		Ref<MaterialAsset> ImportMaterial(const std::filesystem::path& aPath);
		bool ImportMaterial(Ref<MaterialAsset> aMat);
		static void ExportMaterial(Ref<MaterialAsset> aMaterial,  const std::filesystem::path& aPath);
		static void RefreshMaterial(Ref<MaterialAsset> aMaterial, const std::filesystem::path& aPath);
	private:
		
	};
}

