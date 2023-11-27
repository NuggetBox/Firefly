#include "FFpch.h"
#include "ParticleEmitterComponent.h"

#include <Firefly/Application/Application.h>
#include <Firefly/Asset/ResourceCache.h>
#include <Firefly/Components/Mesh/AnimatedMeshComponent.h>
#include <Firefly/ComponentSystem/Entity.h>
#include <Firefly/Event/EntityEvents.h>
#include <Firefly/Rendering/Renderer.h>
#include <Utils/Math/Random.hpp>

#include "ForceFieldManager.h"

#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/ParticleSystem/MeshEmissionController.h"
#include "Firefly/Rendering/ParticleSystem/ParticleEmitter.h"

namespace Firefly
{
	REGISTER_COMPONENT(ParticleEmitterComponent);

	ParticleEmitterComponent::ParticleEmitterComponent() : Component("ParticleEmitterComponent")
	{
		myParticleEmitter = CreateRef<ParticleEmitter>();

		EditorVariable("Emitter Template Path", Firefly::ParameterType::File, &myEmitterTemplatePath, ".emitter");

		//Keep this here, using index 1 to hide this variable
		EditorVariable("Animated Mesh Entity", ParameterType::Entity, &myAnimatedMeshEntity);

		EditorVariable("Emit On Start", Firefly::ParameterType::Bool, &myEmitOnStart);
		EditorVariable("Prewarm", Firefly::ParameterType::Bool, &myPrewarm);
		EditorVariable("Should Rotate with Entity", ParameterType::Bool, &myShouldRotate);
		EditorVariable("Should Scale with Entity", ParameterType::Bool, &myShouldScale);
		EditorButton("Start", [&]() { myParticleEmitter->Start(); });
		EditorButton("Pause", [&]() { myParticleEmitter->Pause(); });
		EditorButton("Reset", [&]() { myParticleEmitter->ClearParticles(); myParticleEmitter->Start(); });
	}

	void ParticleEmitterComponent::Initialize()
	{
		Load();
		myParticleEmitter->SetShouldRotateWithEntity(myShouldRotate);
		myParticleEmitter->SetShouldScaleWithEntity(myShouldScale);
	}

	void ParticleEmitterComponent::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent&)
		{
			if (!myEmitterLoaded)
			{
				if (myEmitterTemplate && myEmitterTemplate->IsLoaded())
				{
					myParticleEmitter->Initialize(*myEmitterTemplate, myEmitOnStart, myPrewarm);

					if (myParticleEmitter->GetEmitterSettings().EmitterType == EmitterType::AnimatedMesh)
					{
						LoadAnimatedMesh();
					}

					myEmitterLoaded = true;
				}
				else
				{
					return false;
				}
			}

			if (myParticleEmitter && myEntity)
			{
#ifndef FF_SHIPIT
				DrawDebugBillboard();
#endif

				if (myParticleEmitter->IsInitialized())
				{
					//Spawn particles in global space
					if (myParticleEmitter->GetIsGlobal())
					{
						auto parent = myEntity->GetParent();
						myParticleEmitter->Update(myEntity->GetTransform().ToBasic(), ForceFieldManager::Get().GetForceFields());
						ParticleEmitterCommand command;
						command.ParentTransform = Utils::Matrix4f();
						command.Emitter = myParticleEmitter;
						command.ShouldCull = false;
						Renderer::Submit(command);
					}
					//Spawn particles in local space
					else
					{
						myParticleEmitter->Update(Utils::BasicTransform(), ForceFieldManager::Get().GetForceFields());
						ParticleEmitterCommand command;
						command.ParentTransform = myEntity->GetTransform().GetMatrix();
						command.Emitter = myParticleEmitter;
						command.ShouldCull = false;
						Renderer::Submit(command);
					}
				}
			}

			return false;
		});

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent&)
		{
			if (myEmitterLoaded)
			{
				if (myParticleEmitter && myParticleEmitter->GetEmitterSettings().EmitterType == EmitterType::AnimatedMesh)
				{
					if (!myAnimatedMeshEntity.expired())
					{
						if (const auto& animMeshEntity = myAnimatedMeshEntity.lock())
						{
							const auto& animMeshCompWeak = animMeshEntity->GetComponent<AnimatedMeshComponent>();

							if (!animMeshCompWeak.expired())
							{
								const auto& animMeshComp = animMeshCompWeak.lock();

								if (animMeshComp)
								{
									if (myParticleEmitter->GetMeshEmissionController())
									{
										if (myParticleEmitter->GetMeshEmissionController()->IsInitialized())
										{
											myParticleEmitter->GetMeshEmissionController()->UpdateAnimatedBoneTransforms(animMeshComp->GetCurrentBoneTransforms());
										}
										else
										{
											LOGERROR("Mesh emission controller not initialized");
										}
									}
									else
									{
										LOGERROR("Mesh emission controller nullptr");
									}
								}
							}
							else
							{
								LOGERROR("Animated Mesh Entity has no animated mesh component");
							}
						}
					}
					else
					{
						LOGERROR("Animated Mesh Entity Nullptr");
					}
				}
			}

			return false;
		});

		dispatcher.Dispatch<EmitterUpdatedEvent>([&](EmitterUpdatedEvent& event)
		{
			if (myEmitterTemplatePath == event.GetPath())
			{
				Load(true);
			}
			return false;
		});

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& event)
		{
			if (event.GetParamType() == ParameterType::File)
			{
				Load();
			}

			if (event.GetParamType() == ParameterType::Entity)
			{
				if (myParticleEmitter->GetEmitterSettings().EmitterType == EmitterType::AnimatedMesh)
				{
					LoadAnimatedMesh();
				}
			}

			myParticleEmitter->SetShouldRotateWithEntity(myShouldRotate);
			myParticleEmitter->SetShouldScaleWithEntity(myShouldScale);

			return false;
		});
	}

	bool ParticleEmitterComponent::LoadAnimatedMesh()
	{
		Ptr<AnimatedMeshComponent> animMeshCompWeak;

		if (myAnimatedMeshEntity.expired())
		{
			if (myEntity->HasComponent<AnimatedMeshComponent>())
			{
				myAnimatedMeshEntity = GetEntityWithID(myEntity->GetID());
				animMeshCompWeak = myEntity->GetComponent<AnimatedMeshComponent>();
			}
			else
			{
				LOGWARNING("Entity {} has no given animated mesh or animated mesh on itself", myEntity->GetName());
			}
		}
		else
		{
			const auto& animMeshEntity = myAnimatedMeshEntity.lock();
			animMeshCompWeak = animMeshEntity->GetComponent<AnimatedMeshComponent>();
		}

		if (!animMeshCompWeak.expired())
		{
			if (myParticleEmitter->GetMeshEmissionController())
			{
				const auto& animMeshComp = animMeshCompWeak.lock();
				myParticleEmitter->GetMeshEmissionController()->SetNewAnimatedMesh(animMeshComp->GetMeshPath(), true);
				myAnimatedMeshLoaded = true;
				return true;
			}
			else
			{
				LOGERROR("Mesh emission controller nullptr");
			}
		}
		else
		{
			LOGERROR("Animated Mesh Entity has no animated mesh component");
		}

		myAnimatedMeshLoaded = false;
		return false;
	}

	void ParticleEmitterComponent::LoadDebugBillboard(bool aLoadNow)
	{
#ifndef FF_SHIPIT
		myDebugBillboard = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_emitter.dds", aLoadNow);
		myDebugBillboardInfo = CreateRef<BillboardInfo>();
		myDebugBillboardInfo->Texture = myDebugBillboard;
		myDebugBillboardInfo->Color = { 1, 1, 1, 1 };
#endif
	}

	void ParticleEmitterComponent::DrawDebugBillboard()
	{
#ifndef FF_SHIPIT
		if (!Application::Get().GetIsInPlayMode())
		{
			if (myDebugBillboardInfo)
			{
				myDebugBillboardInfo->Position = myEntity->GetTransform().GetPosition();
				myDebugBillboardInfo->EntityID = myEntity->GetID();
				Renderer::Submit(*myDebugBillboardInfo);
			}
		}
#endif
	}

	void ParticleEmitterComponent::Load(bool aLoadNow)
	{
		myEmitterLoaded = false;

		if (!myEmitterTemplatePath.empty())
		{
			myEmitterTemplate = ResourceCache::GetAsset<ParticleEmitterTemplate>(myEmitterTemplatePath, aLoadNow);
		}

#ifndef FF_SHIPIT
		LoadDebugBillboard();
#endif
	}
}