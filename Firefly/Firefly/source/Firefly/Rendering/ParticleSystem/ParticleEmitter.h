#pragma once
#include <Firefly/Asset/Texture/Texture2D.h>
#include "Utils/Math/Transform.h"

#include "Utils/Queue.hpp"

#include "EmitterSettings.h"
#include "ParticleVertex.h"
#include "Firefly/Rendering/Buffer/VertexBuffer.h"
#include "Firefly/Asset/Asset.h"

class MeshEmissionController;

namespace Firefly
{
	class MaterialAsset;

	constexpr const char* DefaultParticleMaterialPath = "FireflyEngine/Defaults/ParticleDefault.mat";

	struct ParticleEmitterTemplate : Asset
	{
		~ParticleEmitterTemplate() override = default;
		AssetType GetAssetType() const override { return AssetType::EmitterTemplate; }
		static AssetType GetStaticType() { return AssetType::EmitterTemplate; }

		std::filesystem::path TexturePath;
		std::filesystem::path MeshPath;
		std::filesystem::path MaterialPath = DefaultParticleMaterialPath;
		EmitterSettings EmitterSettings;
	};

	class ParticleEmitter
	{
	public:
		ParticleEmitter() = default;
		//ParticleEmitter(Utils::Transform& aTransform);
		void Initialize(const ParticleEmitterTemplate& aTemplate, bool aStart = true, bool aPrewarm = false, bool aForcePoolSize = false, int aForcedPoolSize = 16384);

		void FillParticlePool();
		void StartPause();
		void Start();
		void Pause();
		void ClearParticles();

		void Update(const Utils::BasicTransform& aEntityTransform, const std::vector<ForceField>& aGlobalForceFields = {});
		bool IsInitialized() const;
		bool ReadyToRender() const;

		void Bind(ID3D11DeviceContext* aContext) const;
		void Draw(ID3D11DeviceContext* aContext) const;

		Ref<MaterialAsset> GetMaterial() const { return myMaterial; }

		void LoadNewMaterial(const std::filesystem::path& aMaterialPath);
		void LoadNewTexture(const std::filesystem::path& aTexturePath);

		void SetEmitterSettings(const EmitterSettings& aEmitterSettings) { myEmitterSettings = aEmitterSettings; }
		void SetShouldRotateWithEntity(bool aShouldRotate = true) { myShouldRotate = aShouldRotate; }
		void SetShouldScaleWithEntity(bool aShouldScale = true) { myShouldScale = aShouldScale; }

		const Ref<MeshEmissionController>& GetMeshEmissionController() { return myMeshEmissionController; }

		const EmitterSettings& GetEmitterSettings() const { return myEmitterSettings; }
		bool GetIsGlobal() const { return myEmitterSettings.Global; }

		size_t GetParticleCount() const { return myParticles.size(); }

		float GetInnerRadius() const;
		float GetOuterRadius() const;
		float GetSphereRadius() const;
		float GetInnerRectangleWidth() const;
		float GetInnerRectangleHeight() const;
		float GetOuterRectangleWidth() const;
		float GetOuterRectangleHeight() const;

	private:
		void InitParticle(size_t aParticleIndex, float aExtraLifeTime);
		void AddParticleToPool(size_t aParticleIndex);

		EmitterSettings myEmitterSettings;
		std::vector<ParticleVertex> myParticles;
		size_t myMaxParticles;

		Utils::Queue<size_t> myParticlePool;
		float mySpawnTimer = 0.0;
		float myTotalEmitTime = 0.0f;
		bool myIsEmitting;
		bool myHasAliveParticles = false;

		float myEmitTimeToMaxRadius;
		float myEmitTimeToMaxInnerRadius;
		float myEmitTimeToMaxOuterRadius;

		Ref<VertexBuffer> myVertexBuffer;
		Ref<Texture2D> myTexture;
		Ref<MaterialAsset> myMaterial;

		Ref<MeshEmissionController> myMeshEmissionController;

		Utils::Vector3f myEntityPosition;
		Utils::Matrix3f myEntityRotationMatrix;
		Utils::Vector3f myEntityScale;

		bool myShouldRotate = true;
		bool myShouldScale = true;
	};
}