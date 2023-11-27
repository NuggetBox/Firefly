#pragma once
#include "Firefly/Rendering/RenderCommands.h"


namespace Firefly
{
	namespace LOD
	{
		enum lod : int32_t
		{
			DontOverride = -1, // This is used for more readable interface for CollectDrawcallsFromList in scene renderer.
			Ultra = 0,
			High = 1,
			Mid = 2,
			Low = 3,
		};


		enum Ranges
		{
			UltraRange = 0,
			HighRange = 1000,
			MidRange = 10000,
			LowRange = 20000,
		};
	}

	struct MeshBatch
	{
		MeshSubmitInfo SubmitInfo;
		size_t BoneOffset;
		size_t InstanceOffset;
		size_t BatchSize;
	};

	struct Cache
	{
		bool IsSubScene = true; 

		std::vector<MeshBatch> MeshBatches;

		std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> DrawBatchIndexes; // <Draw enum, std::pair<startOffset, endOffset>>

		std::array<Ptr<MaterialAsset>, 4> CustomPostprocessPasses;

		std::vector<MeshSubmitInfo> DeferredCommands;
		std::vector<MeshSubmitInfo> ForwardCommands;
		std::vector<MeshSubmitInfo> OutlineCommands;
		std::vector<MeshSubmitInfo> DecalCommands;

		Ref<IndirectBuffer> IndirectCallsBuffer;
		Ref<IndirectBuffer> ParticleDrawCallsBuffer;

		Ref<StructuredBuffer> InstanceGPUBuffer;
		Ref<StructuredBuffer> PrepassInstanceBuffer;
		Ref<StructuredBuffer> InstanceBoneGPUBuffer;

		std::vector<ParticleEmitterCommand> ParticleEmitterCommands;
		ConstantBuffer<ParticleBufferData> ParticleBuffer;

		Ref<Camera> CurrentCamera;
		Ref<Framebuffer> DepthPrePass;
		Ref<Framebuffer> DeferredGBuffer;
		Ref<Framebuffer> ForwardGBuffer;
		Ref<Framebuffer> FPSDepthBuffer;
		Ref<Framebuffer> RendererFrameBuffer;
		Ref<Framebuffer> DirectionalShadowBuffer;
		Ref<Texture2D> DeferredUAVTexture;

		//Atmospheric sky
		Ref<Framebuffer> TransmittanceLUT;
		Ref<Framebuffer> CameraVolume;
		Ref<Framebuffer> Atmosphere;
		Ref<Framebuffer> FinalAtmosphere;
		AtmosphereParameters AtmosphereParams;
		SkyAtmosphereConstantBufferStructure SkyAtmosphereData;
		ConstantBuffer<SkyAtmosphereConstantBufferStructure> SkyAtmosphereBuffer;
		HelperValues SkyHelperValues;
		ConstantBuffer<HelperValues> SkyHelperValuesBuffer;
		Ref<Texture2D> MultiScatterTexture;

		// PostProcess
		Ref<Framebuffer> ToneMappingBuffer;
		Ref<Framebuffer> FogBuffer;
		Ref<Framebuffer> OcclusionFB;
		Ref<Framebuffer> HorzontalBlurFB;
		Ref<Framebuffer> VerticalBlurFB;
		Ref<Framebuffer> VignetteFB;
		Ref<Framebuffer> OutlineBufferFB;
		Ref<Framebuffer> FXAAFB;
		Ref<Framebuffer> VolumeResultFB;
		Ref<Framebuffer> CurrentPostProcessResultFB;
		Ref<Texture3D> LightInjections[2];
		Ref<Texture3D> RayMarchVolume;
		Ref<Framebuffer> SSGI;


		std::array<Ref<Framebuffer>, 8> SpotLightShadowFB;
		std::array<Ref<Framebuffer>, 8> PointLightShadowFB;

		std::array<Ref<Framebuffer>, 4> CustomPostProcess;
		
		Tonemapping ToneMapping;

		// this is for the jump flood alg (AKA JFA). 
		Ref<Framebuffer> OutlineFB;
		Ref<Framebuffer> JFAPrepass_FB;
		Ref<Framebuffer> JFAFinal_FB;

		BloomPackage BloomPack;

		PostProcessInfo PostProcessSpecs;

		Ref<Texture2D> NoiseTexture;

		EnvironmentData EnvironmentInfo;

		UndefinedConstBuffer MaterialBuffer;

		CameraData CameraBuffData;
		ConstantBuffer<CameraData> Camerabuffer;

		ModelData ModelBuffData;
		ConstantBuffer<ModelData> ModelBuffer;

		TimeData TimeBuffData;
		ConstantBuffer<TimeData> TimeBuffer;

		DirLightData DirLightBuffData;
		ConstantBuffer<DirLightData> DirLightBuffer;
		uint32_t DirLightIterator = 0;

		PointLightData PointLightBuffData;
		ConstantBuffer<PointLightData> PointLightBuffer;
		uint32_t PointLightIterator = 0;

		RenderPassData RenderPassBuffData;
		ConstantBuffer<RenderPassData> RenderPassBuffer;

		PostProcessData PostProcessBuffData;
		ConstantBuffer<PostProcessData> PostProcessBuffer;

		SpotLightData SpotLightBuffData;
		ConstantBuffer<SpotLightData> SpotLightBuffer;
		uint32_t SpotLightIterator = 0;

		SpriteBatch SpriteBatchSpecs;
		Utils::Queue<SpriteInfo> SpriteDrawQueue;

		Ref<VertexBuffer> customVBSprite;
		Ref<IndexBuffer> customIBSprite;
		Utils::Queue<SpriteInfo> NoDepthSpriteDrawQueue;

		SpriteBatch TextBatchSpecs;
		Utils::Queue<TextInfo> TextDrawQueue;
		Utils::Queue<TextInfo> NoDepthTextDrawQueue;

		LineBatch LineCacheSpecs;
		LineBatch LineCache2DSpecs;

		BillBoardBatch BillBoardBatchSpecs;

		std::vector<TraceOfFB> TracedFramebuffers;

		std::vector<std::function<void()>> SceneDrawQueue;


		bool DrawGrid = true;
		bool DrawDebugLines = true;
		bool DrawOutlines = true;
		bool PostProcessIsSubmitted = false;
		size_t triCount = 0;
		size_t DrawCount = 0;
		size_t LineCount = 0;
	};
	// this is to buffer the 2 caches.
	struct CachePackage
	{
		Ref<Cache> cache1;
		Ref<Cache> cache2;
	};
}