#include "FFpch.h"
#include "PostProcessRenderer.h"

#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/RenderCache.h"
#include "Firefly/Rendering/RenderCommands.h"

#include "Firefly/Rendering/Shader/ShaderLibrary.h"

#include "Firefly/Rendering/ThreadSafe/DeferredContext.h"

#include "Firefly/Rendering/Postprocess/PostProcessUtils.h"

#include "Firefly/Rendering/Pipeline/PipelineLibrary.h"

#include "Firefly/Asset/Material/MaterialAsset.h"

namespace Firefly
{
	PostProcessRenderer::PostProcessRenderer(Cache* aCache) : myCache(aCache)
	{
	}

	void PostProcessRenderer::RunPostProcessing(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		auto context = aContext.lock()->Get();

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadWrite, context);
		Renderer::CopyDepth(myCache->ForwardGBuffer, myCache->RendererFrameBuffer, context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		if (!myCache->PostProcessIsSubmitted)
		{
			myCache->PostProcessBuffData.Padding.z = 0.25;
		}
		myCache->PostProcessBuffer.SetData(&myCache->PostProcessSpecs.Data, context);
		myCache->PostProcessBuffer.Bind(7, context);
		if (!myCache->PostProcessIsSubmitted)
		{
			GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);

			BloomPass(aContext);
			TonemappingPass(aContext);

			if (myCache->DrawOutlines)
			{
				if (!myCache->OutlineCommands.empty())
				{
					ApplyOutlinePass(aContext);
				}
			}
			FXAAPass(aContext);
			DebandingPass(aContext);
			return;
		}
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);

		//if (myCache->PostProcessSpecs.passes & PostProcessPass::Fog)
		//{
		//	FogPass(aContext);
		//}

		BloomPass(aContext);

		RunCustomPasses(aContext);

		if (myCache->PostProcessSpecs.passes & PostProcessPass::Vignette)
		{
			VignettePass(aContext);
		}

		if (myCache->PostProcessSpecs.passes & PostProcessPass::ToneMapping)
		{
			TonemappingPass(aContext);
		}

		if (!myCache->OutlineCommands.empty())
		{
			ApplyOutlinePass(aContext);
		}
		FXAAPass(aContext);
		DebandingPass(aContext);
	}

	void PostProcessRenderer::VignettePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		if (!myCache->CurrentPostProcessResultFB)
		{
			return;
		}
		auto context = aContext.lock()->Get();
		myCache->CurrentPostProcessResultFB->BindSRV(1, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("Vignette", myCache->VignetteFB, context);
		myCache->CurrentPostProcessResultFB->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->CurrentPostProcessResultFB = myCache->VignetteFB;
	}
	void PostProcessRenderer::TonemappingPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		if (!myCache->CurrentPostProcessResultFB)
		{
			return;
		}
		auto context = aContext.lock()->Get();
		myCache->CurrentPostProcessResultFB->BindSRV(0, 0, ShaderType::Pixel, context);
		if (myCache->ToneMapping.ColorCorrectionLUT)
		{
			myCache->ToneMapping.ColorCorrectionLUT->Bind(1, ShaderType::Pixel, context);
		}
		PostProcessUtils::FullQuadPass("ToneMapping", myCache->ToneMappingBuffer, context);
		myCache->CurrentPostProcessResultFB->UnBindShaderResource(0, ShaderType::Pixel, context);
		if (myCache->ToneMapping.ColorCorrectionLUT)
		{
			myCache->CurrentPostProcessResultFB->UnBindShaderResource(1, ShaderType::Pixel, context);
		}
		myCache->CurrentPostProcessResultFB = myCache->ToneMappingBuffer;
	}
	void PostProcessRenderer::ApplyOutlinePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		auto context = aContext.lock()->Get();
		myCache->CurrentPostProcessResultFB->BindSRV(0, 0, ShaderType::Pixel, context);
		myCache->JFAFinal_FB->BindSRV(1, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("Outline", myCache->OutlineBufferFB, context);
		myCache->JFAFinal_FB->UnBindShaderResource(0, ShaderType::Pixel, context);
		myCache->JFAFinal_FB->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->CurrentPostProcessResultFB = myCache->OutlineBufferFB;
	}
	void PostProcessRenderer::DebandingPass(Ptr<DeferredContext> aContext)
	{
		auto context = aContext.lock()->Get();
		myCache->CurrentPostProcessResultFB->BindSRV(0, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("Debanding", myCache->RendererFrameBuffer, context);
		myCache->CurrentPostProcessResultFB->UnBindShaderResource(0, ShaderType::Pixel, context);
	}
	void PostProcessRenderer::FXAAPass(Ptr<DeferredContext> aContext)
	{
		if (!myCache->CurrentPostProcessResultFB)
		{
			return;
		}
		auto context = aContext.lock()->Get();
		myCache->CurrentPostProcessResultFB->BindSRV(1, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("FXAA", myCache->FXAAFB, context);
		myCache->CurrentPostProcessResultFB->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->CurrentPostProcessResultFB = myCache->FXAAFB;
	}
	void PostProcessRenderer::FogPass(Ptr<DeferredContext> aContext)
	{
		if (!myCache->CurrentPostProcessResultFB)
		{
			return;
		}
		auto context = aContext.lock()->Get();

		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Translucent, context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);
		ShaderLibrary::Bind("Fog", context);
		myCache->TransmittanceLUT->BindSRV(1, 0, ShaderType::Pixel, context);
		myCache->DepthPrePass->BindDepthSRV(0, ShaderType::Pixel, context);
		myCache->DeferredGBuffer->BindSRV(2, 4, ShaderType::Pixel, context);
		myCache->CurrentPostProcessResultFB->Bind(context);
		context->Draw(6, 0);
		myCache->CurrentPostProcessResultFB->UnBind(context);
		myCache->DepthPrePass->UnBindShaderResource(0, ShaderType::Pixel, context);
		myCache->DepthPrePass->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->DepthPrePass->UnBindShaderResource(2, ShaderType::Pixel, context);
		ShaderLibrary::Unbind("Fog", context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		GraphicsContext::GetRenderStateManager().PopBlendState(context);
	}
	void PostProcessRenderer::BloomPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		if (!myCache->CurrentPostProcessResultFB)
		{
			return;
		}
		auto context = aContext.lock()->Get();

		auto& bloomVars = myCache->BloomPack;

		myCache->CurrentPostProcessResultFB->BindSRV(0, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("FirstBloomDownSample", bloomVars.BloomMipFBs[0], context);
		myCache->CurrentPostProcessResultFB->UnBindShaderResource(0, ShaderType::Pixel, context);

		for (size_t i = 1; i < bloomVars.BloomMipFBs.size(); ++i)
		{
			myCache->CameraBuffData.Resolution = { (float)bloomVars.BloomMipFBs[i - 1]->GetSpecs().Width,(float)bloomVars.BloomMipFBs[i - 1]->GetSpecs().Height };
			myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
			myCache->Camerabuffer.Bind(0, context);
			bloomVars.BloomMipFBs[i - 1]->BindSRV(0, 0, ShaderType::Pixel, context);
			PostProcessUtils::FullQuadPass("BloomDownSample", bloomVars.BloomMipFBs[i], context);
			bloomVars.BloomMipFBs[i - 1]->UnBindShaderResource(0, ShaderType::Pixel, context);
		}

		for (size_t i = bloomVars.BloomMipFBs.size() - 1; i > 0; i--)
		{
			const auto& mip = bloomVars.BloomMipFBs[i];
			const auto& nextMip = bloomVars.BloomMipFBs[i - 1];

			mip->BindSRV(0, 0, ShaderType::Pixel, context);
			nextMip->BindSRV(1, 0, ShaderType::Pixel, context);
			PostProcessUtils::FullQuadPass("BloomUpSample", nextMip, context);
			mip->UnBindShaderResource(0, ShaderType::Pixel, context);
			mip->UnBindShaderResource(1, ShaderType::Pixel, context);
		}

		const auto& finalMip = bloomVars.BloomMipFBs[0];
		myCache->CurrentPostProcessResultFB->BindSRV(0, 0, ShaderType::Pixel, context);
		finalMip->BindSRV(1, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("Bloom", bloomVars.FinalFB, context);
		finalMip->UnBindShaderResource(0, ShaderType::Pixel, context);
		finalMip->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->CurrentPostProcessResultFB = bloomVars.FinalFB;
		myCache->CameraBuffData.Resolution = { (float)myCache->RendererFrameBuffer->GetSpecs().Width,(float)myCache->RendererFrameBuffer->GetSpecs().Height };
		myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
		myCache->Camerabuffer.Bind(0, context);
	}
	void PostProcessRenderer::RunCustomPasses(Ptr<DeferredContext> aContext)
	{
		auto context = aContext.lock()->Get();
		for (size_t i = 0; i < 4; ++i)
		{
			if (myCache->CustomPostprocessPasses[i].lock() == nullptr || !myCache->CustomPostprocessPasses[i].lock()->IsLoaded())
			{
				continue;
			}

			myCache->CurrentPostProcessResultFB->BindSRV(20, 0, ShaderType::Pixel, context);
			myCache->DepthPrePass->BindDepthSRV(21, ShaderType::Pixel, context);
			myCache->DepthPrePass->BindSRV(22, 1, ShaderType::Pixel, context);
			myCache->DeferredGBuffer->BindSRV(23, 1, ShaderType::Pixel, context);

			myCache->CustomPostProcess[i]->Bind(context);

			auto pipelineID = myCache->CustomPostprocessPasses[i].lock()->GetInfo().PipelineID;

			PipelineLibrary::Get(pipelineID).Bind(context);

			myCache->CustomPostprocessPasses[i].lock()->Bind(context);

			myCache->MaterialBuffer.SetData(myCache->CustomPostprocessPasses[i].lock()->GetInfo().MaterialData.data.data(), myCache->CustomPostprocessPasses[i].lock()->GetInfo().MaterialData.data.size(), context);
			myCache->MaterialBuffer.Bind(10, ShaderType::Pixel, context);

			context->Draw(3, 0);

			PipelineLibrary::Get(pipelineID).UnBind(context);

			myCache->CustomPostprocessPasses[i].lock()->UnBind(context);

			myCache->CustomPostProcess[i]->UnBind(context);

			myCache->CurrentPostProcessResultFB->UnBindShaderResource(20, ShaderType::Pixel, context);
			myCache->CurrentPostProcessResultFB->UnBindShaderResource(21, ShaderType::Pixel, context);
			myCache->CurrentPostProcessResultFB->UnBindShaderResource(22, ShaderType::Pixel, context);
			myCache->CurrentPostProcessResultFB->UnBindShaderResource(23, ShaderType::Pixel, context);

			myCache->CurrentPostProcessResultFB = myCache->CustomPostProcess[i];
		}
	}
}