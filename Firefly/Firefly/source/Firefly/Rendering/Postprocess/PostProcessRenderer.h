#pragma once
#include "Firefly/Core/Core.h"
namespace Firefly
{
	struct Cache;
	class DeferredContext;
	class PostProcessRenderer
	{
	public:
		PostProcessRenderer(Cache* aCache);

		void RunPostProcessing(Ptr<DeferredContext> aContext);

	private:
		void VignettePass(Ptr<DeferredContext> aContext);
		void TonemappingPass(Ptr<DeferredContext> aContext);
		void ApplyOutlinePass(Ptr<DeferredContext> aContext);
		void DebandingPass(Ptr<DeferredContext> aContext);
		void FXAAPass(Ptr<DeferredContext> aContext);
		void FogPass(Ptr<DeferredContext> aContext);
		void BloomPass(Ptr<DeferredContext> aContext);

		void RunCustomPasses(Ptr<DeferredContext> aContext);

		Cache* myCache;
	};
}