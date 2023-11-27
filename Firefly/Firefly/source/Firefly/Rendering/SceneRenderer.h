#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/RenderCommands.h"
#include <future>

#include <Firefly/Rendering/ThreadSafe/DeferredContext.h>

struct GFSDK_SSAO_Context_D3D11;
namespace Firefly
{
	struct Cache;

	class SceneRenderer
	{
	public:
		SceneRenderer(Cache* aCache, uint32_t aID, Ptr<DeferredContext> aContext, size_t frameIndex);

		void UpdateBuffers(Ptr<DeferredContext> aContext);
		void PrePass(Ptr<DeferredContext> aContext);

		// shadows
		void DirectionalLightShadowPass(Ptr<DeferredContext> aContext);
		void PointLightShadowPass(Ptr<DeferredContext> aContext);
		void SpotLightShadowPass(Ptr<DeferredContext> aContext);

		void VolumePass(Ptr<DeferredContext> aContext);
		void OutlinePass(Ptr<DeferredContext> aContext);
		void SkyLightPass(Ptr<DeferredContext> aContext);
		void GridPass(Ptr<DeferredContext> aContext) const;
		void DeferredPass(Ptr<DeferredContext> aContext);
		void ApplyVolumePass(Ptr<DeferredContext> aContext);
		void DeferredLightPass(Ptr<DeferredContext> aContext);
		void HBAOPass(Ptr<DeferredContext> aContext, GFSDK_SSAO_Context_D3D11* aAOContext);

		void ForwardPass(Ptr<DeferredContext> aContext);
		void ParticlePass(Ptr<DeferredContext> aContext);
		void PostProcessPass(Ptr<DeferredContext> aContext);
		void UIPass(Ptr<DeferredContext> aContext);
		void Flush(Ptr<DeferredContext> aContext);
	private:
		Ref<Texture2D> GetEnvironmentTexture();

		void DynamicSkyBox(Ptr<DeferredContext> aContext);
		void StaticSkyBox(Ptr<DeferredContext> aContext);

		// Volumetric Fog
		void ParticipatingMediaPass(Ptr<DeferredContext> aContext);
		void LightIntegrationPass(Ptr<DeferredContext> aContext);

		// UI
		void TextPass(Ptr<DeferredContext> aContext);
		void SpritePass(Ptr<DeferredContext> aContext);
		void LinePass(Ptr<DeferredContext> aContext);

		void FPSOverlapPass(Ptr<DeferredContext> aContext);
		void CombineMainAndFPSPass(Ptr<DeferredContext> aContext);


		Cache* myCache;
		uint32_t myID;
		bool myShouldFlush;
		size_t myFrameIndex = 0;
		size_t myForwardIndirectOffset = 0;
		size_t myOutlineIndirectOffset = 0;
	};
}