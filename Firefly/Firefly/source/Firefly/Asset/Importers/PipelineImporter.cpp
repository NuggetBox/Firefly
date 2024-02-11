#include "FFpch.h"
#include "PipelineImporter.h"
#include "nlohmann/json.hpp"
#include <Firefly/Rendering/Pipeline/GraphicsPipeline.h>
#include "Firefly/Rendering/Pipeline/PipelineLibrary.h"
using json = nlohmann::json;
namespace Firefly
{
	void PipelineImporter::Initialize()
	{
		Parse("FireflyEngine/Pipelines/Engine/DeferredPBR.ffpl", "DefaultOpaque");
		Parse("FireflyEngine/Pipelines/Engine/ForwardPBR.ffpl", "DefaultTransparent");
		Parse("FireflyEngine/Pipelines/Engine/ParticleDefault.ffpl", "DefaultParticle");
		Parse("FireflyEngine/Pipelines/Engine/BonePipeline.ffpl", "AnimationEditorBonePipeline");
		LoadCustomPipelines("Assets/");
	}
	GraphicsPipelineInfo PipelineImporter::GetInfoFromPath(const std::filesystem::path& aFilepath)
	{
		std::ifstream fin(aFilepath);
		json data = json::parse(fin);

		GraphicsPipelineInfo info{};
		std::vector<std::string> shaderPath;

		info.GenerateHash = true;
		if (data.contains("PipelineID"))
		{
			info.GenerateHash = false;
			info.Hash = data["PipelineID"];
		}

		if (data.contains("DepthState"))
		{
			std::string state = (std::string)data["DepthState"];
			if (state == "ReadWrite")
			{
				info.DepthMode = DepthStencilState::ReadWrite;
			}
			else if (state == "ReadOnly")
			{
				info.DepthMode = DepthStencilState::ReadOnly;
			}
			else if (state == "Off")
			{
				info.DepthMode = DepthStencilState::Off;
			}
			else
			{
				LOGWARNING("Depth mode could not be loaded uses default state {}", aFilepath.string());
			}
		}

		if (data.contains("CullState"))
		{
			std::string state = (std::string)data["CullState"];
			if (state == "Back")
			{
				info.CullMode = CullState::Back;
			}
			else if (state == "Front")
			{
				info.CullMode = CullState::Front;
			}
			else if (state == "None")
			{
				info.CullMode = CullState::None;
			}
			else if (state == "Wireframe")
			{
				info.CullMode = CullState::Wireframe;
			}
			else
			{
				LOGWARNING("Cull mode could not be loaded uses default state {}", aFilepath.string());
			}
		}

		if (data.contains("BlendState"))
		{
			std::string state = (std::string)data["BlendState"];
			if (state == "Opaque")
			{
				info.BlendMode = BlendState::Opaque;
			}
			else if (state == "AlphaBlend")
			{
				info.BlendMode = BlendState::Translucent;
			}
			else if (state == "Additive")
			{
				info.BlendMode = BlendState::Additive;
			}
			else
			{
				LOGWARNING("Blend mode could not be loaded uses default state {}", aFilepath.string());
			}
		}

		info.ShaderStages.clear();
		if (data.contains("Vertex"))
		{
			if ((bool)data["Vertex"])
			{
				info.ShaderStages.emplace_back(ShaderType::Vertex);
			}
		}
		if (data.contains("Geometry"))
		{
			if ((bool)data["Geometry"])
			{
				info.ShaderStages.emplace_back(ShaderType::Geometry);
			}
		}
		if (data.contains("Pixel"))
		{
			if ((bool)data["Pixel"])
			{
				info.ShaderStages.emplace_back(ShaderType::Pixel);
			}
		}

		if (data.contains("VertexPath"))
		{
			auto path = (std::string)data["VertexPath"];
			if (!path.empty())
			{
				info.shaderPaths.emplace_back(path);
			}
		}
		if (data.contains("GeometryPath"))
		{
			auto path = (std::string)data["GeometryPath"];
			if (!path.empty())
			{
				info.shaderPaths.emplace_back(path);
			}
		}
		if (data.contains("PixelPath"))
		{
			auto path = (std::string)data["PixelPath"];
			if (!path.empty())
			{
				info.shaderPaths.emplace_back(path);
			}
		}

		info.PipeType = PipelineType::Graphics;

		bool isDeferredPipeline = false;
		if (data.contains("Deferred"))
		{
			isDeferredPipeline = data["Deferred"];
		}

		if (data.contains("PostProcess"))
		{
			bool isPostProcess = data["PostProcess"];
			if (isPostProcess)
			{
				info.PipeType = PipelineType::Postprocess;
			}
		}

		info.IsDeferred = isDeferredPipeline;
		info.Path = aFilepath;
		return info;
	}
	void PipelineImporter::LoadCustomPipelines(const std::filesystem::path& aPath)
	{
		for (auto& currDirectory : std::filesystem::directory_iterator(aPath))
		{
			if (currDirectory.is_directory())
			{
				LoadCustomPipelines(currDirectory);
			}
			else
			{
				auto& file = currDirectory.path();
				if (file.extension() == ".ffpl")
				{
					auto info =	GetInfoFromPath(file);
					PipelineLibrary::Add(info);
					continue;
				}
			}
		}
	}
	void PipelineImporter::Parse(const std::filesystem::path& aFilepath, std::string_view aCustomName)
	{
		GraphicsPipelineInfo info = GetInfoFromPath(aFilepath);

		if (aCustomName.empty())
		{
			info.Path = aFilepath;
		}
		else
		{
			info.Path = std::string(aCustomName) + ".ffpl";
		}
		PipelineLibrary::Add(info);
	}
}