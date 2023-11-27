#pragma once
#include <Firefly/Rendering/Pipeline/GraphicsPipeline.h>
namespace Firefly
{
	class PipelineLibrary
	{
	public:
		static void Add(GraphicsPipelineInfo& aInfo);
		static bool IsDeferredPipeline(const size_t& aID);
		static GraphicsPipeline& Get(const size_t& aHash);
		static void Remove(const size_t& aHash);
	private:
		static inline std::unordered_map<size_t, GraphicsPipeline> myPipelineCache;
		static inline std::map<size_t, size_t> myDeferredPipelineLUT;
	};
}