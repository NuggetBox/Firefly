#pragma once
#include "RenderingDefines.h"

#include "Firefly/Rendering/Buffer/ComputeBuffer.hpp"
#include "Firefly/Rendering/Buffer/IndirectBuffer.hpp"
#include "Firefly/Rendering/Font/FireflyFont.h"

#include "Utils/Math/Sphere.hpp"

#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Asset/Texture/Texture3D.h"
#include "Firefly/Rendering/AtmosphericSky/SkyAtmosphereData.h"
#include "Firefly/Rendering/Buffer/ConstantBuffer.hpp"
#include "Firefly/Rendering/Buffer/ConstantBuffers.h"
#include "Firefly/Rendering/Buffer/StructuredBuffer.h"
#include "Firefly/Rendering/Buffer/UndefinedConstBuffer.h"
#include "Firefly/Rendering/Camera/Camera.h"

#include "Utils/Queue.hpp"

namespace Firefly
{
	class ParticleEmitter;

	const uint32_t MaxTextures = 32;
	struct RendererStats
	{
		int32_t DrawCalls;
		uint32_t StaticMeshBatchCount;
		uint32_t AnimatedMeshBatchCount;
		int32_t PointLightCount;
		int32_t SpotLightCount;
		int32_t QuadCount;
		int32_t TriCount;
		int32_t LineCount;
		float GPUresponse;
		uint32_t MaterialBinds = 0;
		uint32_t TextureBinds = 0;
		uint32_t shaderBinds = 0;
		uint32_t VisablePass = 0; // 0 = normalRendering.
		void Reset()
		{
			DrawCalls = 0;
			PointLightCount = 0;
			SpotLightCount = 0;
			QuadCount = 0;
			TriCount = 0;
			LineCount = 0;
			MaterialBinds = 0;
			TextureBinds = 0;
			shaderBinds = 0;
			StaticMeshBatchCount = 0;
			AnimatedMeshBatchCount = 0;
		}
	};

	inline RendererStats globalRendererStats;
	inline RendererStats globalPrevRendererStats;


	inline std::string GetNameOfPassID(uint32_t aId)
	{
		switch (aId)
		{
			case 0:
				return "";
				break;
			case 1:
				return "Albedo";
				break;
			case 2:
				return "Vertex normals";
				break;
			case 3:
				return "Pixel normals";
				break;
			case 4:
				return "SSAO";
				break;
			case 5:
				return "Texture AO";
				break;
			case 6:
				return "Metalness";
				break;
		}
	}




	struct RenderPassData
	{
		uint32_t RenderPassId;
		float EnvironmentIntensity;
		Utils::Vector2f IsVolumetricFogActive_Pad;
		Utils::Vec4 EnvironmentFogColorIntensity;
		Utils::Vec4 godraysColorIntensity;
		Utils::Vec4 fogSettings;
		Utils::Vec4 BloomThreshhold;
		Utils::Vector4<int32_t> EnvironmentMip_padded3;
	};
	class MaterialAsset;

	struct MeshSubmitInfo
	{
		MeshSubmitInfo(const Utils::Matrix4f& aTransform = Utils::Matrix4f(), bool aIsAnimation = false) : Transform(aTransform), IsAnimation(aIsAnimation)
		{
			if (aIsAnimation)
			{
				BoneTransforms = new Utils::Matrix4x4<float>[MAX_BONE_COUNT];
			}
		}

		void SetBoneTransforms(const std::vector<Utils::Matrix4f>& someMatrices)
		{
			if (BoneTransforms == nullptr)
			{
				BoneTransforms = new Utils::Matrix4x4<float>[MAX_BONE_COUNT];
			}

			for (int i = 0; i < someMatrices.size(); ++i)
			{
				BoneTransforms[i] = someMatrices[i];
			}

			IsAnimation = true;
		}

		void CleanUp()
		{
			delete[] BoneTransforms;
			BoneTransforms = nullptr;
		}

		SubMesh* Mesh = nullptr;
		Utils::Matrix4x4<float> Transform;
		Utils::Sphere<float> BoundingSphere; // TODO:NIKLAS Ta bort denna med
		Ref<MaterialAsset> Material;
		Utils::Matrix4x4<float>* BoneTransforms = nullptr;
		uint64_t EntityID = 0;
		float CreationTime = 0;
		bool Outline = false;
		bool CastShadows = true;
		bool IsAnimation = false;
		bool AutomaticCleanup = true;
		// internal use of renderer.
		uint32_t lodID = 0;
	private:
		friend class SceneRenderer;
		friend class Renderer;
	};

	constexpr uint32_t globalMipBloomCount = 5;

	struct BloomPackage
	{
		std::array<Ref<Framebuffer>, globalMipBloomCount> BloomMipFBs;
		Ref<Framebuffer> FinalFB;
	};

	struct InstanceData
	{
		Utils::Matrix4f Transform;
		Utils::Vector4f CreationTime_ScaledTotalTime_IsAnimation_Resirve;
	};

	struct InstanceBoneData
	{
		Utils::Matrix4f Bone;
	};

	struct TraceOfFB
	{
		Ref<Framebuffer> FrameBuffer;
		float ResolutionDivider = 1.0f;
	};

	struct PrepassData
	{
		uint32_t Data[4];
	};

	struct DirectionalLightSubmitInfo
	{
		Utils::Vec3 Color = 1.f;
		float Intensity = 1.f;
		Utils::Vec3 Direction = Utils::Vec3(-0.71f, -0.71f, -0.71f);
		bool CastShadows = true;
		bool SoftShadows = true;
	};

	enum class PostProcessPass : uint32_t
	{
		None = 0,
		SSAO = 1 << 0,
		Bloom = 1 << 1,
		Fog = 1 << 2,
		ToneMapping = 1 << 3,
		Vignette = 1 << 4,
	};

	inline PostProcessPass operator|(PostProcessPass a, PostProcessPass b)
	{
		return static_cast<PostProcessPass>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline PostProcessPass& operator |=(PostProcessPass& a, PostProcessPass b)
	{
		return a = a | b;
	}
	inline bool operator&(PostProcessPass a, PostProcessPass b)
	{
		return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
	}

	struct PostProcessInfo
	{
		bool Enable = true;
		PostProcessPass passes = PostProcessPass::ToneMapping;
		PostProcessData Data;
	};

	struct LineBatch
	{
		std::vector<LineVertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t Itreator;
		Ref<VertexBuffer> LineBuffer;
		Ref<IndexBuffer> IndexBuffer;
	};
	struct BillboardInfo
	{
		Utils::Vector3f Position;
		uint64_t EntityID;
		Ref<Texture2D> Texture;
		Utils::Vec4 Color;
	};
	struct BillBoardBatch
	{
		std::vector<BillboardInfo> BillboardContainer;
		Ref<VertexBuffer> BillBoardBuffer;
		uint32_t IndexCount = 0;
		std::array<Ref<Texture2D>, MaxTextures> TextureSlots;
		uint32_t TextureSlotIndex = 1;
	};

	struct ParticleEmitterCommand
	{
		Utils::Matrix4f ParentTransform;
		Ref<ParticleEmitter> Emitter;
		bool ShouldCull = true;
	};

	enum class PassOrder : uint32_t
	{
		First = 0,
		Second = 1,
		Third = 2,
		Forth = 3
	};

	struct CustomPostprocessInfo
	{
		PassOrder Passorder;
		Ref<MaterialAsset> Material;
	};
	
	struct DecalSubmitInfo
	{
		Utils::Mat4 Matrix;
		uint64_t EntityId;
		Ref<MaterialAsset> DecalMaterial;
	};

	struct Sprite2DInfo
	{
		Utils::Vector2f Position;
		Utils::Quaternion Rotation;
		Utils::Vector2f Size;
		Utils::Vector4f Color;
		Utils::Vector2f UV[4];
		Ref<Texture2D> Texture;
		std::string ShaderKey = "";
		Utils::Vector4f CustomValues;
		bool IgnoreDepth = false;
	};

	struct TextInfo
	{
		std::string Text;
		Ref<Font> Font;
		Utils::Vector3f Position;
		Utils::Quaternion Rotation;
		Utils::Vector4f Color;
		Utils::Vector2f Size;
		float TextWrapSize;
		bool Is3D;
		bool IgnoreDepth = false;
	};

	struct Sprite3DInfo
	{
		Utils::Vector3f Position;
		Utils::Quaternion Rotation;
		Utils::Vector2f Size;
		Utils::Vector4f Color;
		Utils::Vector2f UV[4];
		Ref<Texture2D> Texture;
		std::string ShaderKey;
		Utils::Vector4f CustomValues;
		bool IgnoreDepth = false;

	private:
		friend class Renderer;
		bool Is3D = true;
	};

	struct SpriteInfo
	{
		Utils::Matrix4f Transform;
		Utils::Vector4f Color;
		Utils::Vector2f UV[4];
		Ref<Texture2D> Texture;
		bool Is3D;
		bool IgnoreDepth = false;
		std::string ShaderKey;
		Utils::Vector4f CustomValues;
	};

	struct SpriteBatch
	{
		Ref<VertexBuffer> VertBuffer;
		Ref<IndexBuffer> IndBuffer;

		uint32_t IndexCount = 0;

		SpriteVertex* QuadBuffer = nullptr;
		SpriteVertex* QuadBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextures> TextureSlots;
		uint32_t TextureSlotIndex = 1;
	};

	struct EnvironmentData
	{
		Ref<Texture2D> EnvironmentMap;
		Ref<Texture2D> SkyBoxMap;
		float Intensity;
		Utils::Vec3 SkyLightIntensity = { 3.f,3.f,3.f };
		Utils::Vec3 GroundColor = {1.f,1.f,1.f};
		float SunRadius = 0.0045f;
		int32_t EnvironmentMipMap = 0;
	};

	struct Tonemapping
	{
		Ref<Texture2D> ColorCorrectionLUT;
		bool useLUT;
	};

	struct VolumetricFogInfo
	{
		bool Enable;
		Utils::Vec3 Color;
		float_t Intensity;
		Utils::Vec3 ColorGodRay;
		float_t IntensityGodray;

		float_t Density = 0.01f;
		float_t Phase = 0.0f;
		float_t DepthPow = 1.f;
		float_t resurve = 0;
	};
	struct IndirectArgs
	{
		uint32_t IndexCountPerInstance;
		uint32_t InstanceCount;
		uint32_t StartIndexLocation;
		int32_t BaseVertexLocation;
		uint32_t StartInstanceLocation;
	};

	struct DrawInstancedIndirectArgs
	{
		uint32_t VertexCountPerInstance;
		uint32_t InstanceCount;
		uint32_t StartVertexLocation;
		uint32_t StartInstanceLocation;
	};

	struct BloomInfo
	{
		float BloomThreshhold;
	};

	
}
