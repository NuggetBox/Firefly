#pragma once
#include "Firefly/Core/DXHelpers.h"


#include "Firefly/Rendering/Shader/Shader.h"
#include "Firefly/Rendering/RenderStateManager.h"
#include "Firefly/Rendering/Pipeline/PipelineBase.h"

namespace Firefly
{
	enum class PipelineType : uint32_t
	{
		Graphics = 0, 
		Postprocess = 1,
		Decal = 2,
		Particle = 3,
	};

	struct GraphicsPipelineInfo
	{
		// list of stages in the pipeline (OBS! Needs to be in order of the shaderPaths)
		std::vector<ShaderType> ShaderStages = { ShaderType::Vertex, ShaderType::Pixel };

		// path to the hlsl file (OBS! needs to be in the order of the shaderStages)
		std::vector<std::string> shaderPaths;

		// Name of the pipeline.
		std::string_view Name = "Default";

		// Blend state for the pipeline.
		BlendState BlendMode = BlendState::Opaque;

		// Cull state for the pipeline.
		CullState CullMode = CullState::Back;

		// Depth-stencil state for the pipeline.
		DepthStencilState DepthMode = DepthStencilState::ReadWrite;
		
		// If the pipeline should be in the deferred stage.
		bool IsDeferred = true;

		PipelineType PipeType = PipelineType::Graphics;

		// If you want to generate a hash.
		bool GenerateHash = true;

		// The generated hash of the pipeline.
		size_t Hash;

		// path to the Pipeline file
		std::filesystem::path Path;

		bool HasShaderStage(ShaderType aType) const
		{
			return std::find(ShaderStages.begin(), ShaderStages.end(), aType) != ShaderStages.end();
		}
	};

	class GraphicsPipeline : public PipelineBase
	{
	public:
		// This function initializes the GraphicsPipeline object with the given pipeline info.
		void Initialize(GraphicsPipelineInfo& aPipelineInfo);

		// This function binds the pipeline, setting the appropriate render states and binding the shader.
		void Bind(ID3D11DeviceContext* aContext) final;

		// This function unbinds the pipeline, restoring the previous render states and unbinding the shader.
		void UnBind(ID3D11DeviceContext* aContext) final;
		void Cache(const std::filesystem::path& aFilepath) final;

		[[nodiscard]] FORCEINLINE GraphicsPipelineInfo GetInfo() const { return myInfo; }

		void Reload();
	private:
		void HashPipeline();
		GraphicsPipelineInfo myInfo;
	};
}
