#include "FFpch.h"
#include "ParticleEmitter.h"

#include "Firefly/Asset/ResourceCache.h"
#include "Utils/InputHandler.h"
#include "Utils/Timer.h"
#include "Utils/Math/Random.hpp"
#include "Firefly/Rendering/RenderCommands.h"
#include "MeshEmissionController.h"

namespace Firefly
{
	void ParticleEmitter::Initialize(const ParticleEmitterTemplate& aTemplate, bool aStart, bool aPrewarm, bool aForcePoolSize, int aForcedPoolSize)
	{
		myEmitterSettings = aTemplate.EmitterSettings;

		if (aForcePoolSize)
		{
			myMaxParticles = aForcedPoolSize;
		}
		else
		{
			constexpr float particlePoolSizeMult = 1.5f;
			myMaxParticles = static_cast<size_t>(ceilf(myEmitterSettings.SpawnRate * myEmitterSettings.MaxLifeTime) * particlePoolSizeMult + 2);
		}

		myParticles.clear();
		myParticlePool.Clear();
		myVertexBuffer = nullptr;
		myTexture = nullptr;
		myMaterial = nullptr;
		myMeshEmissionController = nullptr;

		myParticles.resize(myMaxParticles);
		myParticlePool.Initialize(myMaxParticles);
		FillParticlePool();

		myTotalEmitTime = 0.0f;
		myIsEmitting = aStart;

		if (aPrewarm)
		{
			mySpawnTimer = -myEmitterSettings.MaxLifeTime;
		}
		else
		{
			mySpawnTimer = 0;
		}

		//Create Vertexbuffer
		if (!myVertexBuffer)
		{
			VertexBufferInfo bufferInfo;
			bufferInfo.Data = myParticles.data();
			bufferInfo.Count = myMaxParticles;
			bufferInfo.ObjectSize = sizeof(ParticleVertex);
			myVertexBuffer = VertexBuffer::Create(bufferInfo);
		}
		//

		myTexture = ResourceCache::GetAsset<Texture2D>(aTemplate.TexturePath);
		myMaterial = ResourceCache::GetAsset<MaterialAsset>(aTemplate.MaterialPath);

		myMeshEmissionController = CreateRef<MeshEmissionController>();

		if (myEmitterSettings.EmitterType == EmitterType::Mesh)
		{
			if (myMeshEmissionController)
			{
				myMeshEmissionController->SetNewMesh(aTemplate.MeshPath, true);
			}
		}
	}

	void ParticleEmitter::FillParticlePool()
	{
		myParticlePool.Clear();

		for (size_t i = 0; i < myParticles.size(); ++i)
		{
			AddParticleToPool(i);
		}
	}

	void ParticleEmitter::StartPause()
	{
		myIsEmitting = !myIsEmitting;
	}

	void ParticleEmitter::Start()
	{
		myIsEmitting = true;
	}

	void ParticleEmitter::Pause()
	{
		myIsEmitting = false;
	}

	void ParticleEmitter::ClearParticles()
	{
		FillParticlePool();
		myTotalEmitTime = 0.0f;
		mySpawnTimer = 0.0f;
	}

	void ParticleEmitter::Update(const Utils::BasicTransform& aEntityTransform, const std::vector<ForceField>& aGlobalForceFields)
	{
		FF_PROFILESCOPE("Particle Emitter Update");

		myEntityPosition = aEntityTransform.GetPosition();
		myEntityRotationMatrix = Utils::Matrix3f::CreateRotationMatrix(aEntityTransform.GetRotation());
		myEntityScale = aEntityTransform.GetScale();

		const float deltaTime = myEmitterSettings.ScaledDeltaTime ? Utils::Timer::GetDeltaTime() : Utils::Timer::GetUnscaledDeltaTime();

		myEmitTimeToMaxRadius = (myEmitterSettings.RadiusMax - myEmitterSettings.Radius) / myEmitterSettings.RadiusChangeSpeed;
		myEmitTimeToMaxInnerRadius = (myEmitterSettings.InnerRadiusMax - myEmitterSettings.InnerRadius) / myEmitterSettings.InnerRadiusChangeSpeed;
		myEmitTimeToMaxOuterRadius = (myEmitterSettings.OuterRadiusMax - myEmitterSettings.OuterRadius) / myEmitterSettings.OuterRadiusChangeSpeed;

		if (!myEmitterSettings.Looping && myTotalEmitTime > myEmitterSettings.MaxEmitTime)
		{
			myTotalEmitTime = 0.0f;
			myIsEmitting = false;
		}

		if (myIsEmitting)
		{
			myTotalEmitTime += deltaTime;
		}

		myHasAliveParticles = myParticlePool.GetSize() != myMaxParticles;

		if (myHasAliveParticles)
		{
			float flipbookCellSizeX = 1.0f;
			float flipbookCellSizeY = 1.0f;

			if (myEmitterSettings.IsFlipbook)
			{
				if (myEmitterSettings.Columns != 0) flipbookCellSizeX = 1.0f / myEmitterSettings.Columns;
				if (myEmitterSettings.Rows != 0) flipbookCellSizeY = 1.0f / myEmitterSettings.Rows;
			}

			for (size_t i = 0; i < myParticles.size(); ++i)
			{
				ParticleVertex& particle = myParticles[i];

				if (!particle.Dead)
				{
					particle.LifeTime -= deltaTime;

					//Particle died, add it to the pool and mark it as dead
					if (particle.LifeTime <= 0.0f)
					{
						AddParticleToPool(i);
					}
					else
					{
						const float lifePercentage = (particle.TotalLifeTime - particle.LifeTime) / particle.TotalLifeTime;

						if (myEmitterSettings.UseAcceleration)
						{
							particle.Speed.x += myEmitterSettings.Acceleration.x * deltaTime;
							particle.Speed.y += myEmitterSettings.Acceleration.y * deltaTime;
							particle.Speed.z += myEmitterSettings.Acceleration.z * deltaTime;

							if (particle.Speed.Length() > myEmitterSettings.MaxSpeed)
							{
								particle.Speed = particle.Speed.GetNormalized() * myEmitterSettings.MaxSpeed;
							}
						}

						const Utils::Vector3f particlePos = Utils::Vec4ToVec3(particle.Position);

						//Local fields
						for (const auto& fieldVector :{ std::cref(myEmitterSettings.LocalForceFields), std::cref(aGlobalForceFields) })
						{
							for (const auto& forceField : fieldVector.get())
							{
								float fieldRange = forceField.Range;
								Utils::Vector3f fieldPos = forceField.Position;

								if (!forceField.Global)
								{
									if (myShouldScale)
									{
										fieldPos *= myEntityScale;
										fieldRange *= Utils::Max(Utils::Max(myEntityScale.x, myEntityScale.y), myEntityScale.z);
									}

									if (myShouldRotate)
									{
										fieldPos = fieldPos * myEntityRotationMatrix;
									}

									fieldPos += myEntityPosition;
								}

								if (forceField.ForceFieldType == ForceFieldType::Point)
								{
									const Utils::Vector3f dirToField = fieldPos - particlePos;
									const float distToField = dirToField.Length();

									if (distToField < fieldRange)
									{
										const float force = forceField.Lerp ? Utils::LerpByType(forceField.LerpType, 0.0f, forceField.Force, distToField / fieldRange, forceField.LerpPower) : forceField.Force;
										particle.Speed += dirToField.GetNormalized() * ((fieldRange - distToField) / fieldRange) * force * deltaTime;
									}
								}
								else if (forceField.ForceFieldType == ForceFieldType::Cuboid)
								{
									if (particlePos.x < fieldPos.x + fieldRange &&
										particlePos.x > fieldPos.x - fieldRange &&
										particlePos.y < fieldPos.y + fieldRange &&
										particlePos.y > fieldPos.y - fieldRange &&
										particlePos.z < fieldPos.z + fieldRange &&
										particlePos.z > fieldPos.z - fieldRange)
									{
										//TODO: LERP CUBOID FIELDS
										//const float force = Utils::LerpByType(forceField.LerpType, 0.0f, forceField.Force, distToField / forceField.Range, forceField.LerpPower);

										particle.Speed += forceField.Direction.GetNormalized() * forceField.Force * deltaTime;
									}
								}
							}
						}

						particle.Position.x += particle.Speed.x * deltaTime;
						particle.Position.y += particle.Speed.y * deltaTime;
						particle.Position.z += particle.Speed.z * deltaTime;

						particle.Rotation -= myEmitterSettings.RotationSpeed * deltaTime;

						particle.Scale = Utils::Vector3f::Lerp(myEmitterSettings.StartScale, myEmitterSettings.EndScale, lifePercentage) * particle.ScaleVariation;

						//particle.Color = Vector4f::Lerp(myEmitterSettings.StartColor, myEmitterSettings.EndColor, lifePercentage);
						particle.Color = myEmitterSettings.ColorGradient.GetCombinedColor(lifePercentage);

						if (myEmitterSettings.IsFlipbook)
						{
							int frame = 0;
							int row = 0;
							int column = 0;

							//Lifetime flipbook
							if (myEmitterSettings.FrameRate == 0)
							{
								frame = myEmitterSettings.Frames * lifePercentage;
								row = frame / myEmitterSettings.Columns;
								column = frame % myEmitterSettings.Columns;
							}
							//Framerate based
							else
							{
								frame = static_cast<int>(myEmitterSettings.FrameRate * (particle.TotalLifeTime - particle.LifeTime)) % myEmitterSettings.Frames;

								row = frame / myEmitterSettings.Columns;
								column = frame % myEmitterSettings.Columns;
							}

							const float uvTopLeftX = flipbookCellSizeX * column;
							const float uvTopLeftY = flipbookCellSizeY * row;

							particle.UVs[0][0] = uvTopLeftX;
							particle.UVs[0][1] = uvTopLeftY;
							particle.UVs[1][0] = uvTopLeftX + flipbookCellSizeX;
							particle.UVs[1][1] = uvTopLeftY;
							particle.UVs[2][0] = uvTopLeftX;
							particle.UVs[2][1] = uvTopLeftY + flipbookCellSizeY;
							particle.UVs[3][0] = uvTopLeftX + flipbookCellSizeX;
							particle.UVs[3][1] = uvTopLeftY + flipbookCellSizeY;
						}
					}
				}
			}
		}

		if (myIsEmitting && !Utils::IsAlmostEqual(myEmitterSettings.SpawnRate, 0.0f, 0.001f))
		{
			//Fix spawntimer == inf
			if (mySpawnTimer > 1 / myEmitterSettings.SpawnRate)
			{
				mySpawnTimer = 1 / myEmitterSettings.SpawnRate;
			}

			if (mySpawnTimer < -1.0f)
			{
				mySpawnTimer = 0.0f;
			}

			if (myEmitterSettings.MinLifeTime >= 0.0f && myEmitterSettings.MaxLifeTime > 0.0f)
			{
				//Spawn new particle, multiple if 1 particle/frame is not enough
				while (mySpawnTimer <= 0)
				{
					mySpawnTimer += 1 / myEmitterSettings.SpawnRate;

					if (!myParticlePool.IsEmpty())
					{
						size_t particleToSpawn = myParticlePool.Dequeue();

						float extraLifeTime = -mySpawnTimer;
						extraLifeTime = fmodf(extraLifeTime, myEmitterSettings.MaxLifeTime);

						InitParticle(particleToSpawn, Utils::Max(extraLifeTime, 0.0f));
					}
				}
			}

			mySpawnTimer -= deltaTime;
		}
	}

	bool ParticleEmitter::IsInitialized() const
	{
		return myMaxParticles > 0 && myVertexBuffer && myTexture && myTexture->IsLoaded();
	}

	void ParticleEmitter::Bind(ID3D11DeviceContext* aContext) const
	{
		if (ReadyToRender())
		{
			myVertexBuffer->SetData(myParticles.data(), myParticles.size() * sizeof(ParticleVertex), aContext);
			myVertexBuffer->Bind(aContext);
			myTexture->Bind(0, ShaderType::Pixel, aContext);
		}
	}

	void ParticleEmitter::Draw(ID3D11DeviceContext* aContext) const
	{
		if (ReadyToRender())
		{
			globalRendererStats.DrawCalls++;
			aContext->Draw(static_cast<UINT>(myParticles.size()), 0);
		}
	}

	void ParticleEmitter::LoadNewMaterial(const std::filesystem::path& aMaterialPath)
	{
		std::string path = aMaterialPath.string();

		if (aMaterialPath.empty())
		{
			LOGWARNING("No Particle material given! Setting to default");
			path = DefaultParticleMaterialPath;
		}

		myMaterial = ResourceCache::GetAsset<MaterialAsset>(path);
	}

	void ParticleEmitter::LoadNewTexture(const std::filesystem::path& aTexturePath)
	{
		myTexture = ResourceCache::GetAsset<Texture2D>(aTexturePath);
	}

	void ParticleEmitter::InitParticle(size_t aParticleIndex, float aExtraLifeTime)
	{
		ParticleVertex& particle = myParticles[aParticleIndex];

		particle.Position = myEntityPosition;
		particle.Position.w = 1.0f;

		if (myEmitterSettings.RandomSpawnRotation)
		{
			particle.Rotation = Utils::RandomAngle();
		}
		else
		{
			particle.Rotation = 0.0f;
		}

		particle.Color = { 0.0f, 0.0f, 0.0f, 0.0f };

		particle.SpeedVariation = Utils::RandomFloat(1, myEmitterSettings.SpeedVariation);

		//Set speed & spawnpos
		{
			Utils::Vector3f spawnPos;
			Utils::Vector3f targetPos;

			switch (myEmitterSettings.EmitterType)
			{
			case EmitterType::Cone:
			{
				const float innerRadius = GetInnerRadius();

				const Utils::Vector2f offset =
					myEmitterSettings.SpawnOnEdge ?
					Utils::RandomPointOnCircleEdge(innerRadius) :
					Utils::RandomPointInCircle(innerRadius);

				Utils::Vector2f outerOffset = offset.GetNormalized() * GetOuterRadius();

				if (!myEmitterSettings.AimForEdge)
				{
					outerOffset = Utils::RandomFloat(0, 1) * (outerOffset - offset) + offset;
				}

				spawnPos = { offset.x, 0, offset.y };
				targetPos = { outerOffset.x, myEmitterSettings.Height, outerOffset.y };
				break;
			}
			case EmitterType::Sphere:
			{
				const float sphereRadius = GetSphereRadius();

				Utils::Vector3f offset =
					myEmitterSettings.SpawnOnSurface ?
					Utils::RandomPointOnSphereSurface(sphereRadius) :
					Utils::RandomPointInSphere(sphereRadius);

				spawnPos = offset;
				targetPos = spawnPos * 2.0f;
				break;
			}
			case EmitterType::Rectangle:
			{
				const float innerWidth = GetInnerRectangleWidth();
				const float innerHeight = GetInnerRectangleHeight();
				const float outerWidth = GetOuterRectangleWidth();
				const float outerHeight = GetOuterRectangleHeight();

				Utils::Vector2f offset =
					myEmitterSettings.SpawnOnEdge ?
					Utils::RandomPointOnRectangleEdge(innerWidth, innerHeight) :
					Utils::RandomPointInRectangle(innerWidth, innerHeight);

				//Center the rectangle to 0,0
				offset.x -= innerWidth * 0.5f;
				offset.y -= innerHeight * 0.5f;

				spawnPos = { offset.x, 0, offset.y };
				targetPos = { (offset.x / innerWidth) * outerWidth, myEmitterSettings.Height, (offset.y / innerHeight) * outerHeight };
				break;
			}
			case EmitterType::Mesh:
			case EmitterType::AnimatedMesh:
			{
				if (myMeshEmissionController && myMeshEmissionController->IsInitialized())
				{
					const RandomMeshPointInfo randomPoint = myMeshEmissionController->GetRandomPointOnMesh();
					spawnPos = randomPoint.Position + randomPoint.Normal * myEmitterSettings.MeshNormalOffset;
					targetPos = spawnPos + randomPoint.Normal;
				}

				break;
			}
			}

			if (myEmitterSettings.Global)
			{
				if (myShouldScale)
				{
					spawnPos *= myEntityScale;
					targetPos *= myEntityScale;
				}

				if (myShouldRotate)
				{
					spawnPos = spawnPos * myEntityRotationMatrix;
					targetPos = targetPos * myEntityRotationMatrix;
				}
			}

			const Utils::Vector3f direction = (targetPos - spawnPos).GetNormalized();
			particle.Speed = direction * myEmitterSettings.Speed * particle.SpeedVariation;

			if (myEmitterSettings.UseAcceleration)
			{
				particle.Speed.x += myEmitterSettings.Acceleration.x * aExtraLifeTime;
				particle.Speed.y += myEmitterSettings.Acceleration.y * aExtraLifeTime;
				particle.Speed.z += myEmitterSettings.Acceleration.z * aExtraLifeTime;

				if (particle.Speed.Length() > myEmitterSettings.MaxSpeed)
				{
					particle.Speed = particle.Speed.GetNormalized() * myEmitterSettings.MaxSpeed;
				}
			}

			particle.Position.x += spawnPos.x;
			particle.Position.y += spawnPos.y;
			particle.Position.z += spawnPos.z;

			particle.Position.x += particle.Speed.x * aExtraLifeTime;
			particle.Position.y += particle.Speed.y * aExtraLifeTime;
			particle.Position.z += particle.Speed.z * aExtraLifeTime;
		}

		particle.UVs[0][0] = 0.0f;
		particle.UVs[0][1] = 0.0f;
		particle.UVs[1][0] = 1.0f;
		particle.UVs[1][1] = 0.0f;
		particle.UVs[2][0] = 0.0f;
		particle.UVs[2][1] = 1.0f;
		particle.UVs[3][0] = 1.0f;
		particle.UVs[3][1] = 1.0f;

		particle.ScaleVariation = Utils::RandomFloat(1, myEmitterSettings.ScaleVariation);
		particle.Scale = myEmitterSettings.StartScale * particle.ScaleVariation;

		particle.TotalLifeTime = Utils::RandomFloat(myEmitterSettings.MinLifeTime, myEmitterSettings.MaxLifeTime);
		particle.LifeTime = particle.TotalLifeTime - aExtraLifeTime;

		particle.Dead = false;
	}

	void ParticleEmitter::AddParticleToPool(size_t aParticleIndex)
	{
		myParticles[aParticleIndex].Dead = true;
		myParticles[aParticleIndex].Color.w = 0.0f;

		//memset(myParticles[aParticleIndex].UVs, 0, 8 * sizeof(float));

		myParticlePool.EnqueueUnsafe(aParticleIndex);
	}

	float ParticleEmitter::GetInnerRadius() const
	{
		if (myIsEmitting && myEmitterSettings.InnerRadiusChangeSpeed != 0.0f)
		{
			float time;

			if (myEmitterSettings.LoopInnerChangeSpeed)
			{
				if (myEmitterSettings.BounceRadiusChange)
				{
					time = (1 - Utils::Abs(fmodf(myTotalEmitTime, 2 * myEmitTimeToMaxInnerRadius) / (myEmitTimeToMaxInnerRadius)-1)) * myEmitTimeToMaxInnerRadius;
				}
				else
				{
					time = fmodf(myTotalEmitTime, myEmitTimeToMaxInnerRadius);
				}
			}
			else
			{
				time = myTotalEmitTime;
			}

			return Utils::Min(Utils::Abs(myEmitterSettings.InnerRadius + myEmitterSettings.InnerRadiusChangeSpeed * time), myEmitterSettings.InnerRadiusMax);
		}
		else
		{
			return myEmitterSettings.InnerRadius;
		}
	}

	float ParticleEmitter::GetOuterRadius() const
	{
		if (myIsEmitting && myEmitterSettings.OuterRadiusChangeSpeed != 0.0f)
		{
			float time;

			if (myEmitterSettings.LoopOuterChangeSpeed)
			{
				if (myEmitterSettings.BounceRadiusChange)
				{
					time = (1 - Utils::Abs(fmodf(myTotalEmitTime, 2 * myEmitTimeToMaxOuterRadius) / (myEmitTimeToMaxOuterRadius)-1)) * myEmitTimeToMaxOuterRadius;
				}
				else
				{
					time = fmodf(myTotalEmitTime, myEmitTimeToMaxOuterRadius);
				}
			}
			else
			{
				time = myTotalEmitTime;
			}

			return Utils::Min(Utils::Abs(myEmitterSettings.OuterRadius + myEmitterSettings.OuterRadiusChangeSpeed * time), myEmitterSettings.OuterRadiusMax);
		}
		else
		{
			return myEmitterSettings.OuterRadius;
		}
	}

	float ParticleEmitter::GetSphereRadius() const
	{
		if (myIsEmitting && myEmitterSettings.RadiusChangeSpeed != 0.0f)
		{
			float time;

			if (myEmitterSettings.LoopInnerChangeSpeed)
			{
				if (myEmitterSettings.BounceRadiusChange)
				{
					time = (1 - Utils::Abs(fmodf(myTotalEmitTime, 2 * myEmitTimeToMaxRadius) / (myEmitTimeToMaxRadius)-1)) * myEmitTimeToMaxRadius;
				}
				else
				{
					time = fmodf(myTotalEmitTime, myEmitTimeToMaxRadius);
				}
			}
			else
			{
				time = myTotalEmitTime;
			}

			return Utils::Min(Utils::Abs(myEmitterSettings.Radius + myEmitterSettings.RadiusChangeSpeed * time), myEmitterSettings.RadiusMax);
		}
		else
		{
			return myEmitterSettings.Radius;
		}
	}

	float ParticleEmitter::GetInnerRectangleWidth() const
	{
		return myEmitterSettings.InnerRectangleWidth;
	}

	float ParticleEmitter::GetInnerRectangleHeight() const
	{
		return myEmitterSettings.InnerRectangleHeight;
	}

	float ParticleEmitter::GetOuterRectangleWidth() const
	{
		return myEmitterSettings.OuterRectangleWidth;
	}

	float ParticleEmitter::GetOuterRectangleHeight() const
	{
		return myEmitterSettings.OuterRectangleHeight;
	}

	bool ParticleEmitter::ReadyToRender() const
	{
		return myHasAliveParticles && myVertexBuffer && myTexture && myTexture->IsLoaded() && myMaterial && myMaterial->IsLoaded();
	}
}