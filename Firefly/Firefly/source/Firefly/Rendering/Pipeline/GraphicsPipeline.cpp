#include "FFpch.h"
#include "GraphicsPipeline.h"

#include "Utils/Math/Random.hpp"

#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"
#include <dpp/nlohmann/json.hpp>

using namespace nlohmann;
namespace Firefly
{
	void GraphicsPipeline::Initialize(GraphicsPipelineInfo& aPipelineInfo)
	{
		LOGINFO("Graphics Pipeline Initializing");

		myInfo = aPipelineInfo;

		if (myInfo.GenerateHash)
		{
			HashPipeline();
			aPipelineInfo.Hash = myInfo.Hash;
		}

		myHash = myInfo.Hash;

		std::vector<Ref<Shader>> shaders(myInfo.ShaderStages.size());
		for (size_t i = 0; i < myInfo.ShaderStages.size(); ++i)
		{
			auto& shader = shaders[i];
			shader = Shader::Create(myInfo.shaderPaths[i], myInfo.ShaderStages[i]);
		}
		ShaderLibrary::Add(myHash, shaders );

		LOGINFO("Graphics Pipeline Initialized");
	}
	void GraphicsPipeline::Bind(ID3D11DeviceContext* aContext)
	{
		auto& rst = GraphicsContext::GetRenderStateManager();

		ShaderLibrary::Bind(myHash, aContext);
	}
	void GraphicsPipeline::UnBind(ID3D11DeviceContext* aContext)
	{
		auto& rst = GraphicsContext::GetRenderStateManager();

		
		ShaderLibrary::Unbind(myHash, aContext);
	}
	void GraphicsPipeline::Cache(const std::filesystem::path& aFilepath)
	{
		json data;
		data["PipelineID"] = myInfo.Hash;
		bool hasVertex = std::find(myInfo.ShaderStages.begin(), myInfo.ShaderStages.end(), ShaderType::Vertex) != myInfo.ShaderStages.end();
		bool hasGeometry = std::find(myInfo.ShaderStages.begin(), myInfo.ShaderStages.end(), ShaderType::Geometry) != myInfo.ShaderStages.end();
		bool hasPixel = std::find(myInfo.ShaderStages.begin(), myInfo.ShaderStages.end(), ShaderType::Pixel) != myInfo.ShaderStages.end();
		data["Vertex"] = hasVertex;
		data["Geometry"] = hasGeometry;
		data["Pixel"] = hasPixel;
		data["VertexPath"] = myInfo.shaderPaths[0];
		data["GeometryPath"] = hasGeometry ? myInfo.shaderPaths[1] : "";
		data["PixelPath"] = hasPixel ? myInfo.shaderPaths[hasGeometry ? 2 : 1] : "";
		data["Deferred"] = myInfo.IsDeferred;
		data["PostProcess"] = myInfo.PipeType == PipelineType::Postprocess;

		std::ofstream o(aFilepath);
		o << data.dump(4);
	}
	void GraphicsPipeline::Reload()
	{
		ShaderLibrary::Recompile(myHash);
	}
	void GraphicsPipeline::HashPipeline()
	{
		size_t seed = 0;
		seed ^= std::hash<std::string_view>{}(myInfo.Name) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<BlendState>{}(myInfo.BlendMode) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<CullState>{}(myInfo.CullMode) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<DepthStencilState>{}(myInfo.DepthMode) + 0x9e3779b9 + (seed << 6) + (seed >> 2);


		for (auto & shaderPath : myInfo.shaderPaths)
		{
			seed ^= std::hash<std::string_view>{}(shaderPath) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		myInfo.Hash = seed;
	}
}