#include "FFpch.h"
#include "SceneRenderer.h"

// Std
#include <vector>
// -- //

// Rendering Common //
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/RenderingDefines.h"
#include "Firefly/Rendering/RenderCache.h"
#include "Firefly/Asset/Material/MaterialAsset.h"
// ---------------- //

// Buffers //
#include "Firefly/Rendering/Buffer/IndexBuffer.h"
#include "Firefly/Rendering/Buffer/VertexBuffer.h"
#include "Firefly/Rendering/Buffer/ConstantBuffers.h"
#include "Firefly/Rendering/Buffer/ConstantBuffer.hpp"
#include "Firefly/Rendering/Buffer/StructuredBuffer.h"
#include "Firefly/Rendering/Buffer/UndefinedConstBuffer.h"
// {}------ //

// Postprocess //
#include "Firefly/Rendering/Postprocess/PostProcessUtils.h"
// ----------- //

#include "Firefly/Rendering/Shader/ShaderLibrary.h"

// External //
#include "Utils/Timer.h"
#include <Firefly/Rendering/AtmosphericSky/SkyAtmosphereData.h>
#include "HBAOPlus/GFSDK_SSAO.h"
#include <Firefly/Rendering/Postprocess/PostProcessRenderer.h>
#include <Firefly/Rendering/Shadows/CascadedShadows.h>

#include "ParticleSystem/ParticleEmitter.h"
// -------- //

namespace Firefly
{
	SceneRenderer::SceneRenderer(Cache* aCache, uint32_t aID, Ptr<DeferredContext> aContext, size_t frameIndex) : myCache(aCache), myID(aID)
	{
		FF_PROFILESCOPE("Firefly::SceneRenderer::PrepareFrame");
		myShouldFlush = true;
		if (!aCache->CurrentCamera)
		{
			return;
		}
		myFrameIndex = frameIndex;
		aContext.lock()->BeginEvent("Prepare Frame");
		auto context = aContext.lock()->Get();
		auto& rsm = GraphicsContext::GetRenderStateManager();
		rsm.SetSamplerState(SamplerState::Wrap, 0, context);
		rsm.SetSamplerState(SamplerState::Border, 1, context);
		rsm.SetSamplerState(SamplerState::Mirror, 2, context);
		rsm.SetSamplerState(SamplerState::Point, 3, context);
		rsm.SetSamplerState(SamplerState::Clamp, 4, context);
		rsm.SetSamplerState(SamplerState::Shadow, 5, context);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::None, context);
		for (auto& TracedFramebuffer : aCache->TracedFramebuffers)
		{
			TracedFramebuffer.FrameBuffer->Clear({ 0.1f, 0.1f, 0.1f, 1.f }, context);
		}
		aCache->DepthPrePass->Clear({ 0,0,0,1.f }, context);
		{
			FF_PROFILESCOPE("Build cascades");
			auto cascades = CascadeBuilder(aCache->DirLightBuffData).SetCascadeCount(4).SetActiveCamera(aCache->CurrentCamera).Build();
			for (size_t i = 0; i < cascades.size(); ++i)
			{
				aCache->DirLightBuffData.DirectionLightPacket[i].ProjMatrix = cascades[i].Projection;
				aCache->DirLightBuffData.DirectionLightPacket[i].ViewMatrix = cascades[i].View;
				aCache->DirLightBuffData.DirectionLightPacket[i].Direction.w = cascades[i].Levels;
			}
		}

		Renderer::SortCommands(myCache);

		std::vector<MeshSubmitInfo> deferredCulledMeshes;
		std::vector<MeshSubmitInfo> forwardCulledMeshes;
		std::vector<MeshSubmitInfo> forwardFPSMeshes;

		for (auto& cmd : myCache->DeferredCommands)
		{
			if (myCache->CurrentCamera->MeshIsVisible(cmd.BoundingSphere))
			{
				deferredCulledMeshes.emplace_back(cmd);
			}
		}

		for (auto& cmd : myCache->ForwardCommands)
		{
			if (cmd.Material->GetInfo().ShouldBeFPS)
			{
				forwardFPSMeshes.emplace_back(cmd);
			}

			if (myCache->CurrentCamera->MeshIsVisible(cmd.BoundingSphere))
			{
				forwardCulledMeshes.emplace_back(cmd);
			}
		}


		auto filterNoMaterialLambda = [](size_t commandIndex, size_t containerSize, size_t currentBatchSize, MeshSubmitInfo& currentCommand, MeshSubmitInfo& nextCommand)
		{
			return (commandIndex == (containerSize - 1) || currentBatchSize >= MAX_MESH_BATCH_ZERO_INDEXED || *currentCommand.Mesh != *nextCommand.Mesh);
		};

		auto filterMaterialLambda = [](size_t commandIndex, size_t containerSize, size_t currentBatchSize, MeshSubmitInfo& currentCommand, MeshSubmitInfo& nextCommand)
		{
			return (commandIndex == (containerSize - 1) || currentBatchSize >= MAX_MESH_BATCH_ZERO_INDEXED || *currentCommand.Mesh != *nextCommand.Mesh || *currentCommand.Material.get() != *nextCommand.Material.get());
		};

		auto filterShadowNoDrawLambda = [](MeshSubmitInfo& currentCommand)
		{
			return !currentCommand.CastShadows;
		};

		Renderer::CollectDrawCallsFromList(Draw::Deferred, myCache->MeshBatches, myCache->DeferredCommands, filterNoMaterialLambda, LOD::DontOverride, filterShadowNoDrawLambda);

		Renderer::CollectDrawCallsFromList(Draw::Forward, myCache->MeshBatches, myCache->ForwardCommands, filterNoMaterialLambda, LOD::DontOverride, filterShadowNoDrawLambda);

		Renderer::CollectDrawCallsFromList(Draw::Outline, myCache->MeshBatches, myCache->OutlineCommands, filterNoMaterialLambda, LOD::DontOverride);

		Renderer::CollectDrawCallsFromList(Draw::CulledDeferred, myCache->MeshBatches, deferredCulledMeshes, filterMaterialLambda, LOD::DontOverride);

		Renderer::CollectDrawCallsFromList(Draw::CulledForward, myCache->MeshBatches, forwardCulledMeshes, filterMaterialLambda, LOD::DontOverride);

		Renderer::CollectDrawCallsFromList(Draw::FPSForward, myCache->MeshBatches, forwardFPSMeshes, filterMaterialLambda, LOD::Ultra);

		Renderer::CollectDrawCallsFromList(Draw::Decals, myCache->MeshBatches, myCache->DecalCommands, filterMaterialLambda, LOD::Ultra);

		deferredCulledMeshes.clear();
		// Cull per pointLight shadow
		for (size_t i = 0; i < myCache->PointLightIterator; ++i)
		{
			auto& pointLight = myCache->PointLightBuffData.PointLightPackets[i];

			for (auto& cmd : myCache->DeferredCommands)
			{
				const auto meshToLight = cmd.BoundingSphere.GetOrigin() - pointLight.Position;

				if (meshToLight.Length() - cmd.BoundingSphere.GetRadius() < pointLight.Radius)
				{
					deferredCulledMeshes.emplace_back(cmd);
				}
			}

			for (auto& cmd : myCache->ForwardCommands)
			{
				const auto meshToLight = cmd.BoundingSphere.GetOrigin() - pointLight.Position;

				if (meshToLight.Length() - cmd.BoundingSphere.GetRadius() < pointLight.Radius)
				{
					deferredCulledMeshes.emplace_back(cmd);
				}
			}

			Renderer::CollectDrawCallsFromList(Renderer::ExtractDrawFlagFromIndex(Draw::PointLight0, i), myCache->MeshBatches, deferredCulledMeshes, filterNoMaterialLambda, LOD::DontOverride, filterShadowNoDrawLambda);
		}

		deferredCulledMeshes.clear();

		for (size_t i = 0; i < myCache->SpotLightIterator; ++i)
		{
			auto& spotLight = myCache->SpotLightBuffData.SpotLightPackets[i];

			if (spotLight.Range_Inner_Outer_ShouldCastShadow.w < 1.f)
			{
				continue;
			}

			auto vec3Position = Utils::Vec4ToVec3(spotLight.Position);
			auto vec3Direction = Utils::Vec4ToVec3(spotLight.Direction);
			Utils::Transform spotTransform(vec3Position, Utils::Quaternion::CreateLookRotation(vec3Direction), Utils::Vec3(1.f));

			auto spotFustrum = Camera::CreateViewFrustum(spotTransform, 512, 512, 1.f, spotLight.Range_Inner_Outer_ShouldCastShadow.x, RADTODEG(spotLight.Range_Inner_Outer_ShouldCastShadow.z));

			for (auto& cmd : myCache->DeferredCommands)
			{
				if (spotFustrum.MeshIsVisible(cmd.BoundingSphere))
				{
					deferredCulledMeshes.emplace_back(cmd);
				}
			}

			for (auto& cmd : myCache->ForwardCommands)
			{
				if (spotFustrum.MeshIsVisible(cmd.BoundingSphere))
				{
					deferredCulledMeshes.emplace_back(cmd);
				}
			}

			Renderer::CollectDrawCallsFromList(Renderer::ExtractDrawFlagFromIndex(Draw::SpotLight0, i), myCache->MeshBatches, deferredCulledMeshes, filterNoMaterialLambda, LOD::DontOverride, filterShadowNoDrawLambda);
			deferredCulledMeshes.clear();
		}

		Renderer::CompileMeshBatches(myCache, context);

		std::vector<DrawInstancedIndirectArgs> particleArgs;
		particleArgs.reserve(myCache->ParticleEmitterCommands.size());
		for (const auto& emitter : myCache->ParticleEmitterCommands)
		{
			particleArgs.emplace_back(static_cast<UINT>(emitter.Emitter->GetParticleCount()), 1, 0, 0);
		}

		myCache->ParticleDrawCallsBuffer->SetData(particleArgs, aContext.lock()->Get());

		aContext.lock()->EndEvent();
		myCache->triCount = 0;
		myCache->LineCount = 0;
		myCache->DrawCount = 0;
	}

	void SceneRenderer::UpdateBuffers(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Update buffers");
		auto cache = myCache;
		auto context = aContext.lock()->Get();

		if (!cache->CurrentCamera)
		{
			return;
		}
		cache->CameraBuffData.CameraSpace = cache->CurrentCamera->GetViewMatrix();
		cache->CameraBuffData.ToProjectionSpace = cache->CurrentCamera->GetProjectionMatrixPerspective();
		cache->CameraBuffData.CameraPosition = { cache->CurrentCamera->GetTransform().GetPosition().x, cache->CurrentCamera->GetTransform().GetPosition().y, cache->CurrentCamera->GetTransform().GetPosition().z, 1 };
		cache->CameraBuffData.NearPlane = cache->CurrentCamera->GetNearPlane();
		cache->CameraBuffData.FarPlane = cache->CurrentCamera->GetFarPlane();
		cache->CameraBuffData.Resolution = { (float)cache->RendererFrameBuffer->GetSpecs().Width, (float)cache->RendererFrameBuffer->GetSpecs().Height };
		cache->Camerabuffer.SetData(&cache->CameraBuffData, context);
		cache->Camerabuffer.Bind(0, context);


		cache->DirLightBuffData.Count = { (int)cache->DirLightIterator, 0, 0, 0 };
		cache->DirLightBuffer.SetData(&cache->DirLightBuffData, context);
		cache->DirLightBuffer.Bind(2, context);

		cache->PointLightBuffData.Count = { (int)cache->PointLightIterator, 0, 0, 0 };
		cache->PointLightBuffer.SetData(&cache->PointLightBuffData, context);
		cache->PointLightBuffer.Bind(4, context);

		cache->SpotLightBuffData.Count = { (int)cache->SpotLightIterator, 0, 0, 0 };
		cache->SpotLightBuffer.SetData(&cache->SpotLightBuffData, context);
		cache->SpotLightBuffer.Bind(5, context);

		cache->RenderPassBuffData.RenderPassId = globalRendererStats.VisablePass;
		cache->RenderPassBuffData.EnvironmentIntensity = cache->EnvironmentInfo.Intensity;
		cache->RenderPassBuffer.SetData(&cache->RenderPassBuffData, context);
		cache->RenderPassBuffer.Bind(6, context);

		cache->TimeBuffData.TimeInfo =
		{
			Utils::Timer::GetScaledTotalTime(),
			static_cast<float>(Utils::Timer::GetTotalTime()),
			Utils::Timer::GetDeltaTime(),
			Utils::Timer::GetUnscaledDeltaTime()
		};
		cache->TimeBuffer.SetData(&cache->TimeBuffData, context);
		cache->TimeBuffer.Bind(12, context);

		if (cache->EnvironmentInfo.EnvironmentMap)
		{
			cache->EnvironmentInfo.EnvironmentMap->Bind(30, ShaderType::Pixel, context);
		}

		cache->InstanceGPUBuffer->Bind(70, ShaderType::Vertex, context);
		cache->InstanceBoneGPUBuffer->Bind(71, ShaderType::Vertex, context);

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::PrePass(Ptr<DeferredContext> aContext)
	{
		aContext.lock()->BeginEvent("Pre-Pass");
		FF_PROFILEFUNCTION();
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::Back, aContext.lock()->Get());

		myCache->DepthPrePass->Bind(aContext.lock()->Get());
		aContext.lock()->BeginEvent("Meshes");

		ShaderLibrary::Bind("DepthPrepass", aContext.lock()->Get());

		Renderer::RenderGeometry(myCache, Draw::CulledDeferred, Binding::DontBindMaterial, aContext.lock()->Get());
		Renderer::RenderGeometry(myCache, Draw::CulledForward, Binding::DontBindMaterial, aContext.lock()->Get());
		Renderer::RenderGeometry(myCache, Draw::Decals, Binding::DontBindMaterial, aContext.lock()->Get());

		ShaderLibrary::Unbind("DepthPrepass", aContext.lock()->Get());
		aContext.lock()->EndEvent();
		GraphicsContext::GetRenderStateManager().PopCullState(aContext.lock()->Get());

		aContext.lock()->BeginEvent("Billboards");
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::None, aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Opaque, aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, aContext.lock()->Get());
		GraphicsContext::Topology(TopologyType::POINTLIST, aContext.lock()->Get());

		ShaderLibrary::Bind("Billboard", aContext.lock()->Get());

		Renderer::BeginBillboardBatch(myCache);

		for (auto& vertex : myCache->BillBoardBatchSpecs.BillboardContainer)
		{
			Renderer::DrawBillboard(myCache, vertex, aContext.lock()->Get());
		}

		Renderer::EndBillboardBatch(myCache, aContext.lock()->Get());

		ShaderLibrary::Unbind("Billboard", aContext.lock()->Get());
		GraphicsContext::Topology(TopologyType::TRIANGLELIST, aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PopBlendState(aContext.lock()->Get());
		aContext.lock()->EndEvent();

		myCache->DepthPrePass->UnBind(aContext.lock()->Get());

		GraphicsContext::GetRenderStateManager().PopCullState(aContext.lock()->Get());
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::DirectionalLightShadowPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Directional Shadow Pass");
		auto context = aContext.lock()->Get();
		myCache->DirectionalShadowBuffer->UnBindShaderResource(20, ShaderType::Pixel, context);
		if (myCache->DirLightBuffData.DirectionLightPacket[0].dirLightInfo.x > 0)
		{
			const auto& shadowMap = myCache->DirectionalShadowBuffer;
			shadowMap->Clear({ 1,1,1,1 }, context);

			if (myCache->CurrentCamera)
			{
				myCache->CameraBuffData.CameraSpace = myCache->DirLightBuffData.DirectionLightPacket[0].ViewMatrix;
				myCache->CameraBuffData.ToProjectionSpace = myCache->DirLightBuffData.DirectionLightPacket[0].ProjMatrix;
				myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
				myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
				myCache->Camerabuffer.Bind(0, context);
			}
			GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadWrite, context);
			GraphicsContext::GetRenderStateManager().PushCullState(CullState::Front, context);

			ShaderLibrary::Bind("DirectionalShadows", context);
			myCache->DirectionalShadowBuffer->Bind(context);

			Renderer::RenderGeometry(myCache, Draw::Deferred, Binding::DontBindMaterial, context);
			Renderer::RenderGeometry(myCache, Draw::Forward, Binding::DontBindMaterial, context);

			myCache->DirectionalShadowBuffer->UnBind(context);
			ShaderLibrary::Unbind("DirectionalShadows", context);

			GraphicsContext::GetRenderStateManager().PopCullState(context);
			GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		}
		if (myCache->CurrentCamera)
		{
			myCache->CameraBuffData.CameraSpace = myCache->CurrentCamera->GetViewMatrix();
			myCache->CameraBuffData.ToProjectionSpace = myCache->CurrentCamera->GetProjectionMatrixPerspective();
			myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
			myCache->CameraBuffData.NearPlane = myCache->CurrentCamera->GetNearPlane();
			myCache->CameraBuffData.FarPlane = myCache->CurrentCamera->GetFarPlane();
			myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
			myCache->Camerabuffer.Bind(0, context);
		}
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::PointLightShadowPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Point Shadow Pass");
		auto context = aContext.lock()->Get();

		for (size_t i = 0; i < myCache->PointLightIterator; ++i)
		{
			auto& currentSpotLight = myCache->PointLightBuffData.PointLightPackets[i];
			if (currentSpotLight.PointlightCustomData.w < 1)
			{
				continue;
			}

			const auto& shadowMap = myCache->PointLightShadowFB[static_cast<size_t>(currentSpotLight.PointlightCustomData.z)];
			shadowMap->Clear({ 1,1,1,1 }, context);

			if (myCache->CurrentCamera)
			{
				myCache->CameraBuffData.FarPlane = myCache->PointLightIterator; // pack the data to a existing format.
				myCache->CameraBuffData.NearPlane = currentSpotLight.PointlightCustomData.z; // pack the data to a existing format.
				myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
				myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
				myCache->Camerabuffer.Bind(0, context);
			}


			GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadWrite, context);
			GraphicsContext::GetRenderStateManager().PushCullState(CullState::Front, context);

			ShaderLibrary::Bind("PointShadows", context);
			shadowMap->Bind(context);

			Renderer::RenderGeometry(myCache, Renderer::ExtractDrawFlagFromIndex(Draw::PointLight0, static_cast<uint32_t>(currentSpotLight.PointlightCustomData.z)), Binding::DontBindMaterial, context);

			shadowMap->UnBind(context);
			ShaderLibrary::Unbind("PointShadows", context);

			GraphicsContext::GetRenderStateManager().PopCullState(context);
			GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

			if (myCache->CurrentCamera)
			{
				myCache->CameraBuffData.CameraSpace = myCache->CurrentCamera->GetViewMatrix();
				myCache->CameraBuffData.ToProjectionSpace = myCache->CurrentCamera->GetProjectionMatrixPerspective();
				myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
				myCache->CameraBuffData.NearPlane = myCache->CurrentCamera->GetNearPlane();
				myCache->CameraBuffData.FarPlane = myCache->CurrentCamera->GetFarPlane();
				myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
				myCache->Camerabuffer.Bind(0, context);
			}
		}

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::SpotLightShadowPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Spot Shadow Pass");
		auto context = aContext.lock()->Get();
		//myCache->DirectionalShadowBuffer->UnBindShaderResource(20, ShaderType::Pixel, context);

		for (size_t i = 0; i < myCache->SpotLightIterator; ++i)
		{
			auto& currentSpotLight = myCache->SpotLightBuffData.SpotLightPackets[i];
			if (currentSpotLight.Range_Inner_Outer_ShouldCastShadow.w < 1)
			{
				continue;
			}
			const auto shadowMapIndex = static_cast<uint32_t>(currentSpotLight.Direction.w);
			const auto& shadowMap = myCache->SpotLightShadowFB[shadowMapIndex];
			shadowMap->Clear({ 1,1,1,1 }, context);

			if (myCache->CurrentCamera)
			{
				myCache->CameraBuffData.CameraSpace = currentSpotLight.ViewProjMatrix;
				myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
				myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
				myCache->Camerabuffer.Bind(0, context);
			}


			GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadWrite, context);
			GraphicsContext::GetRenderStateManager().PushCullState(CullState::Front, context);

			ShaderLibrary::Bind("SpotShadows", context);
			shadowMap->Bind(context);

			Renderer::RenderGeometry(myCache, Renderer::ExtractDrawFlagFromIndex(Draw::SpotLight0, shadowMapIndex), Binding::DontBindMaterial, context);

			shadowMap->UnBind(context);
			ShaderLibrary::Unbind("SpotShadows", context);

			GraphicsContext::GetRenderStateManager().PopCullState(context);
			GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

			if (myCache->CurrentCamera)
			{
				myCache->CameraBuffData.CameraSpace = myCache->CurrentCamera->GetViewMatrix();
				myCache->CameraBuffData.ToProjectionSpace = myCache->CurrentCamera->GetProjectionMatrixPerspective();
				myCache->CameraBuffData.CameraPosition = { myCache->CurrentCamera->GetTransform().GetPosition().x, myCache->CurrentCamera->GetTransform().GetPosition().y, myCache->CurrentCamera->GetTransform().GetPosition().z, 1 };
				myCache->CameraBuffData.NearPlane = myCache->CurrentCamera->GetNearPlane();
				myCache->CameraBuffData.FarPlane = myCache->CurrentCamera->GetFarPlane();
				myCache->Camerabuffer.SetData(&myCache->CameraBuffData, context);
				myCache->Camerabuffer.Bind(0, context);
			}
		}

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::VolumePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		if (myCache->RenderPassBuffData.IsVolumetricFogActive_Pad.x > 0.f)
		{
			aContext.lock()->BeginEvent("Volume Pass");
			ParticipatingMediaPass(aContext);
			LightIntegrationPass(aContext);
			aContext.lock()->EndEvent();
		}
	}

	void SceneRenderer::OutlinePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Outline Pass");
		auto context = aContext.lock()->Get();
		GraphicsContext::Topology(TopologyType::TRIANGLELIST, context);
		myCache->OutlineFB->Bind(context);

		ShaderLibrary::Bind("JFA_Initial", context);

		Renderer::RenderGeometry(myCache, Draw::Outline, Binding::DontBindMaterial, context);

		myCache->OutlineFB->UnBind(context);
		ShaderLibrary::Unbind("JFA_Initial", context);

		myCache->OutlineFB->BindSRV(0, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("JFAuv", myCache->JFAPrepass_FB, context);
		myCache->OutlineFB->UnBindShaderResource(0, ShaderType::Pixel, context);

		myCache->JFAPrepass_FB->BindSRV(0, 0, ShaderType::Pixel, context);
		myCache->OutlineFB->BindSRV(1, 0, ShaderType::Pixel, context);
		PostProcessUtils::FullQuadPass("JFACookie", myCache->JFAFinal_FB, context);
		myCache->JFAPrepass_FB->UnBindShaderResource(0, ShaderType::Pixel, context);
		myCache->JFAPrepass_FB->UnBindShaderResource(1, ShaderType::Pixel, context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::SkyLightPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();

		DynamicSkyBox(aContext);
	}

	void SceneRenderer::GridPass(Ptr<DeferredContext> aContext) const
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Grid");

		auto context = aContext.lock()->Get();
		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Translucent, context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);

		ShaderLibrary::Bind("Grid", context);
		context->Draw(6, 0);
		ShaderLibrary::Unbind("Grid", context);

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		GraphicsContext::GetRenderStateManager().PopBlendState(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::DeferredPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("G-Buffer Pass");
		auto context = aContext.lock()->Get();

		myCache->DeferredGBuffer->Clear({ 0.f,0.f,0.f,0.f }, context);
		myCache->DeferredGBuffer->Bind(context);

		Renderer::RenderGeometry(myCache, Draw::CulledDeferred, Binding::BindMaterial, context);

		myCache->DeferredGBuffer->UnBind(context);

		myCache->DeferredGBuffer->BindSpecificRenderTargets({0, 1, 2}, context);

		myCache->DeferredGBuffer->BindDepthSRV(20, ShaderType::Pixel | ShaderType::Vertex, context);
		myCache->DeferredGBuffer->BindSRV(21, 4, ShaderType::Pixel | ShaderType::Vertex, context);

		Renderer::RenderGeometry(myCache, Draw::Decals, Binding::BindMaterial, context);

		myCache->DeferredGBuffer->UnBindShaderResource(20, ShaderType::Pixel | ShaderType::Vertex, context);
		myCache->DeferredGBuffer->UnBindShaderResource(21, ShaderType::Pixel | ShaderType::Vertex, context);

		myCache->DeferredGBuffer->UnBind(context);

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::ApplyVolumePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Apply Volume Pass");
		auto context = aContext.lock()->Get();

		myCache->ForwardGBuffer->UnBind(context);

		myCache->ForwardGBuffer->BindSRV(2, 0, ShaderType::Pixel, context);
		myCache->RayMarchVolume->Bind(1, ShaderType::Pixel, context);
		myCache->DepthPrePass->BindDepthSRV(0, ShaderType::Pixel, context);

		PostProcessUtils::FullQuadPass("ApplyVolume", myCache->VolumeResultFB, context);

		myCache->ForwardGBuffer->UnBindShaderResource(0, ShaderType::Pixel, context);
		myCache->ForwardGBuffer->UnBindShaderResource(1, ShaderType::Pixel, context);
		myCache->ForwardGBuffer->UnBindShaderResource(2, ShaderType::Pixel, context);

		PostProcessUtils::CopyFBtoFB(myCache->VolumeResultFB, myCache->ForwardGBuffer, context);

		myCache->ForwardGBuffer->Bind(context);

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::DeferredLightPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Deferred Pass");
		auto context = aContext.lock()->Get();
		Cache* cache = myCache;
		cache->DeferredGBuffer->BindSRV(0, 0, ShaderType::Compute, context);
		cache->DeferredGBuffer->BindSRV(1, 1, ShaderType::Compute, context);
		cache->DeferredGBuffer->BindSRV(2, 2, ShaderType::Compute, context);
		cache->DeferredGBuffer->BindSRV(3, 3, ShaderType::Compute, context);
		cache->DeferredGBuffer->BindSRV(4, 4, ShaderType::Compute, context);
		cache->DeferredGBuffer->BindSRV(5, 5, ShaderType::Compute, context);
		cache->OcclusionFB->BindSRV(6, 0, ShaderType::Compute, context);
		cache->ForwardGBuffer->BindSRV(7, 0, ShaderType::Compute, context);
		cache->DirectionalShadowBuffer->BindDepthSRV(20, ShaderType::Compute, context);

		for (size_t i = 1; auto & spotFB : cache->SpotLightShadowFB)
		{
			spotFB->BindDepthSRV(20 + i, ShaderType::Compute, context);
			i++;
		}
		for (size_t i = 1; auto & spotFB : cache->PointLightShadowFB)
		{
			spotFB->BindDepthSRV(30 + i, ShaderType::Compute, context);
			i++;
		}

		cache->TransmittanceLUT->BindSRV(40, 0, ShaderType::Compute, context);

		myCache->RayMarchVolume->Bind(50, ShaderType::Compute, context);
		GetEnvironmentTexture()->Bind(30, ShaderType::Compute, context);

		ShaderLibrary::Bind("DeferredCS", context);

		auto& specs = myCache->RendererFrameBuffer->GetSpecs();

		myCache->DeferredUAVTexture->BindUAV(0, context);

		context->Dispatch((specs.Width / 8) + 1, (specs.Height / 8) + 1, 1);

		myCache->DeferredUAVTexture->UnbindUAV(0, context);

		ShaderLibrary::Unbind("DeferredCS", context);

		cache->DeferredGBuffer->UnBindShaderResource(0, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(1, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(2, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(3, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(4, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(5, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(6, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(7, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(20, ShaderType::Compute, context);

		for (size_t i = 1; auto & spotFB : cache->SpotLightShadowFB)
		{
			spotFB->UnBindShaderResource(20 + i, ShaderType::Compute, context);
			i++;
		}
		for (size_t i = 1; auto & spotFB : cache->PointLightShadowFB)
		{
			spotFB->UnBindShaderResource(30 + i, ShaderType::Compute, context);
			i++;
		}

		cache->DeferredGBuffer->UnBindShaderResource(30, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(40, ShaderType::Compute, context);
		cache->DeferredGBuffer->UnBindShaderResource(50, ShaderType::Compute, context);

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);

		cache->DeferredUAVTexture->Bind(0, ShaderType::Pixel, context);

		PostProcessUtils::FullQuadPass("Copy", cache->ForwardGBuffer, context);

		cache->DeferredUAVTexture->UnBind(0, ShaderType::Pixel, context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadWrite, context);

		Renderer::CopyDepth(cache->DeferredGBuffer, cache->ForwardGBuffer, context);

		cache->ForwardGBuffer->Bind(context);

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::HBAOPass(Ptr<DeferredContext> aContext, GFSDK_SSAO_Context_D3D11* aAOContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("HBAO Pass");
		auto context = aContext.lock()->Get();
		myCache->OcclusionFB->Clear({ 1,1,1,1 }, context);
		const auto& cam = myCache->CurrentCamera;
		auto proj = Utils::Mat4::CreateLeftHandedProjectionMatrixPerspective(cam->GetSizeX(), cam->GetSizeY(), cam->GetNearPlane(), cam->GetFarPlane(), cam->GetFov() / 2.f);
		static GFSDK_SSAO_InputData_D3D11 Input = {};
		Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		Input.DepthData.pFullResDepthTextureSRV = myCache->DeferredGBuffer->GetDepthAttachment().Get();
		Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&proj);
		Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		Input.DepthData.MetersToViewSpaceUnits = 100.f;

		Input.NormalData.Enable = true;
		Input.NormalData.pFullResNormalTextureSRV = myCache->DeferredGBuffer->GetColorAttachment(1).Get();
		Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&cam->GetViewMatrix());
		Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

		static GFSDK_SSAO_Parameters Params = {};
		Params.Bias = 0.1f;
		Params.PowerExponent = 1.f;
		Params.Radius = 2.0f;
		Params.SmallScaleAO = 0.6f;
		Params.LargeScaleAO = 1.000f;
		Params.Blur.Enable = true;
		Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
		Params.Blur.Sharpness = 32.f;
		GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;
		GFSDK_SSAO_Output_D3D11 Output = {};
		Output.pRenderTargetView = myCache->OcclusionFB->GetRenderTarget(0).Get();
		Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;
		auto status = aAOContext->RenderAO(context, Input, Params, Output, RenderMask);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::ForwardPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Forward Pass");

		auto context = aContext.lock()->Get();

		myCache->DirectionalShadowBuffer->BindDepthSRV(20, ShaderType::Pixel, context);
		GetEnvironmentTexture()->Bind(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->BindSRV(40, 0, ShaderType::Pixel, context);
		myCache->RayMarchVolume->Bind(50, ShaderType::Pixel, context);

		Renderer::RenderGeometry(myCache, Draw::CulledForward, Binding::BindMaterial, context);

		myCache->DirectionalShadowBuffer->UnBindShaderResource(20, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(40, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(50, ShaderType::Pixel, context);

		aContext.lock()->BeginEvent("Billboards");
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, aContext.lock()->Get());
		GraphicsContext::Topology(TopologyType::POINTLIST, aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Translucent, aContext.lock()->Get());
		ShaderLibrary::Bind("BillboardColor", aContext.lock()->Get());

		Renderer::BeginBillboardBatch(myCache);

		for (auto& vertex : myCache->BillBoardBatchSpecs.BillboardContainer)
		{
			Renderer::DrawBillboard(myCache, vertex, aContext.lock()->Get());
		}

		Renderer::EndBillboardBatch(myCache, aContext.lock()->Get());

		ShaderLibrary::Unbind("BillboardColor", aContext.lock()->Get());
		GraphicsContext::Topology(TopologyType::TRIANGLELIST, aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(aContext.lock()->Get());
		GraphicsContext::GetRenderStateManager().PopBlendState(aContext.lock()->Get());
		aContext.lock()->EndEvent();

		aContext.lock()->EndEvent();

		FPSOverlapPass(aContext);

		CombineMainAndFPSPass(aContext);
	}

	void SceneRenderer::ParticlePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Particle Pass");
		auto context = aContext.lock()->Get();

		//ShaderLibrary::Bind("Particle", context);

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		/*myCache->DirectionalShadowBuffer->BindDepthSRV(20, ShaderType::Pixel, context);
		Renderer::GetEnvi()->Bind(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->BindSRV(40, 0, ShaderType::Pixel, context);
		myCache->RayMarchVolume->Bind(50, ShaderType::Pixel, context);*/

		size_t i = 0;
		for (const auto& emitterCommand : myCache->ParticleEmitterCommands)
		{
			const auto& material = emitterCommand.Emitter->GetMaterial();

			if (!material)
			{
				continue;
			}

			FF_PROFILESCOPE("Draw single system");

			emitterCommand.Emitter->GetMaterial()->BindWithPipeline(context);

			GraphicsContext::GetRenderStateManager().PushBlendState(emitterCommand.Emitter->GetEmitterSettings().BlendState, context);

			ParticleBufferData bufferData;
			bufferData.ToWorld = emitterCommand.ParentTransform;
			myCache->ParticleBuffer.SetData(&bufferData, context);
			myCache->ParticleBuffer.Bind(3, context);

			myCache->MaterialBuffer.SetData(material->GetInfo().MaterialData.data.data(), material->GetInfo().MaterialData.data.size(), context);
			myCache->MaterialBuffer.Bind(10, ShaderType::Pixel, context);

			emitterCommand.Emitter->Bind(context);

			context->DrawInstancedIndirect(myCache->ParticleDrawCallsBuffer->GetBuffer().Get(), static_cast<UINT>(sizeof(DrawInstancedIndirectArgs) * i));

			GraphicsContext::GetRenderStateManager().PopBlendState(context);

			emitterCommand.Emitter->GetMaterial()->UnbindWithPipeline(context);

			i++;
		}

		/*myCache->DirectionalShadowBuffer->UnBindShaderResource(20, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(40, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(50, ShaderType::Pixel, context);*/

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//ShaderLibrary::Unbind("Particle", context);

		myCache->DeferredGBuffer->UnBind(context);
		myCache->ForwardGBuffer->UnBind(context);
		myCache->RendererFrameBuffer->UnBind(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::PostProcessPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Post Process Pass");
		auto context = aContext.lock()->Get();
		if (globalRendererStats.VisablePass != 0)
		{
			GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
			PostProcessUtils::CopyFBtoFB(myCache->ForwardGBuffer, myCache->RendererFrameBuffer, context);
			GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
			return;
		}

		myCache->CurrentPostProcessResultFB = myCache->ForwardGBuffer;

		PostProcessRenderer postProcessRenderer(myCache);
		postProcessRenderer.RunPostProcessing(aContext);
		myCache->RendererFrameBuffer->Bind(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::UIPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("UI Pass");
		SpritePass(aContext);
		TextPass(aContext);
#ifndef FF_SHIPIT
		LinePass(aContext);
#endif
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::Flush(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Flush Scene");
		myShouldFlush = false;
		auto context = aContext.lock()->Get();

		myCache->RendererFrameBuffer->UnBind(context);
		myCache->PostProcessIsSubmitted = false;

		memset(myCache->LineCacheSpecs.Vertices.data(), 0, sizeof(LineVertex) * myCache->LineCacheSpecs.Vertices.size());
		myCache->LineCacheSpecs.Itreator = 0;

		memset(myCache->LineCache2DSpecs.Vertices.data(), 0, sizeof(LineVertex) * myCache->LineCache2DSpecs.Vertices.size());
		myCache->LineCache2DSpecs.Itreator = 0;

		for (auto& custom : myCache->CustomPostprocessPasses)
		{
			custom.reset();
		}


		memset(&myCache->DirLightBuffData, 0, sizeof(DirLightData));
		myCache->DirLightIterator = 0;

		memset(&myCache->PointLightBuffData, 0, sizeof(PointLightData));
		myCache->PointLightIterator = 0;

		memset(&myCache->SpotLightBuffData, 0, sizeof(SpotLightData));
		myCache->SpotLightIterator = 0;

		for (auto& cmd : myCache->DeferredCommands)
		{
			cmd.CleanUp();
		}
		for (auto& cmd : myCache->ForwardCommands)
		{
			cmd.CleanUp();
		}
		for (auto& cmd : myCache->OutlineCommands)
		{
			cmd.CleanUp();
		}

		myCache->DeferredCommands.clear();
		myCache->ForwardCommands.clear();
		myCache->OutlineCommands.clear();
		myCache->DecalCommands.clear();
		myCache->ParticleEmitterCommands.clear();
		myCache->LineCacheSpecs.Vertices.clear();
		myCache->LineCache2DSpecs.Vertices.clear();
		myCache->BillBoardBatchSpecs.BillboardContainer.clear();
		myCache->MeshBatches.clear();
		aContext.lock()->EndEvent();
	}

	Ref<Texture2D> SceneRenderer::GetEnvironmentTexture()
	{
		return myCache->EnvironmentInfo.EnvironmentMap ? myCache->EnvironmentInfo.EnvironmentMap : Renderer::GetEnvi();
	}

	void SceneRenderer::DynamicSkyBox(Ptr<DeferredContext> aContext)
	{
		aContext.lock()->BeginEvent("Dynamic Skylight Pass");
		Cache* cache = myCache;
		auto context = aContext.lock()->Get();
		SetEarthAtmosphere(cache->AtmosphereParams);
		auto& hv = cache->SkyHelperValues;
		Renderer::UpdateSkyHelperBuffers(cache, context);
		Renderer::UpdateAtmosphereBuffers(cache, context);

		hv.gResolution = { 256, 64 };
		cache->SkyHelperValuesBuffer.SetData(&hv, context);
		cache->SkyHelperValuesBuffer.Bind(7, context);
		cache->SkyAtmosphereBuffer.Bind(8, context);

		cache->TransmittanceLUT->Clear({ 0,0,0,0.f }, context);
		cache->Atmosphere->Clear({ 0,0,0,0 }, context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::None, context);
		PostProcessUtils::FullQuadPass("TransmittanceLUT", cache->TransmittanceLUT, context);
		context->GenerateMips(cache->TransmittanceLUT->GetColorAttachment(0).Get());
		hv.gResolution = { 32, 32 };
		cache->SkyHelperValuesBuffer.Bind(7, context);
		cache->TransmittanceLUT->BindSRV(40, 0, ShaderType::Pixel | ShaderType::Compute, context);
		cache->MultiScatterTexture->BindUAV(0, context);

		ShaderLibrary::Bind("MultiScatter", context);
		context->Dispatch(32, 32, 1);
		cache->TransmittanceLUT->UnBindShaderResource(40, ShaderType::Compute, context);
		cache->MultiScatterTexture->UnbindUAV(0, context);

		cache->MultiScatterTexture->Bind(50, ShaderType::Pixel, context);

		GraphicsContext::GetRenderStateManager().PopCullState(context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::Back, context);

		hv.gResolution = { cache->Atmosphere->GetSpecs().Width,cache->Atmosphere->GetSpecs().Height };

		cache->SkyHelperValuesBuffer.SetData(&hv, context);
		cache->SkyHelperValuesBuffer.Bind(7, context);
		cache->DepthPrePass->BindDepthSRV(4, ShaderType::Pixel, context);
		cache->RayMarchVolume->Bind(48, ShaderType::Pixel, context);

		PostProcessUtils::FullQuadPass("RayMarchingSky", cache->Atmosphere, context);

		cache->ForwardGBuffer->UnBindShaderResource(40, ShaderType::Pixel | ShaderType::Compute, context);
		cache->ForwardGBuffer->UnBindShaderResource(48, ShaderType::Pixel, context);
		cache->ForwardGBuffer->UnBindShaderResource(50, ShaderType::Pixel | ShaderType::Compute, context);
		cache->ForwardGBuffer->UnBindShaderResource(4, ShaderType::Pixel | ShaderType::Compute, context);

		PostProcessUtils::CopyFBtoFB(cache->Atmosphere, cache->ForwardGBuffer, context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

		GraphicsContext::GetRenderStateManager().PopCullState(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::StaticSkyBox(Ptr<DeferredContext> aContext)
	{
		if (!myCache->EnvironmentInfo.EnvironmentMap || !Renderer::GetDefaultCube())
		{
			return;
		}

		aContext.lock()->BeginEvent("Static Skylight Pass");

		auto context = aContext.lock()->Get();

		auto& subMesh = Renderer::GetDefaultCube()->GetSubMeshes()[0];

		subMesh.GetVertexBuffer()->Bind(context);
		subMesh.GetIndexBuffer()->Bind(context);

		ShaderLibrary::Bind("SkyBox", context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::Front, context);
		myCache->EnvironmentInfo.SkyBoxMap->Bind(0, ShaderType::Pixel, context);
		myCache->ForwardGBuffer->Bind(context);
		context->DrawIndexed(subMesh.GetIndexBuffer()->GetBufferSize(), 0, 0);
		myCache->ForwardGBuffer->UnBind(context);
		myCache->EnvironmentInfo.SkyBoxMap->UnBind(0, ShaderType::Pixel, context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		GraphicsContext::GetRenderStateManager().PopCullState(context);

		ShaderLibrary::Unbind("SkyBox", context);

		aContext.lock()->EndEvent();
	}

	void SceneRenderer::ParticipatingMediaPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Froxel Pass");
		ShaderLibrary::Bind("FroxelVolume", aContext.lock()->Get());

		auto dims = myCache->LightInjections[myFrameIndex % 2]->GetDimensions();

		for (size_t i = 1; auto & spotFB : myCache->PointLightShadowFB)
		{
			spotFB->BindDepthSRV(20 + i, ShaderType::Compute, aContext.lock()->Get());
			i++;
		}

		myCache->DirectionalShadowBuffer->BindDepthSRV(35, ShaderType::Compute, aContext.lock()->Get());

		myCache->LightInjections[1]->Bind(36, ShaderType::Compute, aContext.lock()->Get());

		myCache->LightInjections[0]->BindUAV(0, aContext.lock()->Get());

		aContext.lock()->Get()->Dispatch(std::ceil(dims.x / 8.f), std::ceil(dims.y / 8.f), dims.z);

		myCache->LightInjections[0]->UnbindUAV(0, aContext.lock()->Get());

		myCache->LightInjections[1]->UnBind(36, ShaderType::Compute, aContext.lock()->Get());

		for (size_t i = 1; auto & spotFB : myCache->PointLightShadowFB)
		{
			spotFB->UnBindShaderResource(20 + i, ShaderType::Compute, aContext.lock()->Get());
			i++;
		}

		myCache->DirectionalShadowBuffer->UnBindShaderResource(35, ShaderType::Compute, aContext.lock()->Get());

		ShaderLibrary::Unbind("FroxelVolume", aContext.lock()->Get());
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::LightIntegrationPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Light Integration Pass");
		ShaderLibrary::Bind("LightVolume", aContext.lock()->Get());

		auto dims = myCache->RayMarchVolume->GetDimensions();

		myCache->LightInjections[0]->Bind(50, ShaderType::Compute, aContext.lock()->Get());

		myCache->RayMarchVolume->BindUAV(0, aContext.lock()->Get());

		aContext.lock()->Get()->Dispatch(std::ceil(dims.x / 8.f), std::ceil(dims.y / 8.f), 1);

		myCache->RayMarchVolume->UnbindUAV(0, aContext.lock()->Get());

		myCache->LightInjections[0]->UnBind(50, ShaderType::Compute, aContext.lock()->Get());

		ShaderLibrary::Unbind("LightVolume", aContext.lock()->Get());
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::TextPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Text Pass");
		auto context = aContext.lock()->Get();

		ShaderLibrary::Bind("Text", context);

		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Translucent, context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::None, context);

		Renderer::BeginTextBatch(myCache);

		while (!myCache->TextDrawQueue.IsEmpty())
		{
			auto info = myCache->TextDrawQueue.Dequeue();
			Renderer::DrawTextData(info);
		}
		Renderer::EndTextBatch(myCache, context);

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
		Renderer::BeginTextBatch(myCache);

		while (!myCache->NoDepthTextDrawQueue.IsEmpty())
		{
			auto info = myCache->NoDepthTextDrawQueue.Dequeue();
			Renderer::DrawTextData(info);
		}

		Renderer::EndTextBatch(myCache, context);

		GraphicsContext::GetRenderStateManager().PopBlendState(context);
		GraphicsContext::GetRenderStateManager().PopCullState(context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		ShaderLibrary::Unbind("Text", context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::SpritePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Sprite Pass");
		auto context = aContext.lock()->Get();
		GraphicsContext::GetRenderStateManager().PushBlendState(BlendState::Translucent, context);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);
		GraphicsContext::GetRenderStateManager().PushCullState(CullState::None, context);
		ShaderLibrary::Bind("Sprite", context);
		Renderer::BeginSpriteBatch(myCache);
		while (!myCache->SpriteDrawQueue.IsEmpty())
		{
			auto info = myCache->SpriteDrawQueue.Dequeue();
			Renderer::DrawSprite(info, myCache, context);
		}
		Renderer::EndSpriteBatch(myCache, context);

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);

		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::Off, context);
		Renderer::BeginSpriteBatch(myCache);
		while (!myCache->NoDepthSpriteDrawQueue.IsEmpty())
		{
			auto info = myCache->NoDepthSpriteDrawQueue.Dequeue();
			Renderer::DrawSprite(info, myCache, context);
		}
		Renderer::EndSpriteBatch(myCache, context);
		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		GraphicsContext::GetRenderStateManager().PopBlendState(context);
		GraphicsContext::GetRenderStateManager().PopCullState(context);
		ShaderLibrary::Unbind("Sprite", context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::LinePass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Line Pass");
		if (!myCache->LineCacheSpecs.LineBuffer) return;

		auto context = aContext.lock()->Get();
		ShaderLibrary::Bind("LineBatch", context);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		GraphicsContext::GetRenderStateManager().PushDepthStencilState(DepthStencilState::ReadOnly, context);

		Renderer::BeginLineBatch(myCache);

		for (auto& vertex : myCache->LineCacheSpecs.Vertices)
		{
			Renderer::DrawLineVertex(myCache, vertex, context);
		}

		Renderer::EndLineBatch(myCache, context);

		ShaderLibrary::Bind("Line2DBatch", context);

		Renderer::BeginLine2DBatch(myCache);

		for (auto& vertex : myCache->LineCache2DSpecs.Vertices)
		{
			Renderer::DrawLine2DVertex(myCache, vertex, context);
		}

		Renderer::EndLine2DBatch(myCache, context);

		GraphicsContext::GetRenderStateManager().PopDepthStencilState(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::FPSOverlapPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("FPS Overlap Pass");

		auto context = aContext.lock()->Get();

		myCache->ForwardGBuffer->BindWithDifferentDepth(myCache->FPSDepthBuffer, context);


		myCache->DirectionalShadowBuffer->BindDepthSRV(20, ShaderType::Pixel, context);
		GetEnvironmentTexture()->Bind(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->BindSRV(40, 0, ShaderType::Pixel, context);
		myCache->RayMarchVolume->Bind(50, ShaderType::Pixel, context);

		Renderer::RenderGeometry(myCache, Draw::FPSForward, Binding::BindMaterial, context);

		myCache->DirectionalShadowBuffer->UnBindShaderResource(20, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(30, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(40, ShaderType::Pixel, context);
		myCache->TransmittanceLUT->UnBindShaderResource(50, ShaderType::Pixel, context);

		myCache->ForwardGBuffer->Bind(context);
		aContext.lock()->EndEvent();
	}

	void SceneRenderer::CombineMainAndFPSPass(Ptr<DeferredContext> aContext)
	{
		FF_PROFILEFUNCTION();
		aContext.lock()->BeginEvent("Combine Main & FPS pass");



		aContext.lock()->EndEvent();
	}
}