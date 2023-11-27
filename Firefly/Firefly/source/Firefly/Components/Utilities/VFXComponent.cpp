#include "FFpch.h"
#include "VFXComponent.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Asset/Material/MaterialAsset.h"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Components/Mesh/MeshComponent.h"

REGISTER_COMPONENT(VFXComponent);

VFXComponent::VFXComponent() :
	Component("VFXComponent")
{
	EditorVariable("Lifetime", Firefly::ParameterType::Float, &myTime);
	EditorVariable("Should Blend", Firefly::ParameterType::Bool, &myShouldBlend);
}

VFXComponent::~VFXComponent()
{
	Utils::TimerManager::RemoveTimer(myTimerID);
}

void VFXComponent::Initialize()
{
	if (!Firefly::Application::Get().GetIsInPlayMode())
	{
		return;
	}

	if (!myEntity)
	{
		return;
	}

	uint64_t id = myEntity->GetID();

	auto timer = Utils::TimerManager::AddTimer([id]()
		{
			Firefly::QueueDeleteEntity(id);
		},
		myTime);

	myTimerID = timer->TimerID;

	if (myShouldBlend)
	{
		auto mesh = myEntity->GetComponent<Firefly::MeshComponent>();
		if (!mesh.expired())
		{
			for (auto mats : mesh.lock()->GetMaterials())
			{
				mats->SetShouldBlend(true);
			}
		}
		for (auto child : myEntity->GetChildren())
		{
			if (child.expired())
			{
				continue;
			}

			auto mesh = child.lock()->GetComponent<Firefly::MeshComponent>();
			if (mesh.expired())
			{
				continue;
			}

			for (auto mats : mesh.lock()->GetMaterials())
			{
				mats->SetShouldBlend(true);
			}
		}
	}
}