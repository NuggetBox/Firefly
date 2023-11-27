#include "FFpch.h"
#include "DecalComponent.h"

#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"

#include <Firefly/Asset/ResourceCache.h>
#include <Firefly/Asset/Material/MaterialAsset.h>
#include <Firefly/Event/Event.h>

#include <Firefly/Event/ApplicationEvents.h>
#include <Firefly/Rendering/RenderCommands.h>
#include <Firefly/Rendering/Renderer.h>
#include <Firefly/Event/EntityEvents.h>
namespace Firefly
{
	REGISTER_COMPONENT(DecalComponent);
	const Utils::Vec3 ScaleMover(100.f, 100.f, 100.f);
	DecalComponent::DecalComponent() : Component("DecalComponent")
	{
		myMaterialPath = "";
		EditorVariable("DecalMaterial", ParameterType::File, &myMaterialPath, ".mat");
	}

	void DecalComponent::Initialize()
	{
		LoadMaterial();
	}

	void DecalComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				
					LoadMaterial();
				return false;
			});

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent& aEvent) -> bool
			{
				auto& transform = myEntity->GetTransform();

				const auto scaledSize = transform.GetScale() * ScaleMover;

				Renderer::SubmitDebugCuboid(transform, Utils::Vec3(0.f), scaledSize, Utils::Vec4(219 / 255.f, 0.f, 182 / 255.f, 1.f));


				auto arrowBegin = transform.GetPosition() + transform.GetBackward() * (scaledSize / 2.f);
				auto arrowEnd = transform.GetPosition() + transform.GetForward() * (scaledSize / 2.f);

				Renderer::SubmitDebugArrow(arrowBegin, arrowEnd, Utils::Vec4(219 / 255.f, 0.f, 182 / 255.f, 1.f));
				if(myMaterial == nullptr || !myMaterial->IsLoaded())
				{
					return false;
				}

				DecalSubmitInfo decalSubmit = {};
				decalSubmit.Matrix = myEntity->GetTransform().GetMatrix();
				decalSubmit.DecalMaterial = myMaterial;
				decalSubmit.EntityId = myEntity->GetID();

				Renderer::Submit(decalSubmit);

				return false;
			});
	}

	void DecalComponent::LoadMaterial()
	{
		if (myMaterialPath.empty() == false)
		{
			myMaterial = Firefly::ResourceCache::GetAsset<MaterialAsset>(myMaterialPath);
		}
	}
}