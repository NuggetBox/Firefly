#include "FFpch.h"
#include "MaterialAsset.h"
#include <Firefly/Rendering/RenderCommands.h>

#include "Firefly/Rendering/Pipeline/PipelineLibrary.h"

namespace Firefly
{
	void MaterialAsset::Init(const InternalMaterial& aInternalMaterial)
	{
		myMatInfo = aInternalMaterial;
	}
	void MaterialAsset::Bind(ID3D11DeviceContext* aContext)
	{
		globalRendererStats.MaterialBinds++;
		auto& rsm = GraphicsContext::GetRenderStateManager();
		rsm.PushBlendState(myMatInfo.BlendMode, aContext);
		rsm.PushCullState(myMatInfo.CullMode, aContext);
		rsm.PushDepthStencilState(myMatInfo.DepthMode, aContext);
		for (auto& packet : myMatInfo.Textures)
		{
			if (packet.Texture)
				packet.Texture->Bind(packet.BindPoint, packet.ShaderStage, aContext);
		}
	}

	void MaterialAsset::BindWithPipeline(ID3D11DeviceContext* aContext)
	{
		auto& pipeline = PipelineLibrary::Get(myMatInfo.PipelineID);
		pipeline.Bind(aContext);
		Bind(aContext);
	}

	void MaterialAsset::UnBind(ID3D11DeviceContext* aContext)
	{
		auto& rsm = GraphicsContext::GetRenderStateManager();
		rsm.PopBlendState(aContext);
		rsm.PopCullState(aContext);
		rsm.PopDepthStencilState(aContext);
		for (auto& packet : myMatInfo.Textures)
		{
			if (packet.Texture)
				packet.Texture->UnBind(packet.BindPoint, packet.ShaderStage, aContext);
		}
	}

	void MaterialAsset::UnbindWithPipeline(ID3D11DeviceContext* aContext)
	{
		auto& pipeline = PipelineLibrary::Get(myMatInfo.PipelineID);
		pipeline.UnBind(aContext);
		UnBind(aContext);
	}

	void MaterialAsset::AssignID(size_t aId)
	{
		myMatID = aId;
	}
	void MaterialAsset::SetShouldBlend(const bool aBlend)
	{
		myMatInfo.ShouldBlend = aBlend;
	}
}
