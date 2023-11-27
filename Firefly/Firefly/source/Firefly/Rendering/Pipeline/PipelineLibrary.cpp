#include "FFpch.h"
#include "PipelineLibrary.h"
namespace Firefly
{
	void PipelineLibrary::Add(GraphicsPipelineInfo& aInfo)
	{
		GraphicsPipeline pipeline;
		pipeline.Initialize(aInfo);
		myPipelineCache[pipeline.GetHash()] = pipeline;
		if (aInfo.IsDeferred)
		{
			myDeferredPipelineLUT[pipeline.GetHash()] = pipeline.GetHash();
		}
	}
	bool PipelineLibrary::IsDeferredPipeline(const size_t& aID)
	{
		return myDeferredPipelineLUT.contains(aID);
	}
	GraphicsPipeline& PipelineLibrary::Get(const size_t & aHash)
	{
		if (!myPipelineCache.contains(aHash))
		{
			LOGERROR("Need to add pipeline before it can be used");
		}
		return myPipelineCache[aHash];
	}
	void PipelineLibrary::Remove(const size_t& aHash)
	{
		if (!myPipelineCache.contains(aHash))
		{
			return;
		}
		myPipelineCache.erase(aHash);
	}
}