#pragma once
#include <unordered_map>

namespace Firefly
{
	struct ShaderAssetInfo
	{
		std::string shaderKey;
		std::string shaderKeyAnimation;
		std::filesystem::path pixelShaderPath;
	};
	class ShaderImporter
	{
	public:
		static void Initialize();
		static ShaderAssetInfo LoadDataFromFile(std::filesystem::path aPath);
		static void UpdateFileFromData(const std::filesystem::path& aPath, const ShaderAssetInfo& aInfo);
		static void RegistryKeys(ShaderAssetInfo info);
		static void RemoveKeys(ShaderAssetInfo info);
		static std::vector<std::string> GetKeys();
		static std::unordered_map<std::string, ShaderAssetInfo>& Map() { return ShaderMap; }
	private:
		inline static std::unordered_map<std::string, ShaderAssetInfo> ShaderMap;

		static void LoadCustomShaders(const std::filesystem::path& aPath);
	};
}
