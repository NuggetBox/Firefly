#include "FFpch.h"
#include "Renderer.h"
#include "Framebuffer.h"

#include <future>
#include <vector>

#include <Optick/optick.h>

#include <HBAOPlus/GFSDK_SSAO.h>

#include <Utils/InputHandler.h>
#include <Utils/Queue.hpp>
#include <Utils/Timer.h>
#include <Utils/Math/Random.hpp>

#include "Firefly/Core/DXHelpers.h"

#include "Firefly/Rendering/RenderCache.h"
#include "Firefly/Rendering/RenderingDefines.h"
#include "Firefly/Rendering/SceneRenderer.h"
#include "Firefly/Rendering/Camera/Camera.h"

#include "Firefly/Rendering/Shader/Shader.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"

#include "Firefly/Rendering/AtmosphericSky/SkyAtmosphereData.h"
#include "Firefly/Rendering/Pipeline/PipelineLibrary.h"
#include "Firefly/Rendering/Postprocess/PostprocessManager.h"
#include "Firefly/Rendering/Postprocess/PostProcessUtils.h"
#include "Firefly/Rendering/Shadows/CascadedShadows.h"
#include "Firefly/Rendering/ThreadSafe/DeferredContext.h"

#include "Firefly/Rendering/Buffer/ConstantBuffer.hpp"
#include "Firefly/Rendering/Buffer/ConstantBuffers.h"
#include "Firefly/Rendering/Buffer/StructuredBuffer.h"
#include "Firefly/Rendering/Buffer/UndefinedConstBuffer.h"

#include "Firefly/Asset/ResourceCache.h"

#include "Firefly/Asset/Animation.h"

#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/Mesh/Mesh.h"

#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Asset/Texture/Texture3D.h"

#include "Firefly/Rendering/ParticleSystem/ParticleEmitter.h"

// this is the current render cache that you should write to on the main thread.
#define WRITE_CACHE_MAIN GetCacheFromID(localCurrentRenderSceneId[GetFrameIndex(1)], 1)

// this is the current render cache that you should write to on the render thread.
#define WRITE_CACHE_THREAD GetCacheFromID(localCurrentRenderSceneId[GetFrameIndex()], 1)

// this is the current render cache that you should read from on the main thread.
#define READ_CACHE_MAIN GetCacheFromID(localCurrentRenderSceneId[GetFrameIndex(1)])

// this is the current render cache that you should read from on the render thread.
#define READ_CACHE_THREAD GetCacheFromID(localCurrentRenderSceneId[GetFrameIndex()])

#define CASTDOUBLE(X) static_cast<double>(X)

namespace Firefly
{
	constexpr uint32_t InvalidSceneID = ~0;
	constexpr uint32_t globalMaxQuads = 20000;
	constexpr uint32_t globalMaxVertices = globalMaxQuads * 4;
	constexpr uint32_t globalMaxIndices = globalMaxQuads * 6;
#ifndef FF_SHIPIT
	constexpr bool DrawDiffrentScenes = true;
	constexpr bool DrawDebugLines = true;
#else
	constexpr bool DrawDiffrentScenes = false;
	constexpr bool DrawDebugLines = false;

#endif
	static GFSDK_SSAO_CustomHeap localCustomHeap;
	static GFSDK_SSAO_Context_D3D11* localAOContext;
	static Ref<CachePackage> localCache = {};
	static RenderSceneId localOneshotID = { InvalidSceneID };
	static Ref<Texture2D> localTempEnvironmentTexture = nullptr; // TODO_Niklas: this is temp Environment until I find a better solution to the problem. 
	static bool localDrawOneshot = true;
	static std::array<size_t, 2> localFrameCount = {};
	static size_t localGlobalFrameCount = 0;
	static Ref<MaterialAsset> localDefaultMat = nullptr;
	static Ref<Mesh> localDefaultCube = nullptr;
	static RenderSceneId localCurrentRenderSceneId[2]; // This is might not be thread Safe idk.
	static std::unordered_map<RenderSceneId, CachePackage> localCacheRegistry;
	static Ref<MaterialAsset> localCurrentlyBoundMaterial;
	static std::future<void> localRenderThreadFuture;


	static std::array<LineVertex, MAX_LINE_BATCH_COUNT> localLineVertexBatchBuild;
	static uint32_t localLineVertexInterator = 0;
	static std::array<BillBoardVertex, MAX_BILLBOARD_BATCH_COUNT> localBillboardBatchBuild;
	static uint32_t localBillboardInterator = 0;

	// These save down the instance batches that later will be moved over to GPU memory. SEE "CompileMeshBatches" to see usages.
	static std::vector<InstanceData> localInstanceProperties;
	static std::vector<InstanceBoneData> localBonesProperties;
	static std::vector<PerMeshMaterialData> localPerMeshMaterialDataProperties;
	static std::vector<IndirectArgs> localIndirectDrawCallsProperties;
	static std::array<Ref<Texture2D>, 16> localBlueNoise;

	thread_local std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> localDrawDataBatches;

	void ClearBatchProperties()
	{
		localIndirectDrawCallsProperties.clear();
		localInstanceProperties.clear();
		localBonesProperties.clear();
	}

	void ExtractLodLevel(Firefly::Cache* cache, Firefly::MeshSubmitInfo* cmd, bool& retflag)
	{
		retflag = true;
		if (!cache->CurrentCamera)
		{
			cmd->lodID = LOD::Low;
			return;
		}

		auto cameraPosition = cache->CurrentCamera->GetTransform().GetPosition();

		auto length = (cmd->BoundingSphere.GetOrigin() - cameraPosition).Length() - cmd->BoundingSphere.GetRadius();

		if (length > LOD::LowRange)
		{
			cmd->lodID = LOD::Low;
			return;
		}

		if (length > LOD::MidRange)
		{
			cmd->lodID = LOD::Mid;
			return;
		}

		if (length > LOD::HighRange)
		{
			cmd->lodID = LOD::High;
			return;
		}

		if (length > LOD::UltraRange)
		{
			cmd->lodID = LOD::Ultra;
			return;
		}
		retflag = false;
	}

	static size_t GetFrameIndex(const int32_t aOffset = 0)
	{
		return (localGlobalFrameCount + aOffset) % 2;
	}

	void Renderer::SubmitActiveCamera(Ref<Camera> camera)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		cache->CurrentCamera = camera;
	}

	Ref<Camera> Renderer::GetActiveCamera()
	{
		return READ_CACHE_MAIN->CurrentCamera;
	}

	Ref<Framebuffer> Renderer::GetFrameBuffer()
	{
		return GetCacheFromID(0)->RendererFrameBuffer;
	}

	Ref<Framebuffer> Renderer::GetSceneFrameBuffer(RenderSceneId aId)
	{
		return GetCacheFromID(aId)->RendererFrameBuffer;
	}

	Ref<MaterialAsset> Renderer::GetDefaultMaterial()
	{
		return localDefaultMat;
	}

	// ------------- Submit functions --------------- //

	void Renderer::Submit(MeshSubmitInfo& modelInfo)
	{
		Cache* cache = WRITE_CACHE_MAIN;

		if (localDefaultMat == nullptr)
		{
			localDefaultMat = ResourceCache::GetAsset<MaterialAsset>("Default", true);

		}

		MeshSubmitInfo* cmd;
		Ref<MaterialAsset> material;
		if (modelInfo.Outline)
		{
			auto& outlineCmd = cache->OutlineCommands.emplace_back(modelInfo.Transform, modelInfo.IsAnimation);

			if (modelInfo.IsAnimation)
			{
				memcpy(&outlineCmd.BoneTransforms[0], &modelInfo.BoneTransforms[0], sizeof(Utils::Matrix4f) * MAX_BONE_COUNT);
			}

			outlineCmd.Material = material;
			outlineCmd.Mesh = modelInfo.Mesh;
			outlineCmd.EntityID = modelInfo.EntityID;
			outlineCmd.CreationTime = modelInfo.CreationTime;
			outlineCmd.CastShadows = modelInfo.CastShadows;
			bool retflag;
			ExtractLodLevel(cache, &outlineCmd, retflag);
		}

		if (modelInfo.Material)
		{
			if (modelInfo.Material->IsLoaded())
			{
				material = modelInfo.Material;

				if (!CheckIfShaderIsDeferred(material->GetInfo().PipelineID))
				{
					cmd = &cache->ForwardCommands.emplace_back(modelInfo.Transform, modelInfo.IsAnimation);
				}
				else
				{
					cmd = &cache->DeferredCommands.emplace_back(modelInfo.Transform, modelInfo.IsAnimation);
				}
			}
			else
			{
				material = localDefaultMat;
				cmd = &cache->DeferredCommands.emplace_back(modelInfo.Transform, modelInfo.IsAnimation);
			}
		}
		else
		{
			material = localDefaultMat;
			cmd = &cache->DeferredCommands.emplace_back(modelInfo.Transform, modelInfo.IsAnimation);
		}

		if (modelInfo.IsAnimation)
		{
			memcpy(&cmd->BoneTransforms[0], &modelInfo.BoneTransforms[0], sizeof(Utils::Matrix4f) * MAX_BONE_COUNT);
			if (modelInfo.AutomaticCleanup)
			{
				modelInfo.CleanUp();
			}
		}

		cmd->Material = material;
		cmd->Mesh = modelInfo.Mesh;
		cmd->EntityID = modelInfo.EntityID;
		cmd->CreationTime = modelInfo.CreationTime;
		cmd->CastShadows = modelInfo.CastShadows;
		cmd->BoundingSphere = modelInfo.BoundingSphere;

		bool retflag;
		ExtractLodLevel(cache, cmd, retflag);
	}

	void Renderer::Submit(const DirLightPacket& aLight, const ShadowResolutions& aShadowRes)
	{
		uint32_t res = GetResFromEnum(aShadowRes);
		Cache* cache = WRITE_CACHE_MAIN;

		if (cache->DirLightIterator >= FF_MAX_DIRLIGHTS)
		{
			return;
		}

		cache->DirectionalShadowBuffer->Resize({ res, res });

		cache->DirLightBuffData.DirectionLightPacket[cache->DirLightIterator++] = aLight;
	}

	void Renderer::Submit(Tonemapping aToneMappingSettings)
	{
		WRITE_CACHE_MAIN->ToneMapping = aToneMappingSettings;
	}

	void Renderer::Submit(const PointLightPacket& aLight)
	{
		Cache* cache = WRITE_CACHE_MAIN;

		if (cache->PointLightIterator >= FF_MAX_POINTLIGHTS)
		{
			return;
		}

		cache->PointLightBuffData.PointLightPackets[cache->PointLightIterator] = aLight;
		if (cache->PointLightBuffData.Count.y < 8)
		{
			cache->PointLightBuffData.PointLightPackets[cache->PointLightIterator].PointlightCustomData.z = cache->PointLightBuffData.Count.y;
			cache->PointLightBuffData.Count.y++;
		}
		cache->PointLightIterator++;
	}

	void Renderer::Submit(const SpotLightPacket& aLight)
	{
		Cache* cache = WRITE_CACHE_MAIN;

		if (cache->SpotLightIterator >= FF_MAX_SPOTLIGHTS)
		{
			return;
		}

		cache->SpotLightBuffData.SpotLightPackets[cache->SpotLightIterator] = aLight;
		if (cache->SpotLightBuffData.Count.y < 8)
		{
			cache->SpotLightBuffData.SpotLightPackets[cache->SpotLightIterator].Direction.w = cache->SpotLightBuffData.Count.y;
			cache->SpotLightBuffData.Count.y++;
		}
		cache->SpotLightIterator++;
	}

	void Renderer::Submit(const VolumetricFogInfo& aFogInfo)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		cache->RenderPassBuffData.IsVolumetricFogActive_Pad.x = static_cast<float>(aFogInfo.Enable);

		cache->RenderPassBuffData.EnvironmentFogColorIntensity.x = aFogInfo.Color.x;
		cache->RenderPassBuffData.EnvironmentFogColorIntensity.y = aFogInfo.Color.y;
		cache->RenderPassBuffData.EnvironmentFogColorIntensity.z = aFogInfo.Color.z;
		cache->RenderPassBuffData.EnvironmentFogColorIntensity.w = aFogInfo.Intensity;
		cache->RenderPassBuffData.godraysColorIntensity.x = aFogInfo.ColorGodRay.x;
		cache->RenderPassBuffData.godraysColorIntensity.y = aFogInfo.ColorGodRay.y;
		cache->RenderPassBuffData.godraysColorIntensity.z = aFogInfo.ColorGodRay.z;
		cache->RenderPassBuffData.godraysColorIntensity.w = aFogInfo.IntensityGodray;
		cache->RenderPassBuffData.fogSettings.x = aFogInfo.Density;
		cache->RenderPassBuffData.fogSettings.y = aFogInfo.Phase;
		cache->RenderPassBuffData.fogSettings.z = aFogInfo.DepthPow;
		cache->RenderPassBuffData.fogSettings.w = aFogInfo.resurve;
	}

	void Renderer::Submit(EnvironmentData& aEnvironmentData)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		cache->EnvironmentInfo.EnvironmentMap = aEnvironmentData.EnvironmentMap;
		cache->EnvironmentInfo.SkyBoxMap = aEnvironmentData.SkyBoxMap ? aEnvironmentData.SkyBoxMap : aEnvironmentData.EnvironmentMap;
		cache->EnvironmentInfo.Intensity = aEnvironmentData.Intensity;
		cache->EnvironmentInfo.SkyLightIntensity = aEnvironmentData.SkyLightIntensity;
		cache->EnvironmentInfo.GroundColor = aEnvironmentData.GroundColor;
		cache->EnvironmentInfo.SunRadius = aEnvironmentData.SunRadius;
		cache->RenderPassBuffData.EnvironmentMip_padded3.x = aEnvironmentData.EnvironmentMipMap;
	}

	void Renderer::Submit(const ParticleEmitterCommand& anEmitterCommand)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		cache->ParticleEmitterCommands.push_back(anEmitterCommand);
	}

	void Renderer::Submit(const Sprite2DInfo& info)
	{
		Sprite3DInfo dInfo{};
		dInfo.Position = { info.Position.x,info.Position.y, 0 };
		dInfo.Rotation = info.Rotation;
		dInfo.Color = info.Color;
		dInfo.Size = info.Size;
		dInfo.Texture = info.Texture;
		dInfo.Is3D = false;
		dInfo.UV[0] = info.UV[0];
		dInfo.UV[1] = info.UV[1];
		dInfo.UV[2] = info.UV[2];
		dInfo.UV[3] = info.UV[3];
		dInfo.ShaderKey = info.ShaderKey;
		dInfo.CustomValues = info.CustomValues;
		dInfo.IgnoreDepth = info.IgnoreDepth;
		Submit(dInfo);
	}

	void Renderer::Submit(const Sprite3DInfo& info)
	{
		SpriteInfo drawInfo{};
		Utils::Matrix4f translate;
		translate(4, 1) = info.Position.x;
		translate(4, 2) = info.Position.y;
		translate(4, 3) = info.Position.z;
		Utils::Matrix4f trans = (Utils::Matrix4f::CreateScaleMatrix({ info.Size.x, info.Size.y, 1.0f }) * Utils::Matrix4f::CreateRotationMatrix(info.Rotation) * translate);
		drawInfo.Transform = trans;
		drawInfo.Color = info.Color;
		drawInfo.Texture = (info.Texture);
		drawInfo.UV[0] = info.UV[0];
		drawInfo.UV[1] = info.UV[1];
		drawInfo.UV[2] = info.UV[2];
		drawInfo.UV[3] = info.UV[3];
		drawInfo.Is3D = info.Is3D;
		drawInfo.ShaderKey = info.ShaderKey;
		drawInfo.CustomValues = info.CustomValues;
		drawInfo.IgnoreDepth = info.IgnoreDepth;

		Cache* cache = WRITE_CACHE_MAIN;
		drawInfo.IgnoreDepth == false ? cache->SpriteDrawQueue.Enqueue(drawInfo) : cache->NoDepthSpriteDrawQueue.Enqueue(drawInfo);
	}

	void Renderer::Submit(TextInfo& aInfo)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		aInfo.IgnoreDepth == false ? cache->TextDrawQueue.Enqueue(aInfo) : cache->NoDepthTextDrawQueue.Enqueue(aInfo);
	}

	void Renderer::Submit(const PostProcessInfo& aInfo)
	{
		WRITE_CACHE_MAIN->PostProcessSpecs = aInfo;
		WRITE_CACHE_MAIN->PostProcessIsSubmitted = true;
	}

	void Renderer::Submit(const BillboardInfo& aInfo)
	{
		//TODO: Put billboard rendering back in
		//WRITE_CACHE_MAIN->BillBoardBatchSpecs.BillboardContainer.emplace_back(aInfo);
	}

	void Renderer::Submit(const CustomPostprocessInfo& aInfo)
	{
		WRITE_CACHE_MAIN->CustomPostprocessPasses[static_cast<uint32_t>(aInfo.Passorder)] = aInfo.Material;
	}

	void Renderer::Submit(const DecalSubmitInfo& aInfo)
	{
		if (!localDefaultCube)
		{
			localDefaultCube = ResourceCache::GetAsset<Mesh>("Cube");
		}

		auto& decalCommand = WRITE_CACHE_MAIN->DecalCommands.emplace_back();
		decalCommand.BoneTransforms = nullptr;
		decalCommand.Material = aInfo.DecalMaterial;
		decalCommand.Mesh = &localDefaultCube->GetSubMeshes()[0];
		decalCommand.EntityID = aInfo.EntityId;
		decalCommand.CreationTime = 0;
		decalCommand.CastShadows = false;
		decalCommand.BoundingSphere = localDefaultCube->GetBoundingSphere();
		decalCommand.lodID = LOD::Ultra;
		decalCommand.Outline = false;
		decalCommand.Transform = aInfo.Matrix;
	}

	void Renderer::Submit(const BloomInfo& aInfo)
	{
		Cache* cache = WRITE_CACHE_MAIN;
		cache->RenderPassBuffData.BloomThreshhold.x = aInfo.BloomThreshhold;
	}

	// ------------- Submit debug functions --------------- //
	void Renderer::SubmitDebugLine(const Utils::Vector3f& aStart, const Utils::Vector3f& aEnd, const Utils::Vector4f& aColor, bool aIs2D)
	{
		if (!DrawDebugLines) return;

		Cache* cache = WRITE_CACHE_MAIN;

		auto& lineCache = aIs2D ? cache->LineCache2DSpecs : cache->LineCacheSpecs;

		if (cache->DrawDebugLines == false)
		{
			return;
		}

		bool startIsVisable = true;
		bool endIsVisable = true;
		if (aIs2D == false && cache->CurrentCamera)
		{
			Utils::Sphere<float> sphere;
			sphere.InitWithCenterAndRadius({ aStart.x, aStart.y, aStart.z }, FLT_MAX);
			startIsVisable = cache->CurrentCamera->MeshIsVisible(sphere);

			sphere.InitWithCenterAndRadius({ aEnd.x, aEnd.y, aEnd.z }, FLT_MAX);
			endIsVisable = cache->CurrentCamera->MeshIsVisible(sphere);

		}

		if (startIsVisable || endIsVisable)
		{
			globalRendererStats.LineCount++;
			lineCache.Vertices.push_back({ {aStart.x, aStart.y, aStart.z, 1}, aColor });
			lineCache.Vertices.push_back({ {aEnd.x, aEnd.y, aEnd.z, 1}, aColor });
			cache->LineCacheSpecs.Itreator += 2;
		}
	}

	void Renderer::SubmitDebugCircle(const Utils::Vector3f& aCenter, float aRadius, int aNumLines, const Utils::Vector3f& aRotation, const Utils::Vector4f& aColor)
	{
		float rotationStep = 2 * PI / static_cast<float>(aNumLines);

		for (int i = 0; i < aNumLines; ++i)
		{
			float x1 = cos(rotationStep * i) * aRadius;
			float y1 = sin(rotationStep * i) * aRadius;
			float x2 = cos(rotationStep * (i + 1)) * aRadius;
			float y2 = sin(rotationStep * (i + 1)) * aRadius;

			Vector4f start = Utils::Vector4f(x1, 0, y1, 1);
			Vector4f end = Utils::Vector4f(x2, 0, y2, 1);

			Utils::Matrix4f rotation = Utils::Matrix4f::CreateRotationMatrix(aRotation);

			start = start * rotation;
			end = end * rotation;

			SubmitDebugLine({ start.x + aCenter.x, start.y + aCenter.y, start.z + aCenter.z }, { end.x + aCenter.x, end.y + aCenter.y, end.z + aCenter.z }, aColor);
		}
	}

	void Renderer::SubmitDebugCircle(const Utils::Vector3f& aCenter, float aRadius, int aNumLines, const Utils::Quaternion& aRotation, const Utils::Vector4f& aColor)
	{
		float rotationStep = 2 * PI / static_cast<float>(aNumLines);

		for (int i = 0; i < aNumLines; ++i)
		{
			float x1 = cos(rotationStep * i) * aRadius;
			float y1 = sin(rotationStep * i) * aRadius;
			float x2 = cos(rotationStep * (i + 1)) * aRadius;
			float y2 = sin(rotationStep * (i + 1)) * aRadius;

			Vector4f start = Utils::Vector4f(x1, 0, y1, 1);
			Vector4f end = Utils::Vector4f(x2, 0, y2, 1);

			Utils::Matrix4f rotation = Utils::Matrix4f::CreateRotationMatrix(aRotation);

			start = start * rotation;
			end = end * rotation;

			SubmitDebugLine({ start.x + aCenter.x, start.y + aCenter.y, start.z + aCenter.z }, { end.x + aCenter.x, end.y + aCenter.y, end.z + aCenter.z }, aColor);
		}
	}

	void Renderer::SubmitDebugSphere(const Utils::Vector3f& aCenter, float aRadius, int aNumLines, const Utils::Vector4f& aColor)
	{
		SubmitDebugCircle(aCenter, aRadius, aNumLines, { 0, 0, 0 }, aColor);
		SubmitDebugCircle(aCenter, aRadius, aNumLines, { 90, 0, 0 }, aColor);
		SubmitDebugCircle(aCenter, aRadius, aNumLines, { 90, 90, 0 }, aColor);
	}

	void Renderer::SubmitDebugCube(const Utils::Vector3f& aCenter, float aSize, const Utils::Vector4f& aColor)
	{
		SubmitDebugCuboid(aCenter, { aSize, aSize, aSize }, aColor);
	}

	void Renderer::SubmitDebugCube(const Utils::Transform& aOriginTransform, const Utils::Vector3f& aOffset, float aSize, const Utils::Vector4f& aColor)
	{
		SubmitDebugCuboid(aOriginTransform, aOffset, { aSize, aSize, aSize }, aColor);
	}

	void Renderer::SubmitDebugCuboid(const Utils::Vector3f& aCenter, const Utils::Vector3f& aSize, const Utils::Vector4f& aColor)
	{
		const Utils::Vector3f halfSize = aSize * 0.5f;

		const Utils::Vector3f leftDownBack = { aCenter.x - halfSize.x, aCenter.y - halfSize.y, aCenter.z - halfSize.z };
		const Utils::Vector3f leftDownForward = { aCenter.x - halfSize.x, aCenter.y - halfSize.y, aCenter.z + halfSize.z };
		const Utils::Vector3f leftUpBack = { aCenter.x - halfSize.x, aCenter.y + halfSize.y, aCenter.z - halfSize.z };
		const Utils::Vector3f leftUpForward = { aCenter.x - halfSize.x, aCenter.y + halfSize.y, aCenter.z + halfSize.z };
		const Utils::Vector3f rightUpBack = { aCenter.x + halfSize.x, aCenter.y + halfSize.y, aCenter.z - halfSize.z };
		const Utils::Vector3f rightUpForward = { aCenter.x + halfSize.x, aCenter.y + halfSize.y, aCenter.z + halfSize.z };
		const Utils::Vector3f rightDownBack = { aCenter.x + halfSize.x, aCenter.y - halfSize.y, aCenter.z - halfSize.z };
		const Utils::Vector3f rightDownForward = { aCenter.x + halfSize.x, aCenter.y - halfSize.y, aCenter.z + halfSize.z };

		SubmitDebugLine(leftDownBack, leftDownForward, aColor);
		SubmitDebugLine(leftDownBack, rightDownBack, aColor);
		SubmitDebugLine(leftDownBack, leftUpBack, aColor);
		SubmitDebugLine(rightDownForward, leftDownForward, aColor);
		SubmitDebugLine(rightDownForward, rightDownBack, aColor);
		SubmitDebugLine(rightDownForward, rightUpForward, aColor);
		SubmitDebugLine(rightDownBack, rightUpBack, aColor);
		SubmitDebugLine(leftDownForward, leftUpForward, aColor);
		SubmitDebugLine(leftUpForward, rightUpForward, aColor);
		SubmitDebugLine(leftUpBack, rightUpBack, aColor);
		SubmitDebugLine(leftUpForward, leftUpBack, aColor);
		SubmitDebugLine(rightUpBack, rightUpForward, aColor);
	}

	void Renderer::SubmitDebugCuboid(const Utils::Transform& aOriginTransform, const Utils::Vector3f& aOffset, const Utils::Vector3f& aSize, const Utils::Vector4f& aColor)
	{
		const Utils::Vector3f halfSize = aSize * 0.5f;

		Utils::Transform posRot = aOriginTransform;
		posRot.SetScale(1, 1, 1);
		const Utils::Matrix4f matrix = posRot.GetMatrix();

		Utils::Vector3f leftDownBack = { aOffset.x - halfSize.x, aOffset.y - halfSize.y, aOffset.z - halfSize.z };
		Utils::Vector3f leftDownForward = { aOffset.x - halfSize.x, aOffset.y - halfSize.y, aOffset.z + halfSize.z };
		Utils::Vector3f leftUpBack = { aOffset.x - halfSize.x, aOffset.y + halfSize.y, aOffset.z - halfSize.z };
		Utils::Vector3f leftUpForward = { aOffset.x - halfSize.x, aOffset.y + halfSize.y, aOffset.z + halfSize.z };
		Utils::Vector3f rightUpBack = { aOffset.x + halfSize.x, aOffset.y + halfSize.y, aOffset.z - halfSize.z };
		Utils::Vector3f rightUpForward = { aOffset.x + halfSize.x, aOffset.y + halfSize.y, aOffset.z + halfSize.z };
		Utils::Vector3f rightDownBack = { aOffset.x + halfSize.x, aOffset.y - halfSize.y, aOffset.z - halfSize.z };
		Utils::Vector3f rightDownForward = { aOffset.x + halfSize.x, aOffset.y - halfSize.y, aOffset.z + halfSize.z };

		leftDownBack = Utils::Vec4ToVec3(Utils::Vec3ToVec4(leftDownBack) * matrix);
		leftDownForward = Utils::Vec4ToVec3(Utils::Vec3ToVec4(leftDownForward) * matrix);
		leftUpBack = Utils::Vec4ToVec3(Utils::Vec3ToVec4(leftUpBack) * matrix);
		leftUpForward = Utils::Vec4ToVec3(Utils::Vec3ToVec4(leftUpForward) * matrix);
		rightUpBack = Utils::Vec4ToVec3(Utils::Vec3ToVec4(rightUpBack) * matrix);
		rightUpForward = Utils::Vec4ToVec3(Utils::Vec3ToVec4(rightUpForward) * matrix);
		rightDownBack = Utils::Vec4ToVec3(Utils::Vec3ToVec4(rightDownBack) * matrix);
		rightDownForward = Utils::Vec4ToVec3(Utils::Vec3ToVec4(rightDownForward) * matrix);

		SubmitDebugLine(leftDownBack, leftDownForward, aColor);
		SubmitDebugLine(leftDownBack, rightDownBack, aColor);
		SubmitDebugLine(leftDownBack, leftUpBack, aColor);
		SubmitDebugLine(rightDownForward, leftDownForward, aColor);
		SubmitDebugLine(rightDownForward, rightDownBack, aColor);
		SubmitDebugLine(rightDownForward, rightUpForward, aColor);
		SubmitDebugLine(rightDownBack, rightUpBack, aColor);
		SubmitDebugLine(leftDownForward, leftUpForward, aColor);
		SubmitDebugLine(leftUpForward, rightUpForward, aColor);
		SubmitDebugLine(leftUpBack, rightUpBack, aColor);
		SubmitDebugLine(leftUpForward, leftUpBack, aColor);
		SubmitDebugLine(rightUpBack, rightUpForward, aColor);
	}

	void Renderer::SubmitDebugArrow(const Utils::Vector3f& aStart, const Utils::Vector3f& aEnd, const Utils::Vector4f& aColor, int aNumLines)
	{
		const float circleRadius = (aEnd - aStart).Length() / 10.0f;
		const auto quatRot = Utils::Quaternion::CreateLookRotation(aEnd - aStart) * Utils::Quaternion::CreateFromEulerAngles(90.0f, 0.0f, 0.0f);
		const auto circleCenter = aEnd + (aStart - aEnd) * 0.1f;

		SubmitDebugLine(aStart, aEnd, aColor);
		SubmitDebugCircle(circleCenter, circleRadius, aNumLines, quatRot, aColor);

		const float rotationStep = 2 * PI / static_cast<float>(aNumLines);

		for (int i = 0; i < aNumLines; ++i)
		{
			float x1 = cos(rotationStep * i) * circleRadius;
			float y1 = sin(rotationStep * i) * circleRadius;

			auto start = Utils::Vector4f(x1, 0, y1, 1);

			Utils::Matrix4f rotation = Utils::Matrix4f::CreateRotationMatrix(quatRot);

			start = start * rotation;

			SubmitDebugLine(Utils::Vec4ToVec3(start) + circleCenter, Utils::Vec4ToVec3(aEnd), aColor);
		}
	}

	// ------------- Helper functions --------------- //
	void Renderer::SetGridActive(bool aActive)
	{
		WRITE_CACHE_MAIN->DrawGrid = aActive;
		READ_CACHE_MAIN->DrawGrid = aActive;
	}

	bool Renderer::GetGridActive()
	{
		return WRITE_CACHE_MAIN->DrawGrid;
	}

	void Renderer::SetDebugLinesActive(bool aActive)
	{
		WRITE_CACHE_MAIN->DrawDebugLines = aActive;
		READ_CACHE_MAIN->DrawDebugLines = aActive;
	}

	void Renderer::SetOutlineActive(bool aActive)
	{
		WRITE_CACHE_MAIN->DrawOutlines = aActive;
		READ_CACHE_MAIN->DrawOutlines = aActive;
	}

	uint64_t Renderer::GetEntityFromScreenPos(int x, int y)
	{
		return WRITE_CACHE_MAIN->DepthPrePass->ReadPixel<uint64_t>(0, x, y);
	}

	uint64_t Renderer::GetEntityFromScreenPos(RenderSceneId aId, int aX, int aY)
	{
		return GetCacheFromID(aId, 1)->DepthPrePass->ReadPixel<uint64_t>(0, aX, aY);
	}

	std::array<Ref<Texture2D>, 16> Renderer::GetShadowMap(uint32_t aID)
	{
		return localBlueNoise;
	}

	// ------------- Helper functions --------------- //
	RenderSceneId Renderer::InitializeScene(bool aIsOneshot)
	{
		auto id = (RenderSceneId)Utils::RandomFloat(1.f, 9999999.f);
		if (aIsOneshot == false)
		{
			Ref<Cache> cache = CreateRef<Cache>();
			Ref<Cache> cache2 = CreateRef<Cache>();
			localCacheRegistry[id].cache1 = std::move(cache);
			localCacheRegistry[id].cache2 = std::move(cache2);
		}
		else
		{
			localOneshotID = id;
			Ref<Cache> cache = CreateRef<Cache>();
			localCacheRegistry[id].cache1 = std::move(cache);
			localCacheRegistry[id].cache2 = localCacheRegistry[id].cache1;
		}
		if (DrawDiffrentScenes)
		{
			InternalInitialize(id);
		}
		return id;
	}

	void Renderer::BeginScene(RenderSceneId aId)
	{
		if (aId == localOneshotID)
		{
			localDrawOneshot = true;
		}
		localCurrentRenderSceneId[GetFrameIndex(1)] = aId;
	}

	void Renderer::EndScene()
	{
		if (localCurrentRenderSceneId[GetFrameIndex(1)] == 0)
		{
			FF_ASSERT(false && L"End scene was called but there was no Begin scene before.");
		}
		RenderSceneId id = localCurrentRenderSceneId[GetFrameIndex(1)];
		if (DrawDiffrentScenes)
		{
			if (id != InvalidSceneID)
			{
				if (id != localOneshotID)
				{
					GetCacheFromID(0, 1)->SceneDrawQueue.emplace_back([id]() {
						localCurrentRenderSceneId[GetFrameIndex()] = id;
						DrawSceneWithID(id);
						localCurrentRenderSceneId[GetFrameIndex()] = 0;
						});
				}
			}


		}
		localCurrentRenderSceneId[GetFrameIndex(1)] = 0;
	}
	Ref<DeferredContext> localDeferredContext;

	void Renderer::Initialize()
	{
		localDefaultCube = ResourceCache::GetAsset<Mesh>("Cube");
		LOGINFO("Initializing the renderer...");
		localDeferredContext = CreateRef<DeferredContext>();
		localCache = CreateRef<CachePackage>();
		localCache->cache1 = CreateRef<Cache>();
		localCache->cache2 = CreateRef<Cache>();

		localCache->cache1->IsSubScene = false;
		localCache->cache2->IsSubScene = false;

		LOGINFO("Initalizing HBAO instance");
		InitalizeHBAO();
		LOGINFO("Loading shaders");
		InitializeShaders();
		LOGINFO("Initalizing main cache");
		InternalInitialize(0);
		localTempEnvironmentTexture = ResourceCache::GetAsset<Texture2D>("FireflyEngine/Defaults/skansen_cubemap.dds");

		LOGINFO("Generating Bluenoise textures");

		std::filesystem::path noisePath = "FireflyEngine/Defaults/NoiseTextures/HDR_L_";
		size_t i = 0;
		for (auto& texture : localBlueNoise)
		{
			auto noiseTexturePath = noisePath.string() + std::to_string(i) + ".dds";
			texture = ResourceCache::GetAsset<Texture2D>(noiseTexturePath);
			i++;
		}

		LOGINFO("Initialization success.");
	}

	void Firefly::Renderer::Begin()
	{
		localGlobalFrameCount++;
		localFrameCount[localGlobalFrameCount % 2]++;
		auto localBufferCache = GetCacheFromID(0);



		if (localBufferCache->CurrentCamera)
		{
			localBufferCache->CurrentCamera = std::make_shared<Camera>(*localBufferCache->CurrentCamera.get());
		}
#ifdef FF_SHIPIT
		else
		{
			const std::wstring message(L"You are trying to render without a camera! This is not alowed and needs to be looked at! This could be caused by not having a camera component in the loaded scene.");
			MessageBox(NULL, message.c_str(), L"Renderer Error!", MB_ICONERROR | MB_OK);
		}
#endif


#if FF_MT_RENDERING
		localRenderThreadFuture = std::async(std::launch::async, [&]()
			{
				InternalBegin();
			});
#else
		{
			InternalBegin();
		}
#endif
	}

	void Renderer::InternalBegin()
	{
		FF_PROFILETHREAD("Render Thread");
		FF_PROFILESCOPE("Firefly::Renderer::Begin");
		auto localBufferCache = GetCacheFromID(0);
		if (localBufferCache->CurrentCamera)
		{
			SceneRenderer sceneRenderer(localBufferCache, 0, localDeferredContext, localFrameCount[localGlobalFrameCount % 2]);

			sceneRenderer.UpdateBuffers(localDeferredContext);

			sceneRenderer.PrePass(localDeferredContext);

			sceneRenderer.DirectionalLightShadowPass(localDeferredContext);
			sceneRenderer.PointLightShadowPass(localDeferredContext);
			sceneRenderer.SpotLightShadowPass(localDeferredContext);

			localBlueNoise[localFrameCount[localGlobalFrameCount % 2] % 16]->Bind(90, ShaderType::Compute, localDeferredContext->Get());
			sceneRenderer.VolumePass(localDeferredContext);
			localBlueNoise[localFrameCount[localGlobalFrameCount % 2] % 16]->UnBind(90, ShaderType::Compute, localDeferredContext->Get());
			sceneRenderer.SkyLightPass(localDeferredContext);

			sceneRenderer.DeferredPass(localDeferredContext);

			sceneRenderer.HBAOPass(localDeferredContext, localAOContext);

			sceneRenderer.DeferredLightPass(localDeferredContext);
			if (localBufferCache->DrawGrid)
			{
				sceneRenderer.GridPass(localDeferredContext);
			}
			sceneRenderer.ForwardPass(localDeferredContext);

			sceneRenderer.ParticlePass(localDeferredContext);

			if (localBufferCache->DrawOutlines)
			{
				sceneRenderer.OutlinePass(localDeferredContext);
			}

			sceneRenderer.PostProcessPass(localDeferredContext);

			sceneRenderer.UIPass(localDeferredContext);
			sceneRenderer.Flush(localDeferredContext);

			GraphicsContext::GetRenderStateManager().PopCullState(localDeferredContext->Get());
		}
		RenderScenes();
	}

	void Renderer::Sync()
	{
		FF_PROFILEFUNCTION();
		GraphicsContext::BeginEvent("Main Scene");

		{
			FF_PROFILESCOPE("Find what postprocess volume to use.");
			PostprocessManager::FindPostProcessValue();
		}

		if (localRenderThreadFuture.valid())
		{
			localRenderThreadFuture.wait();
		}
		if (localCacheRegistry.contains(localOneshotID))
		{
			if (localDrawOneshot)
			{
				DrawSceneWithID(localOneshotID);
				localDrawOneshot = false;
			}
		}


		ID3D11CommandList* list = nullptr;
		localDeferredContext->Get()->FinishCommandList(FALSE, &list);
		if (list)
		{
			GraphicsContext::Context()->ExecuteCommandList(list, FALSE);
			list->Release();
		}

		auto& mainFBSpecs = WRITE_CACHE_MAIN->RendererFrameBuffer->GetSpecs();
		bool resizeMain = false;
		for (auto& TracedFramebuffer : WRITE_CACHE_MAIN->TracedFramebuffers)
		{
			bool needResize = TracedFramebuffer.FrameBuffer->Resize({
				mainFBSpecs.Width /
				(uint32_t)TracedFramebuffer.ResolutionDivider,
				mainFBSpecs.Height /
				(uint32_t)TracedFramebuffer.ResolutionDivider });
			if (needResize)
			{
				resizeMain = true;
			}
		}
		if (resizeMain)
		{
			//TODO: Fix memoryleak
			WRITE_CACHE_MAIN->DeferredUAVTexture = CreateRef<Texture2D>(mainFBSpecs.Width, mainFBSpecs.Height, ImageFormat::RGBA16F, 1, nullptr, true, true);
			READ_CACHE_MAIN->DeferredUAVTexture = WRITE_CACHE_MAIN->DeferredUAVTexture;
		}

		for (auto& [id, cache] : localCacheRegistry)
		{
			auto& fsBSpecs = GetCacheFromID(id, 1)->RendererFrameBuffer->GetSpecs();
			bool resize = false;
			for (auto& TracedFramebuffer : GetCacheFromID(id, 1)->TracedFramebuffers)
			{
				auto didToResize = TracedFramebuffer.FrameBuffer->Resize({
					fsBSpecs.Width /
					(uint32_t)TracedFramebuffer.ResolutionDivider,
					fsBSpecs.Height /
					(uint32_t)TracedFramebuffer.ResolutionDivider });
				if (didToResize)
				{
					resize = true;
				}
			}
			if (resize)
			{
				//TODO: Fix memoryleak
				GetCacheFromID(id, 1)->DeferredUAVTexture = CreateRef<Texture2D>(fsBSpecs.Width, fsBSpecs.Height, ImageFormat::RGBA16F, 1, nullptr, true, true);
				GetCacheFromID(id, 0)->DeferredUAVTexture = GetCacheFromID(id, 1)->DeferredUAVTexture;
			}
		}
		if (localOneshotID != InvalidSceneID)
		{
			const auto& oneshotFBSpecs = GetCacheFromID(localOneshotID, 1)->RendererFrameBuffer->GetSpecs();
			bool resize = false;
			for (auto& TracedFramebuffer : GetCacheFromID(localOneshotID, 1)->TracedFramebuffers)
			{
				auto didToResize = TracedFramebuffer.FrameBuffer->Resize({
					512 /
					(uint32_t)TracedFramebuffer.ResolutionDivider,
					512 /
					(uint32_t)TracedFramebuffer.ResolutionDivider });
				if (didToResize)
				{
					resize = true;
				}
			}
			if (resize)
			{
				//TODO: Fix memoryleak
				GetCacheFromID(localOneshotID, 1)->DeferredUAVTexture = CreateRef<Texture2D>(oneshotFBSpecs.Width, oneshotFBSpecs.Height, ImageFormat::RGBA16F, 1, nullptr, true, true);
				GetCacheFromID(localOneshotID, 0)->DeferredUAVTexture = GetCacheFromID(localOneshotID, 1)->DeferredUAVTexture;
			}
		}

		globalRendererStats.TriCount = READ_CACHE_MAIN->triCount;
		globalRendererStats.LineCount = READ_CACHE_MAIN->LineCount;
		globalRendererStats.DrawCalls = READ_CACHE_MAIN->DrawCount;

		READ_CACHE_MAIN->LightInjections[1] = WRITE_CACHE_MAIN->LightInjections[0];
		WRITE_CACHE_MAIN->LightInjections[1] = READ_CACHE_MAIN->LightInjections[0];

		WRITE_CACHE_MAIN->CameraBuffData.OldViewProj = READ_CACHE_MAIN->CameraBuffData.CameraSpace * READ_CACHE_MAIN->CameraBuffData.ToProjectionSpace;

		READ_CACHE_MAIN->CurrentCamera = WRITE_CACHE_MAIN->CurrentCamera;

		ShaderLibrary::BuildShaders();
	}

	void Renderer::RenderScenes()
	{
		FF_PROFILESCOPE("Draw sub scenes");
		for (auto& func : GetCacheFromID(0)->SceneDrawQueue)
		{
			func();
		}
		GetCacheFromID(0)->SceneDrawQueue.clear();
	}

	void Renderer::CollectDrawCallsFromList(Draw aDrawID, std::vector<MeshBatch>& aMeshBatch, std::vector<MeshSubmitInfo>& aCommandList, std::function<bool(size_t, size_t, size_t, MeshSubmitInfo&, MeshSubmitInfo&)>&& aFilterLambda, int32_t aOverrideLOD, std::function<bool(MeshSubmitInfo&)>&& aNotDrawFilter)
	{
		FF_PROFILEFUNCTION();

		thread_local static std::array<InstanceBoneData, MAX_BONE_COUNT> boneInsert;

		size_t currentBatchSize = 0;

		int32_t currentLOD = 0;
		auto& indexRange = localDrawDataBatches[static_cast<uint32_t>(aDrawID)];
		indexRange.first = aMeshBatch.size();
		for (size_t i = 0; i < aCommandList.size(); ++i)
		{
			auto& command = aCommandList[i];
			auto& nextCommand = aCommandList[(i + 1) % aCommandList.size()];

			if (aNotDrawFilter && aNotDrawFilter(command))
			{
				continue;
			}
			// just keep track of the current batch size.
			currentBatchSize++;

			// Instance properties to bind 
			auto& instanceProperty = localInstanceProperties.emplace_back(command.Transform);

			instanceProperty.CreationTime_ScaledTotalTime_IsAnimation_Resirve.x = Utils::Timer::GetScaledTotalTime() - command.CreationTime;
			instanceProperty.CreationTime_ScaledTotalTime_IsAnimation_Resirve.y = static_cast<float>(command.IsAnimation);
			memcpy(&instanceProperty.CreationTime_ScaledTotalTime_IsAnimation_Resirve.z, &command.EntityID, sizeof(size_t));

			currentLOD = command.lodID;

			if (aOverrideLOD > -1)
			{
				currentLOD = aOverrideLOD;
			}

			if (command.IsAnimation)
			{
				memcpy(&boneInsert[0], &command.BoneTransforms[0], sizeof(Utils::Matrix4f) * MAX_BONE_COUNT);
				localBonesProperties.insert(localBonesProperties.end(), boneInsert.begin(), boneInsert.end());
			}

			// the lambda is what desides what can be bundled together.
			if (aFilterLambda(i, aCommandList.size(), currentBatchSize, command, nextCommand) || command.lodID != nextCommand.lodID)
			{
				localIndirectDrawCallsProperties.emplace_back(command.Mesh->GetIndexBuffer(currentLOD)->GetBufferSize(), currentBatchSize, 0, 0, 0);

				auto& batch = aMeshBatch.emplace_back(command);
				batch.InstanceOffset = localInstanceProperties.size() - currentBatchSize;
				batch.BoneOffset = localBonesProperties.size() - currentBatchSize * MAX_BONE_COUNT;
				batch.BatchSize = currentBatchSize;
				currentBatchSize = 0;
			}
		}
		indexRange.second = aMeshBatch.size();
	}

	void Renderer::SortCommands(Cache* aCache)
	{
		FF_PROFILEFUNCTION();
		auto deferred = std::async(std::launch::async, [&]() { { Renderer::SortMeshCmd(aCache->DeferredCommands); } });
		auto forward = std::async(std::launch::async, [&]() { { Renderer::SortMeshCmdByDepth(aCache->ForwardCommands); }});

		forward.wait();
		deferred.wait();
	}

	Ref<Mesh> Renderer::GetDefaultCube()
	{
		if (!localDefaultCube)
		{
			localDefaultCube = ResourceCache::GetAsset<Mesh>("Cube");
		}
		return localDefaultCube;
	}

	void Renderer::Shutdown()
	{
	}

	void Renderer::SortMeshCmd(std::vector<MeshSubmitInfo>& aCmdList)
	{
		std::sort(aCmdList.begin(), aCmdList.end(), [&](MeshSubmitInfo& first, MeshSubmitInfo& sec) -> bool
			{
				const size_t firstID = ((first.Material->GetID() ^ first.Material->GetInfo().PipelineID) + first.Mesh->GetHash()) - first.lodID;
				const size_t secondID = ((sec.Material->GetID() ^ sec.Material->GetInfo().PipelineID) + sec.Mesh->GetHash()) - sec.lodID;
				if (firstID > secondID)
				{
					return true;
				}
				return false;
			});
	}

	void Renderer::SortMeshCmdByDepth(std::vector<MeshSubmitInfo>& aCmdList)
	{
		if (auto camera = GetActiveCamera())
		{
			std::sort(aCmdList.begin(), aCmdList.end(), [&](MeshSubmitInfo& first, MeshSubmitInfo& sec) -> bool
				{

					float firstClosestLgh = std::numeric_limits<float>::min();
					float secClosestLgh = std::numeric_limits<float>::min();

					const auto firstWorldHigh = Utils::Vec3ToVec4(first.Mesh->GetHighPoint()) * first.Transform;
					const auto secondWorldHigh = Utils::Vec3ToVec4(sec.Mesh->GetHighPoint()) * sec.Transform;

					firstClosestLgh = first.Material->GetInfo().ShouldBlend ? firstWorldHigh.y : firstClosestLgh;
					secClosestLgh = sec.Material->GetInfo().ShouldBlend ? secondWorldHigh.y : secClosestLgh;

					if (firstClosestLgh < secClosestLgh)
					{
						return true;
					}
					return false;
				});
		}
	}



	Cache* Renderer::GetCacheFromID(RenderSceneId aId)
	{
		auto whichCache = localGlobalFrameCount % 2;
		if (aId != 0)
		{
			if (whichCache == 1)
			{
				return localCacheRegistry[aId].cache2.get();
			}
			else
			{
				return localCacheRegistry[aId].cache1.get();
			}
		}
		else
		{
			if (whichCache)
			{
				return localCache->cache2.get();
			}
			else
			{
				return localCache->cache1.get();
			}

		}
	}

	void Firefly::Renderer::InitializeCache(Cache* aCache, RenderSceneId aId)
	{
		auto& cache = aCache;

		InitializeLineBatch(cache, aId);
		InitializeSpriteBatch(cache, aId);
		InitializeTextBatch(cache, aId);
		InitializeBillboardBatch(cache, aId);
		cache->Camerabuffer.Create("Camera buffer");
		cache->ModelBuffer.Create("Model buffer");
		cache->DirLightBuffer.Create("DirLight buffer");
		cache->ParticleBuffer.Create("Particle buffer");
		cache->TimeBuffer.Create("Time buffer");
		cache->PointLightBuffer.Create("PointLight buffer");
		cache->SpotLightBuffer.Create("SpotLight buffer");
		cache->MaterialBuffer.Create();
		cache->RenderPassBuffer.Create("RenderPass buffer");
		cache->PostProcessBuffer.Create("Postprocess buffer");
		cache->SkyAtmosphereBuffer.Create("Sky atmosphere buffer");
		cache->SkyHelperValuesBuffer.Create("");

		cache->IndirectCallsBuffer = IndirectBuffer::Create<IndirectArgs>(aCache->IsSubScene ? MAX_SUBSCENE_INDIRECT_COMMANDS : MAX_INDIRECT_COMMANDS);

		cache->ParticleDrawCallsBuffer = IndirectBuffer::Create<DrawInstancedIndirectArgs>(MAX_INDIRECT_PARTICLE_COMMANDS);

		cache->PrepassInstanceBuffer = StructuredBuffer::Create<PrepassData>(MAX_MESH_BATCH_COUNT);



		std::vector<Utils::Vector4f> ssaoNoise;
		ssaoNoise.reserve(16);
		for (size_t i = 0; i < 16; ++i)
		{
			Utils::Vector3f noise(Utils::RandomFloat() * 2.0 - 1.0, Utils::RandomFloat() * 2.0 - 1.0, 0.0f);
			ssaoNoise.push_back(Utils::Vec3ToVec4(noise));
		}
		//uint32_t width, uint32_t height, ImageFormat format, uint32_t levels, void* data, bool aIsFloat
		cache->NoiseTexture = std::make_shared<Texture2D>(4, 4, ImageFormat::RGBA32F, 1, ssaoNoise.data());

		cache->InstanceGPUBuffer = StructuredBuffer::Create<InstanceData>(aCache->IsSubScene ? MAX_SUBSCENE_INSTANCE_COUNT : MAX_INSTANCE_COUNT);

		cache->InstanceBoneGPUBuffer = StructuredBuffer::Create<InstanceBoneData>((aCache->IsSubScene ? MAX_SUBSCENE_BONE_INSTANCE_COUNT : MAX_BONE_INSTANCE_COUNT) * MAX_BONE_COUNT);

		cache->MultiScatterTexture = CreateRef<Texture2D>(32, 32, ImageFormat::RGBA16F, 0, nullptr, true, true);
		cache->DeferredUAVTexture = CreateRef<Texture2D>(64, 64, ImageFormat::RGBA16F, 0, nullptr, true, true);


		cache->LightInjections[0] = Texture3D::Create(160, 90, 92, ImageFormat::RGBA16F, 1);
		cache->LightInjections[1] = Texture3D::Create(160, 90, 92, ImageFormat::RGBA16F, 1);
		cache->RayMarchVolume = Texture3D::Create(160, 90, 92, ImageFormat::RGBA16F, 1);

		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::R8G8B8A8UN, ImageFormat::Depth32 };
			cache->RendererFrameBuffer = Framebuffer::Create(info);
			HandleFramebuffer(cache->RendererFrameBuffer, aId);
		}
		float initialMipSize = 1.0f;
		for (auto& BloomMipFB : cache->BloomPack.BloomMipFBs)
		{
			initialMipSize /= 2.0f;
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			BloomMipFB = Framebuffer::Create(info);
			HandleFramebuffer(BloomMipFB, aId, 1.f / initialMipSize);
		}
		for (auto& customPass : cache->CustomPostProcess)
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			customPass = Framebuffer::Create(info);
			HandleFramebuffer(customPass, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->BloomPack.FinalFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->BloomPack.FinalFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->FXAAFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->FXAAFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->SSGI = Framebuffer::Create(info);
			HandleFramebuffer(cache->SSGI, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RG32UI, ImageFormat::RGBA32F, ImageFormat::Depth32 };
			cache->DepthPrePass = Framebuffer::Create(info);
			HandleFramebuffer(cache->DepthPrePass, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->FogBuffer = Framebuffer::Create(info);
			HandleFramebuffer(cache->FogBuffer, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->ToneMappingBuffer = Framebuffer::Create(info);
			HandleFramebuffer(cache->ToneMappingBuffer, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 256;
			info.Height = 64;
			info.Mips = 4;
			info.Formats = { ImageFormat::RGBA16F };
			cache->TransmittanceLUT = Framebuffer::Create(info);
		}
		{
			FramebufferSpecs info{};
			info.Width = 32;
			info.Height = 32;
			info.Depth = 32;
			info.Is3D = true;
			info.Formats = { ImageFormat::RGBA16F };
			cache->CameraVolume = Framebuffer::Create(info);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->VignetteFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->VignetteFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->Atmosphere = Framebuffer::Create(info);
			HandleFramebuffer(cache->Atmosphere, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->VolumeResultFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->VolumeResultFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->OcclusionFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->OcclusionFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::R8UN };
			cache->OutlineFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->OutlineFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 2;
			info.Height = 1;
			info.Formats = { ImageFormat::RGBA16F };
			cache->JFAPrepass_FB = Framebuffer::Create(info);
			HandleFramebuffer(cache->JFAPrepass_FB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 1280;
			info.Height = 720;
			info.Formats = { ImageFormat::RGBA16F };
			cache->OutlineBufferFB = Framebuffer::Create(info);
			HandleFramebuffer(cache->OutlineBufferFB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 1280;
			info.Height = 720;
			info.Formats = { ImageFormat::RGBA16F };
			cache->JFAFinal_FB = Framebuffer::Create(info);
			HandleFramebuffer(cache->JFAFinal_FB, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 1280;
			info.Height = 720;
			info.Mips = 6;
			info.Formats = { ImageFormat::RGBA16F, ImageFormat::Depth16 };
			cache->ForwardGBuffer = Framebuffer::Create(info);
			HandleFramebuffer(cache->ForwardGBuffer, aId);
		}
		{
			FramebufferSpecs info{};
			info.Width = 1280;
			info.Height = 720;
			info.Mips = 1;
			info.Formats = { ImageFormat::Depth16 };
			cache->FPSDepthBuffer = Framebuffer::Create(info);
			HandleFramebuffer(cache->FPSDepthBuffer, aId);
		}
		{
			FramebufferSpecs specs{};
			specs.Height = 720;
			specs.Width = 1280;
			specs.Formats = { ImageFormat::RGBA16F, ImageFormat::RGBA16SN, ImageFormat::R8G8B8A8UN, ImageFormat::RGBA16SN, ImageFormat::RGBA32F, ImageFormat::R8UN, ImageFormat::RG32UI, ImageFormat::Depth32 };
			cache->DeferredGBuffer = Framebuffer::Create(specs);
			HandleFramebuffer(cache->DeferredGBuffer, aId);
		}
		FramebufferSpecs specs{};
		specs.Height = 4094;
		specs.Width = 4094;
		specs.Depth = 5;
		specs.Is2DArray = true;
		specs.Formats = { ImageFormat::Depth32 };
		cache->DirectionalShadowBuffer = Framebuffer::Create(specs);

		for (auto& spotFB : cache->SpotLightShadowFB)
		{
			FramebufferSpecs specs{};
			specs.Height = 512;
			specs.Width = 512;
			specs.Depth = 1;
			specs.Is2DArray = false;
			specs.Formats = { ImageFormat::Depth16 };
			spotFB = Framebuffer::Create(specs);
		}
		for (auto& pointFB : cache->PointLightShadowFB)
		{
			FramebufferSpecs specs{};
			specs.Height = 256;
			specs.Width = 256;
			specs.Depth = 6;
			specs.Is2DArray = false;
			specs.Formats = { ImageFormat::Depth16 };
			specs.IsCube = true;
			pointFB = Framebuffer::Create(specs);
		}
	}

	Cache* Firefly::Renderer::GetCacheFromID(RenderSceneId aId, size_t aFrameOffset)
	{
		auto whichCache = (localGlobalFrameCount + aFrameOffset) % 2;
		if (aId != 0)
		{
			if (whichCache == 1)
			{
				return localCacheRegistry[aId].cache2.get();
			}
			else
			{
				return localCacheRegistry[aId].cache1.get();
			}
		}
		else
		{
			if (whichCache == 1)
			{
				return localCache->cache2.get();
			}
			else
			{
				return localCache->cache1.get();
			}

		}
	}

	// Private
	void Renderer::InternalInitialize(RenderSceneId aId)
	{

		Cache* cache1;
		Cache* cache2;
		if (aId > 0)
		{
			cache1 = localCacheRegistry[aId].cache1.get();
			cache2 = localCacheRegistry[aId].cache2.get();
		}
		else
		{
			cache1 = localCache->cache1.get();
			cache2 = localCache->cache2.get();
		}

		InitializeCache(cache1, aId);
		InitializeCache(cache2, aId);
	}

	void Renderer::DrawSprite(const SpriteInfo& info, Cache* aCache, ID3D11DeviceContext* aContext)
	{
		static Utils::Vector4f poses[4];

		// generates the vertex positions of a sprite.
		poses[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		poses[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		poses[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		poses[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		auto& sprite = aCache->SpriteBatchSpecs;
		constexpr size_t quadVertexCount = 4;

		float textureIndex = 0.0f; // White Texture
		const Utils::Vector2<float> textureCoords[] = { info.UV[0], info.UV[1], info.UV[2], info.UV[3] };
		const float tilingFactor = 1.0f;

		// checks if there is a custom sprite shader. if the string is empty it persumes that it should use default sprite shader.
		if (!info.ShaderKey.empty())
		{
			ShaderLibrary::Bind(info.ShaderKey, aContext);
			std::array<SpriteVertex, 4> verts;
			for (size_t i = 0; i < quadVertexCount; i++)
			{
				// fill the sprite vertex for every quad.
				SpriteVertex buf;
				Vector4f vec4NewPos = poses[i] * info.Transform;
				verts[i].Position = { vec4NewPos.x, vec4NewPos.y, vec4NewPos.z };
				verts[i].Color = info.Color;
				verts[i].UV = textureCoords[i];
				verts[i].TexId = static_cast<uint32_t>(textureIndex);
				verts[i].Is3D = info.Is3D;
			}

			aCache->MaterialBuffer.SetData(&info.CustomValues.x, 4, aContext);
			aCache->MaterialBuffer.Bind(10);

			aCache->customVBSprite->SetData(verts.data(), verts.size() * sizeof(SpriteVertex), aContext);
			aCache->customVBSprite->Bind(aContext); // bind VertexBuffer.
			aCache->customIBSprite->Bind(aContext); // bind IndexBuffer.
			// we do not draw custom sprite shaders instanced due to the shaders will be very complex for a TA to work with. 
			aContext->DrawIndexed(aCache->customIBSprite->GetBufferSize(), 0, 0);
			ShaderLibrary::Unbind(info.ShaderKey, aContext);
			return;
		}


		if (sprite.IndexCount >= globalMaxIndices)
		{
			return;
		}

		// this loops through the 
		for (uint32_t i = 1; i < sprite.TextureSlotIndex; i++)
		{
			if (info.Texture)
			{
				if (*sprite.TextureSlots[i].get() == *info.Texture.get())
				{
					textureIndex = (float)i;
					break;
				}
			}
		}

		if (textureIndex == 0.0f)
		{
			if (sprite.TextureSlotIndex >= MaxTextures)
			{
				return;
			}
			if (info.Texture)
			{
				textureIndex = (float)sprite.TextureSlotIndex;
				sprite.TextureSlots[sprite.TextureSlotIndex] = info.Texture;
				sprite.TextureSlotIndex++;
			}
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			Vector4f vec4NewPos = poses[i] * info.Transform;
			if (sprite.QuadBufferPtr)
			{
				sprite.QuadBufferPtr->Position = { vec4NewPos.x, vec4NewPos.y, vec4NewPos.z };
				sprite.QuadBufferPtr->Color = info.Color;
				sprite.QuadBufferPtr->UV = textureCoords[i];
				sprite.QuadBufferPtr->TexId = static_cast<uint32_t>(textureIndex);
				sprite.QuadBufferPtr->Is3D = info.Is3D;
				sprite.QuadBufferPtr++;
			}
		}

		sprite.IndexCount += 6;
	}

	bool Renderer::CompileMeshBatches(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		const size_t currentMaxIndirectCommands = aCache->IsSubScene ? MAX_SUBSCENE_INDIRECT_COMMANDS : MAX_INDIRECT_COMMANDS;
		const size_t currentMaxInstanceCount = aCache->IsSubScene ? MAX_SUBSCENE_INSTANCE_COUNT : MAX_INSTANCE_COUNT;
		const size_t currentMaxBoneInstanceCount = aCache->IsSubScene ? MAX_SUBSCENE_BONE_INSTANCE_COUNT : MAX_BONE_INSTANCE_COUNT;

		if (localIndirectDrawCallsProperties.size() > currentMaxIndirectCommands)
		{
			LOGERROR("Too many Indirect draw calls! failed to compile frame!");
			ClearBatchProperties();
			return false;
		}

		if (localInstanceProperties.size() > currentMaxInstanceCount)
		{
			LOGERROR("Too many instances! failed to compile frame!");
			ClearBatchProperties();
			return false;
		}

		if (localBonesProperties.size() > currentMaxBoneInstanceCount * MAX_BONE_COUNT)
		{
			LOGERROR("Too many bone instances! failed to compile frame!");
			ClearBatchProperties();
			return false;
		}

		if (localIndirectDrawCallsProperties.empty() == false)
		{
			aCache->IndirectCallsBuffer->SetData(localIndirectDrawCallsProperties, aContext);
		}

		if (localInstanceProperties.empty() == false)
		{
			aCache->InstanceGPUBuffer->SetData(localInstanceProperties, aContext);
		}

		if (localBonesProperties.empty() == false)
		{
			aCache->InstanceBoneGPUBuffer->SetData(localBonesProperties, aContext);
		}

		ClearBatchProperties();

		aCache->DrawBatchIndexes = localDrawDataBatches;

		return true;
	}

	void Renderer::DrawTextData(TextInfo& aInfo)
	{
		Utils::Matrix4f translate;
		translate(4, 1) = aInfo.Position.x;
		translate(4, 2) = aInfo.Position.y;
		translate(4, 3) = aInfo.Position.z;
		if (!aInfo.Is3D)
		{
			translate(4, 3) = 0;
			aInfo.Position.z = 0;
		}
		Utils::Matrix4f trans = Utils::Matrix4f::CreateScaleMatrix({ aInfo.Size.x, aInfo.Size.y, 1.0f })
			* Utils::Matrix4f::CreateRotationMatrix(aInfo.Rotation)
			* translate;
		Cache* cache = READ_CACHE_THREAD;
		if (aInfo.Text.empty())
		{
			return;
		}
		uint32_t textureId = 0;
		for (size_t i = 0; i < cache->TextBatchSpecs.TextureSlotIndex; ++i)
		{
			if (cache->TextBatchSpecs.TextureSlots[i] == aInfo.Font->GetTexture())
			{
				textureId = i;
				break;
			}
		}

		if (textureId == 0)
		{
			textureId = cache->TextBatchSpecs.TextureSlotIndex;
			cache->TextBatchSpecs.TextureSlots[textureId] = aInfo.Font->GetTexture();
			cache->TextBatchSpecs.TextureSlotIndex++;
		}

		const auto& fontgeo = aInfo.Font->GetFontData().FontGeometry;
		const auto& metrics = fontgeo.getMetrics();
		std::u32string string;
		for (auto car : aInfo.Text)
		{
			string.push_back(car);
		}
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0;
			uint32_t lastNextLine = 0;
			for (int32_t i = 0; i < string.size(); ++i)
			{
				char32_t character = string[i];
				if (character == '\n')
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontgeo.getGlyph(character);
				if (!glyph)
				{
					glyph = fontgeo.getGlyph('?');
					continue;
				}
				double advance = glyph->getAdvance();
				fontgeo.getAdvance(advance, character, string[i + 1]);

				if (aInfo.TextWrapSize > 0)
				{
					if (character == ' ')
					{
						//find the next space
						int32_t nextSpace = i + 1;
						for (; nextSpace < string.size(); ++nextSpace)
						{
							if (string[nextSpace] == ' ')
							{
								break;
							}
						}

						//calculate the size of all the characters in the line
						double wordWidth = 0.0;
						for (int wordIndex = i + 1; wordIndex < nextSpace; wordIndex++)
						{
							auto glyph = fontgeo.getGlyph(string[wordIndex]);
							if (glyph)
							{
								double advance = glyph->getAdvance();
								fontgeo.getAdvance(advance, character, string[i + 1]);
								wordWidth += fsScale * advance;
							}
						}

						if ((x + wordWidth) > aInfo.TextWrapSize)
						{
							x = 0.0;
							y -= fsScale * metrics.lineHeight;
							continue;
						}
					}
				}


				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);
				double p1, pb, pr, pt;
				glyph->getQuadPlaneBounds(p1, pb, pr, pt);

				p1 *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				p1 += x, pb += y, pr += x, pt += y;

				double texelWidth = 1.0 / aInfo.Font->GetTexture()->GetWidth();
				double texelHeight = 1.0 / aInfo.Font->GetTexture()->GetHeight();
				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				auto& textData = cache->TextBatchSpecs;
				textData.QuadBufferPtr->Position = Utils::Vec4ToVec3(Utils::Vector4f(p1, pb, 0.f, 1.0f) * trans);
				textData.QuadBufferPtr->Color = aInfo.Color;
				textData.QuadBufferPtr->UV = { (float)l, (float)b };
				textData.QuadBufferPtr->TexId = textureId;
				textData.QuadBufferPtr->Is3D = aInfo.Is3D;
				textData.QuadBufferPtr++;

				textData.QuadBufferPtr->Position = Utils::Vec4ToVec3(Utils::Vector4f(pr, pb, 0.f, 1.0f) * trans);
				textData.QuadBufferPtr->Color = aInfo.Color;
				textData.QuadBufferPtr->UV = { (float)r, (float)b };
				textData.QuadBufferPtr->TexId = textureId;
				textData.QuadBufferPtr->Is3D = aInfo.Is3D;
				textData.QuadBufferPtr++;

				textData.QuadBufferPtr->Position = Utils::Vec4ToVec3(Utils::Vector4f(pr, pt, 0.f, 1.0f) * trans);
				textData.QuadBufferPtr->Color = aInfo.Color;
				textData.QuadBufferPtr->UV = { (float)r, (float)t };
				textData.QuadBufferPtr->TexId = textureId;
				textData.QuadBufferPtr->Is3D = aInfo.Is3D;
				textData.QuadBufferPtr++;

				textData.QuadBufferPtr->Position = Utils::Vec4ToVec3(Utils::Vector4f(p1, pt, 0.f, 1.0f) * trans);
				textData.QuadBufferPtr->Color = aInfo.Color;
				textData.QuadBufferPtr->UV = { (float)l, (float)t };
				textData.QuadBufferPtr->TexId = textureId;
				textData.QuadBufferPtr->Is3D = aInfo.Is3D;
				textData.QuadBufferPtr++;

				textData.IndexCount += 6;

				x += fsScale * advance;
			}
		}
	}
	void Renderer::DrawLineVertex(Cache* aCache, LineVertex& vertex, ID3D11DeviceContext* aContext)
	{
		localLineVertexBatchBuild[localLineVertexInterator++] = vertex;
		if (localLineVertexInterator >= MAX_LINE_BATCH_COUNT)
		{
			EndLineBatch(aCache, aContext);
			BeginLineBatch(aCache);
		}
	}

	void Renderer::DrawLine2DVertex(Cache* aCache, LineVertex& vertex, ID3D11DeviceContext* aContext)
	{
		localLineVertexBatchBuild[localLineVertexInterator++] = vertex;
		if (localLineVertexInterator >= MAX_LINE_BATCH_COUNT)
		{
			EndLine2DBatch(aCache, aContext);
			BeginLine2DBatch(aCache);
		}
	}

	void Renderer::DrawBillboard(Cache* aCache, BillboardInfo& aInfo, ID3D11DeviceContext* aContext)
	{
		auto& billboardBatch = aCache->BillBoardBatchSpecs;
		localBillboardBatchBuild[localBillboardInterator].Position = Utils::Vec3ToVec4(aInfo.Position);

		uint32_t textureIndex = 0u;
		// this loops through the 
		for (uint32_t i = 1; i < billboardBatch.TextureSlotIndex; i++)
		{
			if (aInfo.Texture)
			{
				if (*billboardBatch.TextureSlots[i].get() == *aInfo.Texture.get())
				{
					textureIndex = i;
					break;
				}
			}
		}

		if (textureIndex == 0)
		{
			if (billboardBatch.TextureSlotIndex >= MaxTextures)
			{
				return;
			}
			if (aInfo.Texture)
			{
				textureIndex = billboardBatch.TextureSlotIndex;
				billboardBatch.TextureSlots[billboardBatch.TextureSlotIndex] = aInfo.Texture;
				billboardBatch.TextureSlotIndex++;
			}
		}
		localBillboardBatchBuild[localBillboardInterator].TexId = textureIndex;
		localBillboardBatchBuild[localBillboardInterator].Color = aInfo.Color;

		memcpy(&localBillboardBatchBuild[localBillboardInterator].EntityID.x, &aInfo.EntityID, sizeof(size_t));

		++localBillboardInterator;
		if (localBillboardInterator >= MAX_BILLBOARD_BATCH_COUNT)
		{
			EndBillboardBatch(aCache, aContext);
			BeginBillboardBatch(aCache);
		}
	}

	Ref<Texture2D> Firefly::Renderer::GetEnvi()
	{
		return localTempEnvironmentTexture;
	}

	void Renderer::InitializeLineBatch(Cache* aCache, RenderSceneId aId)
	{
		Cache* cache = aCache;

		VertexBufferInfo info{};
		info.Data = localLineVertexBatchBuild.data();
		info.Count = MAX_LINE_BATCH_COUNT;
		info.ObjectSize = sizeof(LineVertex);
		cache->LineCacheSpecs.LineBuffer = VertexBuffer::Create(info);

		std::array<uint32_t, MAX_LINE_BATCH_COUNT> indices = {};

		for (size_t i = 0; i < indices.size(); ++i)
		{
			indices[i] = i;
		}

		cache->LineCacheSpecs.IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());

		info.Data = localLineVertexBatchBuild.data();
		info.Count = MAX_LINE_BATCH_COUNT;
		info.ObjectSize = sizeof(LineVertex);
		cache->LineCache2DSpecs.LineBuffer = VertexBuffer::Create(info);


		cache->LineCache2DSpecs.IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());
	}

	bool Renderer::CheckIfShaderIsDeferred(const size_t& pipelineID)
	{
		return PipelineLibrary::IsDeferredPipeline(pipelineID);
	}

	void Renderer::DrawSceneWithID(RenderSceneId aId)
	{
#ifndef FF_SHIPIT
		FF_PROFILEFUNCTION();
		Cache* cache = GetCacheFromID(aId);
		if (!cache->CurrentCamera)
		{
			return;
		}
		SceneRenderer sceneRenderer(cache, aId, localDeferredContext, localFrameCount[localGlobalFrameCount % 2]);

		sceneRenderer.UpdateBuffers(localDeferredContext);


		sceneRenderer.PrePass(localDeferredContext);
		sceneRenderer.SkyLightPass(localDeferredContext);
		sceneRenderer.DirectionalLightShadowPass(localDeferredContext);
		sceneRenderer.DeferredPass(localDeferredContext);
		sceneRenderer.HBAOPass(localDeferredContext, localAOContext);
		sceneRenderer.DeferredLightPass(localDeferredContext);
		if (cache->DrawGrid)
		{
			sceneRenderer.GridPass(localDeferredContext);
		}
		sceneRenderer.ForwardPass(localDeferredContext);

		sceneRenderer.ParticlePass(localDeferredContext);



		if (cache->DrawOutlines)
		{
			sceneRenderer.OutlinePass(localDeferredContext);
		}

		sceneRenderer.PostProcessPass(localDeferredContext);

		sceneRenderer.UIPass(localDeferredContext);

		sceneRenderer.Flush(localDeferredContext);
		GraphicsContext::GetRenderStateManager().PopCullState(localDeferredContext->Get());
#endif
}

	void Renderer::InitializeShaders()
	{
		///////////////////////COMPUTE SHADERS//////////////////////////

		ShaderLibrary::Add("BlueNoiseGen", { Shader::Create("FireflyEngine/Shaders/Engine/BlueNoiseGenerator_cs.hlsl", ShaderType::Compute) });
		ShaderLibrary::Add("GenerateCubeMap", { Shader::Create("FireflyEngine/Shaders/Engine/GenerateCubeMap_cs.hlsl", ShaderType::Compute) });

		ShaderLibrary::Add("MultiScatter", { Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/MultiScatter_cs.hlsl", ShaderType::Compute, true, {{"ILLUMINANCE_IS_ONE", ""}}) });

		ShaderLibrary::Add("CameraVolume",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/CameraVolume_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/CameraVolume_gs.hlsl", ShaderType::Geometry),
			Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/CameraVolume_ps.hlsl", ShaderType::Pixel, true, {{"MULTISCATAPPROX_ENABLED", "1"}})
			});
		ShaderLibrary::Add("FroxelVolume",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FroxelVolume_cs.hlsl", ShaderType::Compute)

			});
		ShaderLibrary::Add("DeferredCS",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/DeferredLightCalculation_cs.hlsl", ShaderType::Compute)
			});
		ShaderLibrary::Add("LightVolume",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/LightVolume_cs.hlsl", ShaderType::Compute)
			});

		ShaderLibrary::Add("SkyBox",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/SkyBox_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/SkyBox_ps.hlsl", ShaderType::Pixel)
			});

		ShaderLibrary::Add("RayMarchingSky",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/RayMarching_ps.hlsl", ShaderType::Pixel, true, {{"MULTISCATAPPROX_ENABLED", "1"}})
			});
		////////////////////////MESH DRAWING/////////////////////////////
		ShaderLibrary::Add("DeferredLightCalculation",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/DeferredLightCalculation_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/DeferredLightCalculation_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("JFA_Initial",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/JFA_Initial_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/JFA_Initial_ps.hlsl", ShaderType::Pixel)
			});;
		////////////////////////////////////////////////////////////////

		///////////////////////////BATCH SHADERS////////////////////////
		ShaderLibrary::Add("LineBatch",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Line_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Line_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Line2DBatch",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Line2D_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Line_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("ShaderTest",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/StaticMesh_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Asset/ShaderTest.hlsl", ShaderType::Pixel)
			});
		/*ShaderLibrary::Add("Particle",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Particle_GS.hlsl", ShaderType::Geometry),
				Shader::Create("FireflyEngine/Shaders/Engine/Particle_VS.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Particle_PS.hlsl", ShaderType::Pixel)
			});*/
		ShaderLibrary::Add("Billboard",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Billboard_gs.hlsl", ShaderType::Geometry),
				Shader::Create("FireflyEngine/Shaders/Engine/Billboard_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Billboard_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("BillboardColor",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Billboard_gs.hlsl", ShaderType::Geometry),
				Shader::Create("FireflyEngine/Shaders/Engine/Billboard_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/BillboardColor_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Grid",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Grid_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Grid_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Sprite",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Sprite_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Sprite_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("UIOrb",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Sprite_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/UIOrb_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Text",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Text_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Text_ps.hlsl", ShaderType::Pixel)
			});
		////////////////////////////////////////////////////////////////

		//////////////////// POST PROCESSING ///////////////////////////
		ShaderLibrary::Add("Occlusion",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Occlusion_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("VolumetricLighting",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/VolumetricLighting_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("HorizontalBlur",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/HorizontalBlur_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("VerticalBlur",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/VerticalBlur_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Fog",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Fog_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Fog_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("SSGI",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/SSGIRayMatch_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("JFAuv",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/JFA_UV_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("JFAFinal",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/JFA_Final_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("JFACookie",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/JFACookie_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Outline",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Outline_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("BloomDownSample",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/BloomDownSample_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("FirstBloomDownSample",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/FirstBloomDownSample_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("BloomUpSample",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/BloomUpSample_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Bloom",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Bloom_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("ApplyVolume",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/ApplyVolume_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("DirectionalShadows",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/DirectionalShadow_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/DirectionalShadow_gs.hlsl", ShaderType::Geometry),
				/*Shader::Create("FireflyEngine/Shaders/Engine/DirectionalShadow_ps.hlsl", ShaderType::Pixel)*/
			});
		ShaderLibrary::Add("SpotShadows",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/SpotShadow_vs.hlsl", ShaderType::Vertex),
				/*Shader::Create("FireflyEngine/Shaders/Engine/DirectionalShadow_ps.hlsl", ShaderType::Pixel)*/
			});
		ShaderLibrary::Add("PointShadows",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/PointShadow_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PointShadow_gs.hlsl", ShaderType::Geometry),
				Shader::Create("FireflyEngine/Shaders/Engine/PointShadow_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Copy",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Copy_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Debanding",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Debanding_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("CopyDepth",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/CopyDepth_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("ToneMapping",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/ToneMapping_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("FXAA",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/FXAA_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("Vignette",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/Vignette_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("CloudShadow",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/CloudShadow_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("StochasticNormals",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/stochsticNormals_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("TransmittanceLUT",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/TransmittanceLut_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("SkyViewLut",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/FullScreenQuad_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/AtmosphericSky/SkyViewLut_ps.hlsl", ShaderType::Pixel, true, {{"MULTISCATAPPROX_ENABLED", "1"}})
			});

		ShaderLibrary::Add("WorldPositions",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/StaticMesh_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/WorldPosition_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("DepthPrepass",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/Prepass_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/Prepass_ps.hlsl", ShaderType::Pixel)
			});
		ShaderLibrary::Add("WorldPositionsAnimation",
			{
				Shader::Create("FireflyEngine/Shaders/Engine/AnimatedMesh_vs.hlsl", ShaderType::Vertex),
				Shader::Create("FireflyEngine/Shaders/Engine/PostProcessing/WorldPosition_ps.hlsl", ShaderType::Pixel)
			});
		////////////////////////////////////////////////////////////////
	}

	void Renderer::InitializeSpriteBatch(Cache* aCache, RenderSceneId aId)
	{
		Cache* cache = aCache;
		cache->SpriteDrawQueue.Initialize(128);

		auto& sprite = cache->SpriteBatchSpecs;
		sprite.QuadBuffer = new SpriteVertex[globalMaxVertices];
		auto f = new SpriteVertex[4];

		{
			VertexBufferInfo info{};
			info.Data = &sprite.QuadBuffer[0];
			info.Count = globalMaxVertices;
			info.ObjectSize = sizeof(SpriteVertex);
			cache->SpriteBatchSpecs.VertBuffer = VertexBuffer::Create(info);
			info.Data = &f[0];
			info.Count = 4;
			cache->customVBSprite = VertexBuffer::Create(info);
			delete[] f;
		}

		uint32_t customIndices[6];
		customIndices[0] = 0;
		customIndices[1] = 1;
		customIndices[2] = 2;

		customIndices[3] = 2;
		customIndices[4] = 3;
		customIndices[5] = 0;
		cache->customIBSprite = IndexBuffer::Create(&customIndices[0], 6);
		uint32_t* quadIndices = new uint32_t[globalMaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < globalMaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		sprite.IndBuffer = IndexBuffer::Create(quadIndices, globalMaxIndices);
		delete[] quadIndices;
	}

	void Renderer::InitializeTextBatch(Cache* aCache, RenderSceneId aId)
	{
		Cache* cache = aCache;

		auto& textbatch = cache->TextBatchSpecs;
		textbatch.QuadBuffer = new SpriteVertex[globalMaxVertices];

		{
			VertexBufferInfo info{};
			info.Data = &textbatch.QuadBuffer[0];
			info.Count = globalMaxVertices;
			info.ObjectSize = sizeof(SpriteVertex);
			textbatch.VertBuffer = VertexBuffer::Create(info);
		}

		auto* quadIndices = new uint32_t[globalMaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < globalMaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		textbatch.IndBuffer = IndexBuffer::Create(quadIndices, globalMaxIndices);
		delete[] quadIndices;
	}

	void Renderer::InitializeBillboardBatch(Cache* aCache, RenderSceneId aId)
	{
		Cache* cache = aCache;

		auto& billboardBatch = cache->BillBoardBatchSpecs;

		{
			VertexBufferInfo info{};
			info.Data = localBillboardBatchBuild.data();
			info.Count = MAX_BILLBOARD_BATCH_COUNT;
			info.ObjectSize = sizeof(BillBoardVertex);
			billboardBatch.BillBoardBuffer = VertexBuffer::Create(info);
		}

	}

	void Renderer::BeginSpriteBatch(Cache* aCache)
	{
		aCache->SpriteBatchSpecs.IndexCount = 0;
		aCache->SpriteBatchSpecs.QuadBufferPtr = aCache->SpriteBatchSpecs.QuadBuffer;
		aCache->SpriteBatchSpecs.TextureSlotIndex = 1;
	}

	void Renderer::BeginTextBatch(Cache* aCache)
	{
		aCache->TextBatchSpecs.IndexCount = 0;
		aCache->TextBatchSpecs.QuadBufferPtr = aCache->TextBatchSpecs.QuadBuffer;
		aCache->TextBatchSpecs.TextureSlotIndex = 1;
	}

	void Renderer::EndSpriteBatch(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		auto& sprite = aCache->SpriteBatchSpecs;
		if (sprite.IndexCount)
		{
			const auto dataSize = (uint32_t)((uint8_t*)sprite.QuadBufferPtr - (uint8_t*)sprite.QuadBuffer);
			sprite.VertBuffer->SetData(sprite.QuadBuffer, dataSize, aContext);
			sprite.VertBuffer->Bind(aContext);
			sprite.IndBuffer->Bind(aContext);

			// Bind textures
			for (uint32_t i = 0; i < sprite.TextureSlotIndex; i++)
			{
				if (sprite.TextureSlots[i])
					sprite.TextureSlots[i]->Bind(i, ShaderType::Pixel, aContext);
			}
			globalRendererStats.DrawCalls++;
			aContext->DrawIndexed(sprite.IndexCount, 0, 0);
		}
	}

	void Renderer::BeginLineBatch(Cache* aCache)
	{
		localLineVertexInterator = 0;
		memset(localLineVertexBatchBuild.data(), 0, MAX_LINE_BATCH_COUNT * sizeof(LineVertex));
	}

	void Renderer::BeginLine2DBatch(Cache* aCache)
	{
		localLineVertexInterator = 0;
		memset(localLineVertexBatchBuild.data(), 0, MAX_LINE_BATCH_COUNT * sizeof(LineVertex));
	}

	void Renderer::EndLineBatch(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		aCache->LineCacheSpecs.LineBuffer->SetData(localLineVertexBatchBuild.data(), localLineVertexBatchBuild.size() * sizeof(LineVertex), aContext);
		aCache->LineCacheSpecs.LineBuffer->Bind(aContext);

		aCache->LineCacheSpecs.IndexBuffer->Bind(aContext);

		aCache->LineCount += localLineVertexInterator / 2;

		aContext->DrawIndexed(aCache->LineCacheSpecs.IndexBuffer->GetBufferSize(), 0, 0);
	}

	void Renderer::EndLine2DBatch(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		aCache->LineCache2DSpecs.LineBuffer->SetData(localLineVertexBatchBuild.data(), localLineVertexBatchBuild.size() * sizeof(LineVertex), aContext);
		aCache->LineCache2DSpecs.LineBuffer->Bind(aContext);

		aCache->LineCache2DSpecs.IndexBuffer->Bind(aContext);

		aCache->LineCount += localLineVertexInterator / 2;

		aContext->DrawIndexed(aCache->LineCache2DSpecs.IndexBuffer->GetBufferSize(), 0, 0);
	}

	void Renderer::EndTextBatch(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		auto& textbatch = aCache->TextBatchSpecs;
		if (textbatch.IndexCount == 0)
		{
			return;
		}

		const auto dataSize = (uint32_t)((uint8_t*)textbatch.QuadBufferPtr - (uint8_t*)textbatch.QuadBuffer);
		textbatch.VertBuffer->SetData(textbatch.QuadBuffer, dataSize, aContext);
		textbatch.VertBuffer->Bind(aContext);
		textbatch.IndBuffer->Bind(aContext);

		// Bind textures
		for (uint32_t i = 0; i < textbatch.TextureSlotIndex; i++)
		{
			if (textbatch.TextureSlots[i])
			{
				textbatch.TextureSlots[i]->Bind(i, ShaderType::Pixel, aContext);
			}
		}
		globalRendererStats.DrawCalls++;
		aContext->DrawIndexed(textbatch.IndexCount, 0, 0);
	}

	void Renderer::BeginBillboardBatch(Cache* aCache)
	{
		localBillboardInterator = 0;
		aCache->BillBoardBatchSpecs.TextureSlotIndex = 1;
		memset(localBillboardBatchBuild.data(), 0, MAX_BILLBOARD_BATCH_COUNT * sizeof(BillBoardVertex));
	}

	void Renderer::EndBillboardBatch(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		auto& billboardBatch = aCache->BillBoardBatchSpecs;
		billboardBatch.BillBoardBuffer->SetData(localBillboardBatchBuild.data(), localBillboardBatchBuild.size(), aContext);
		billboardBatch.BillBoardBuffer->Bind(aContext);

		// Bind textures
		for (uint32_t i = 0; i < billboardBatch.TextureSlotIndex; i++)
		{
			if (billboardBatch.TextureSlots[i])
				billboardBatch.TextureSlots[i]->Bind(i, ShaderType::Pixel, aContext);
		}

		aContext->Draw(localBillboardInterator, 0);
	}

	void Renderer::RenderGeometry(Cache* aCache, Draw aDraw, Binding aBinding, ID3D11DeviceContext* aContext)
	{
		const bool bindMaterial = aBinding == Binding::BindMaterial;

		const auto& ranges = aCache->DrawBatchIndexes[static_cast<uint32_t>(aDraw)];

		for (size_t i = ranges.first; i < ranges.second; ++i)
		{
			auto& batch = aCache->MeshBatches[i];
			if (batch.SubmitInfo.Mesh == nullptr)
			{
				continue;
			}

			if (bindMaterial)
			{
				auto& material = batch.SubmitInfo.Material;

				if (!material)
				{
					continue;
				}

				auto& pipeline = PipelineLibrary::Get(material->GetInfo().PipelineID);

				pipeline.Bind(aContext);
				material->Bind(aContext);

				aCache->MaterialBuffer.SetData(material->GetInfo().MaterialData.data.data(), material->GetInfo().MaterialData.data.size(), aContext);
				aCache->MaterialBuffer.Bind(10, ShaderType::Pixel | ShaderType::Vertex, aContext);
			}

			auto& perDrawInfo = aCache->ModelBuffData;
			perDrawInfo.matrixBufferOffset = batch.InstanceOffset;
			perDrawInfo.boneStartOffset = batch.BoneOffset;

			aCache->ModelBuffer.SetData(&perDrawInfo, aContext);
			aCache->ModelBuffer.Bind(1, aContext);

			batch.SubmitInfo.Mesh->GetVertexBuffer()->Bind(aContext);
			batch.SubmitInfo.Mesh->GetIndexBuffer(batch.SubmitInfo.lodID)->Bind(aContext);

			aCache->triCount += (batch.SubmitInfo.Mesh->GetIndexBuffer(batch.SubmitInfo.lodID)->GetBufferSize() / 3.f) * batch.BatchSize;
			aCache->DrawCount++;

			aContext->DrawIndexedInstancedIndirect(aCache->IndirectCallsBuffer->GetBuffer().Get(), sizeof(IndirectArgs) * i);

			if (bindMaterial)
			{
				auto& material = batch.SubmitInfo.Material;

				if (!material)
				{
					continue;
				}

				auto& pipeline = PipelineLibrary::Get(material->GetInfo().PipelineID);

				material->UnBind(aContext);
				pipeline.UnBind(aContext);
			}
		}
	}

	Draw Renderer::ExtractDrawFlagFromIndex(Firefly::Draw aDraw, const size_t aIndex)
	{
		return static_cast<Draw>(static_cast<uint32_t>(aDraw) + aIndex);
	}

	void Renderer::CopyDepth(Ref<Framebuffer> aFrom, Ref<Framebuffer> aTo, ID3D11DeviceContext* aContext)
	{
		aFrom->BindDepthSRV(0, ShaderType::Pixel, aContext);
		PostProcessUtils::FullQuadPass("CopyDepth", aTo, aContext);
		aFrom->UnBindShaderResource(0, ShaderType::Pixel, aContext);
	}

	void Renderer::HandleFramebuffer(Ref<Framebuffer> aFB, RenderSceneId aId, float aScale)
	{
		Cache* cache = GetCacheFromID(aId);
		cache->TracedFramebuffers.push_back({ aFB, aScale });
	}

	void Renderer::UpdateAtmosphereBuffers(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		Cache* cache = aCache;
		cache->AtmosphereParams.mie_phase_function_g = 0.99;
		auto& cb = cache->SkyAtmosphereData;
		cb.solar_irradiance = cache->EnvironmentInfo.SkyLightIntensity.GetNormalized();
		cb.sun_angular_radius = 0.0004675f;
		cb.absorption_extinction = cache->AtmosphereParams.absorption_extinction;
		cb.mu_s_min = cache->AtmosphereParams.mu_s_min;


		memcpy(cb.rayleigh_density, &cache->AtmosphereParams.rayleigh_density, sizeof(cache->AtmosphereParams.rayleigh_density));
		memcpy(cb.mie_density, &cache->AtmosphereParams.mie_density, sizeof(cache->AtmosphereParams.mie_density));
		memcpy(cb.absorption_density, &cache->AtmosphereParams.absorption_density, sizeof(cache->AtmosphereParams.absorption_density));

		cb.mie_phase_function_g = cache->AtmosphereParams.mie_phase_function_g;
		cb.rayleigh_scattering = cache->AtmosphereParams.rayleigh_scattering;
		const float RayleighScatScale = 1.0f;
		cb.rayleigh_scattering.x *= RayleighScatScale;
		cb.rayleigh_scattering.y *= RayleighScatScale;
		cb.rayleigh_scattering.z *= RayleighScatScale;
		cb.mie_scattering = cache->AtmosphereParams.mie_scattering;
		auto subtracked = (cache->AtmosphereParams.mie_extinction - cache->AtmosphereParams.mie_scattering);
		cb.mie_absorption = { Utils::Clamp(subtracked.x, 0.f, FLT_MAX), Utils::Clamp(subtracked.y, 0.f, FLT_MAX), Utils::Clamp(subtracked.z, 0.f,FLT_MAX) };
		cb.mie_extinction = cache->AtmosphereParams.mie_extinction;
		cb.ground_albedo = cache->EnvironmentInfo.GroundColor;
		cb.bottom_radius = cache->AtmosphereParams.bottom_radius;
		cb.top_radius = cache->AtmosphereParams.top_radius;
		cb.MultipleScatteringFactor = 1;
		cb.MultiScatteringLUTRes = 32;

		//
		cb.TRANSMITTANCE_TEXTURE_WIDTH = 256;
		cb.TRANSMITTANCE_TEXTURE_HEIGHT = 64;
		cb.IRRADIANCE_TEXTURE_WIDTH = 64;
		cb.IRRADIANCE_TEXTURE_HEIGHT = 16;
		cb.SCATTERING_TEXTURE_R_SIZE = 32;
		cb.SCATTERING_TEXTURE_MU_SIZE = 128;
		cb.SCATTERING_TEXTURE_MU_S_SIZE = 32;
		cb.SCATTERING_TEXTURE_NU_SIZE = 8;
		cb.SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = Utils::Vec3(114974.916437f, 71305.954816f, 65310.548555f); // Not used if using LUTs as transfert
		cb.SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = Utils::Vec3(98242.786222f, 69954.398112f, 66475.012354f);  // idem
		auto& dirlight = cache->DirLightBuffData.DirectionLightPacket[0];
		auto camera = cache->CurrentCamera;
		auto& trans = camera->GetTransform();
		Utils::Vec3 uePos = { trans.GetPosition().z / 1000.f, trans.GetPosition().x / 1000.f, 0.f + 1 };
		Utils::Vec3 ueFront = { trans.GetForward().z, trans.GetForward().x, trans.GetForward().y };
		auto view = Utils::Matrix4f::CreateLookAt(uePos, uePos + ueFront, { 0,0.f,1.f });
		const auto viewProj = cache->CurrentCamera->GetProjectionMatrixPerspective() * view;
		cb.gSkyViewProjMat = viewProj;
		cb.gSkyInvViewProjMat = Utils::Matrix4f::GetInverse(viewProj);
		cb.gSkyInvProjMat = Utils::Matrix4f::GetInverse(cache->CurrentCamera->GetProjectionMatrixPerspective());
		cb.gSkyInvViewMat = Utils::Matrix4f::GetInverse(view);
		cb.gShadowmapViewProjMat = dirlight.ViewMatrix * dirlight.ProjMatrix;
		cb.camera = uePos;
		cb.view_ray = ueFront;
		auto sunDir = Utils::Vec4ToVec3(dirlight.Direction);
		cb.sun_direction = { sunDir.z, sunDir.x, sunDir.y };

		cache->SkyAtmosphereBuffer.SetData(&cb, aContext);
	}

	void Renderer::UpdateSkyHelperBuffers(Cache* aCache, ID3D11DeviceContext* aContext)
	{
		Cache* cache = aCache;
		auto& hv = cache->SkyHelperValues;
		auto camera = cache->CurrentCamera;
		auto& trans = camera->GetTransform();
		Utils::Vec3 uePos = { trans.GetPosition().z / 1000.f , trans.GetPosition().x / 1000.f , (trans.GetPosition().y / 1000.f) + 1 };
		Utils::Vec3 ueFront = { trans.GetForward().z, trans.GetForward().x, trans.GetForward().y };
		auto view = Utils::Matrix4f::CreateLookAt(uePos, uePos + ueFront, { 0,0.f,1.f });
		const auto viewProj = cache->CurrentCamera->GetProjectionMatrixPerspective() * view;

		hv.gViewProjMat = viewProj;
		hv.RayMarchMinMaxSPP.x = 4;
		hv.RayMarchMinMaxSPP.y = 14;
		hv.gSunIlluminance = { cache->EnvironmentInfo.SkyLightIntensity.x, cache->EnvironmentInfo.SkyLightIntensity.y, cache->EnvironmentInfo.SkyLightIntensity.z };
		hv.gColor = { 0,1,1,1 };
		hv.gScatteringMaxPathDepth = 40;
		cache->SkyHelperValuesBuffer.SetData(&hv, aContext);
	}

	void Renderer::InitalizeHBAO()
	{
		localCustomHeap.new_ = ::operator new;
		localCustomHeap.delete_ = ::operator delete;

		GFSDK_SSAO_Status status;
		status = GFSDK_SSAO_CreateContext_D3D11(GraphicsContext::Device().Get(), &localAOContext, &localCustomHeap);
		assert(status == GFSDK_SSAO_OK);
	}
}