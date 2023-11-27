#pragma once
#include <filesystem>
namespace Firefly
{
	struct GraphicsPipelineInfo;

	class PipelineImporter
	{
	public:
		static void Initialize();
		static GraphicsPipelineInfo GetInfoFromPath(const std::filesystem::path& aFilepath);
	private:
		static void LoadCustomPipelines(const std::filesystem::path& aPath);
		static void Parse(const std::filesystem::path& aFilepath, std::string_view aCustomName);
	};
}
