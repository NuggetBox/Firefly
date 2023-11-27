#pragma once
#include <vector>
#include <functional>
#include "Firefly/Rendering/RenderCommands.h"
#include "Firefly/Rendering/Camera/Camera.h"
#include "Firefly/Rendering/Shader/Shader.h"
#include "Firefly/Rendering/Buffer/ConstantBuffer.hpp"
#include "Firefly/Rendering/Buffer/ConstantBuffers.h"

//LLLLLLLLLLL             
//L:::::::::L             
//L:::::::::L             
//LL:::::::LL             
//  L:::::L               
//  L:::::L               
//  L:::::L               
//  L:::::L               
//  L:::::L               
//  L:::::L               
//  L:::::L               
//  L:::::L         LLLLLL
//LL:::::::LLLLLLLLL:::::L
//L::::::::::::::::::::::L
//L::::::::::::::::::::::L
//LLLLLLLLLLLLLLLLLLLLLLLL

//	      | \__ / , | (`\
//	   _. | o o | _) )
//	- (((-- - (((--------

//        ________________   ___/-\___     ___/-\___     ___/-\___
//      / /             ||  |---------|   |---------|   |---------|
//     / /              ||   |       |     | | | | |     |   |   |
//    / /             __||   |       |     | | | | |     | | | | |
//   / /   \\        I  ||   |       |     | | | | |     | | | | |
//  (-------------------||   | | | | |     | | | | |     | | | | |
//  ||               == ||   |_______|     |_______|     |_______|
//  || FIREFLY TRASH CO | =============================================
//  ||          ____    |                                ____      |
// ( | o      / ____ \                                 / ____ \    |)
//  ||      / / . . \ \                              / / . . \ \   |
// [ |_____| | .   . | |____________________________| | .   . | |__]
//           | .   . |                                | .   . |
//            \_____/                                  \_____/

namespace Firefly
{
	using RenderSceneId = uint32_t;
	struct Cache;
	struct MeshCmd;
	struct AnimMeshCmd;
	class Framebuffer;
	struct Tonemapping;
	struct EnvironmentData;
	struct VolumetricFogInfo;
	struct MeshBatch;

	enum class Binding
	{
		BindMaterial,
		DontBindMaterial,
		Count,
	};

	enum class Draw : uint32_t
	{
		CulledDeferred = 0,
		CulledForward = 1,
		Deferred = 2,
		Forward = 3,
		Outline = 4,
		FPSForward = 5,
		Decals = 6,
		SpotLight0 = 10,
		SpotLight1 = 11,
		SpotLight2 = 12,
		SpotLight3 = 13,
		SpotLight4 = 14,
		SpotLight5 = 15,
		SpotLight6 = 16,
		SpotLight7 = 17,
		PointLight0 = 100,
		PointLight1 = 101,
		PointLight2 = 102,
		PointLight3 = 103,
		PointLight4 = 104,
		PointLight5 = 105,
		PointLight6 = 106,
		PointLight7 = 107,
	};

	class Renderer
	{
		friend class SceneRenderer;
		friend class PostProcessRenderer;
	public:
		/// <summary>
		/// Send a camera to be active during rendering.
		/// this will be needed to be updated everyframe.
		/// </summary>
		/// <param name="camera"></param>
		static void SubmitActiveCamera(Ref<Camera> camera);
		static Ref<Camera> GetActiveCamera();
		/// <summary>
		/// This is the Framebuffer.
		/// This is a form of image that the renderer is drawing to.
		/// This could be used to be sent to the swapchain image or to a ImGui::Image.
		/// To see more info about Framebuffer go to Framebuffer.h in:
		/// Firefly/Rendering/Framebuffer.h
		/// </summary>
		/// <returns>a Framebuffer</returns>
		static Ref<Framebuffer> GetFrameBuffer();

		static Ref<Framebuffer> GetSceneFrameBuffer(RenderSceneId aId);
		[[nodiscard]] static Ref<MaterialAsset> GetDefaultMaterial();

		static void Submit(MeshSubmitInfo& aCommand);
		static void Submit(const DirLightPacket& aLight, const ShadowResolutions& aShadowRes);
		static void Submit(const PointLightPacket& aLight);
		static void Submit(const SpotLightPacket& aLight);
		static void Submit(const VolumetricFogInfo& aFogInfo);
		static void Submit(EnvironmentData& aEnvironmentData);
		static void Submit(Tonemapping aToneMappingSettings);
		static void Submit(const ParticleEmitterCommand& anEmitterCommand);
		static void Submit(const Sprite2DInfo& aInfo);
		static void Submit(const Sprite3DInfo& aInfo);
		static void Submit(const BloomInfo& aInfo);
		static void Submit(TextInfo& aInfo);
		static void Submit(const PostProcessInfo& aInfo);
		static void Submit(const BillboardInfo& aInfo);
		static void Submit(const CustomPostprocessInfo& aInfo);
		static void Submit(const DecalSubmitInfo& aInfo);

		static void SubmitDebugLine(const Utils::Vector3f& aStart, const Utils::Vector3f& aEnd, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f }, bool aIs2D = false);
		static void SubmitDebugCircle(const Utils::Vector3f& aCenter = { 0.0f, 0.0f, 0.0f }, float aRadius = 50.0f, int aNumLines = 25, const Utils::Vector3f& aRotation = { 0.0f, 0.0f, 0.0f }, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugCircle(const Utils::Vector3f& aCenter, float aRadius, int aNumLines, const Utils::Quaternion& aRotation, const Utils::Vector4f& aColor);
		static void SubmitDebugSphere(const Utils::Vector3f& aCenter = { 0.0f, 0.0f, 0.0f }, float aRadius = 50.0f, int aNumLines = 25, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugCube(const Utils::Vector3f& aCenter = { 0.0f, 0.0f, 0.0f }, float aSize = 50.0f, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugCube(const Utils::Transform& aOriginTransform, const Utils::Vector3f& aOffset, float aSize = 50.0f, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugCuboid(const Utils::Vector3f& aCenter = { 0.0f, 0.0f, 0.0f }, const Utils::Vector3f& aSize = { 50.0f, 50.0f, 50.0f }, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugCuboid(const Utils::Transform& aOriginTransform, const Utils::Vector3f& aOffset, const Utils::Vector3f& aSize = { 50.0f,50.0f,50.0f }, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static void SubmitDebugArrow(const Utils::Vector3f& aStart, const Utils::Vector3f& aEnd, const Utils::Vector4f& aColor = { 1.0f, 1.0f, 1.0f, 1.0f }, int aNumLines = 8);

		static void SetGridActive(bool aActive = true);
		static bool GetGridActive();
		static void SetDebugLinesActive(bool aActive = true);
		static void SetOutlineActive(bool aActive = true);

		//Returns an id if an entity was found, 0 if no entity was found
		static uint64_t GetEntityFromScreenPos(int aX, int aY);
		static uint64_t GetEntityFromScreenPos(RenderSceneId aId, int aX, int aY);

		static std::array<Ref<Texture2D>, 16> GetShadowMap(uint32_t aID);

		static RenderSceneId InitializeScene(bool aIsOneshot = false);
		static void BeginScene(RenderSceneId aId);
		static void EndScene();

		static void Initialize();
		static void Begin();
		static void Sync();
		static void Shutdown();

	private:
		static Ref<Texture2D> GetEnvi();
		// used to ensure that threaded and non threaded scenes are the same.
		static void InternalBegin();

		static void SortCommands(Cache* aCache);

		static Ref<Mesh> GetDefaultCube();

		static void SortMeshCmd(std::vector<MeshSubmitInfo>& aCmdList);
		static void SortMeshCmdByDepth(std::vector<MeshSubmitInfo>& aCmdList);

		static Cache* GetCacheFromID(RenderSceneId aId);
		static Cache* GetCacheFromID(RenderSceneId aId, size_t aFrameOffset);

		static void InitializeCache(Cache* aCache, RenderSceneId aId);

		static void InternalInitialize(RenderSceneId aId);

		static void DrawSceneWithID(RenderSceneId aId);
		static void InitializeShaders();
		static void InitializeLineBatch(Cache* aCache, RenderSceneId aId = 0);

		static bool CheckIfShaderIsDeferred(const size_t& pipelineID);

		static void RenderScenes();
		
		static void CollectDrawCallsFromList(Draw aDrawID, std::vector<MeshBatch>& aMeshBatch, std::vector<MeshSubmitInfo>& aCommandList, std::function<bool(size_t, size_t, size_t, MeshSubmitInfo&, MeshSubmitInfo&)>&& aFilterLambda, int32_t aOverrideLOD = -1, std::function<bool(MeshSubmitInfo&)>&& aNotDrawFilter = 0);
		
		static bool CompileMeshBatches(Cache* aCache, ID3D11DeviceContext* aContext);

		static void RenderGeometry(Cache* aCache, Draw aDraw, Binding aBinding, ID3D11DeviceContext* aContext);

		// Atmospheric Sky
		static void UpdateAtmosphereBuffers(Cache* aCache, ID3D11DeviceContext* aContext);
		static void UpdateSkyHelperBuffers(Cache* aCache, ID3D11DeviceContext* aContext);
		
		// Draw funcs
		static void DrawSprite(const SpriteInfo& aInfo, Cache* aCache, ID3D11DeviceContext* aContext);
		static void DrawTextData(TextInfo& aInfo);
		static void DrawLineVertex(Cache* aCache, LineVertex& vertex, ID3D11DeviceContext* aContext);
		static void DrawLine2DVertex(Cache* aCache, LineVertex& vertex, ID3D11DeviceContext* aContext);
		static void DrawBillboard(Cache* aCache, BillboardInfo& aInfo, ID3D11DeviceContext* aContext);

		static void InitializeSpriteBatch(Cache* aCache, RenderSceneId aId = 0);
		static void InitializeTextBatch(Cache* aCache, RenderSceneId aId = 0);
		static void InitializeBillboardBatch(Cache* aCache, RenderSceneId aId = 0);

		static void BeginSpriteBatch(Cache* aCache);
		static void EndSpriteBatch(Cache* aCache, ID3D11DeviceContext* aContext);

		static void BeginLineBatch(Cache* aCache);
		static void BeginLine2DBatch(Cache* aCache);
		static void EndLineBatch(Cache* aCache, ID3D11DeviceContext* aContext);
		static void EndLine2DBatch(Cache* aCache, ID3D11DeviceContext* aContext);

		static void BeginTextBatch(Cache* aCache);
		static void EndTextBatch(Cache* aCache, ID3D11DeviceContext* aContext);

		static void BeginBillboardBatch(Cache* aCache);
		static void EndBillboardBatch(Cache* aCache, ID3D11DeviceContext* aContext);
		//\\//\\//\\//\\//\\

		static Draw ExtractDrawFlagFromIndex(Firefly::Draw aDraw, const size_t aIndex);

		/// <summary>
		/// This function will not work if you want to copy with framebuffers that does not have a depth buffer.
		/// This is true for both aFrom and aTo.
		/// This will fully copy the depth using a simple copying shader.
		/// </summary>
		/// <param name="aFrom"></param>
		/// <param name="aTo"></param>
		static void CopyDepth(Ref<Framebuffer> aFrom, Ref<Framebuffer> aTo, ID3D11DeviceContext* aContext);
		static void HandleFramebuffer(Ref<Framebuffer> aFB, RenderSceneId aId = 0, float aScale = 1.0f);
		static void InitalizeHBAO();
	};
	void ExtractLodLevel(Firefly::Cache* cache, Firefly::MeshSubmitInfo* cmd, bool& retflag);
}
