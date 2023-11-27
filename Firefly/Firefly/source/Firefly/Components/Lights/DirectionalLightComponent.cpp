#include "FFpch.h"
#include "DirectionalLightComponent.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/ComponentSystem/Entity.h"
namespace Firefly
{
	REGISTER_COMPONENT(DirectionalLightComponent);
	DirectionalLightComponent::DirectionalLightComponent() : Component("DirectionalLightComponent")
	{
		myColor = { 1,1,1, 1 };
		myIntensity = 1;
		myEnumNames.push_back("512");
		myEnumNames.push_back("1024");
		myEnumNames.push_back("2048");
		myEnumNames.push_back("4098");
		myEnumNames.push_back("8192");

		//EditorVariable("Test Int", Firefly::ParameterType::Int, &myInt);
		
		EditorVariable("Color", Firefly::ParameterType::Color, &myColor);
		EditorVariable("Intensity", Firefly::ParameterType::Float, &myIntensity);
		EditorVariable("Cast shadows", Firefly::ParameterType::Bool, &myShouldCastShadows);
		EditorVariable("Soft shadows", Firefly::ParameterType::Bool, &myShouldSoftShadows);
		EditorVariable("Resolution", Firefly::ParameterType::Enum, &myShadowResolutions, myEnumNames);

	}
	void DirectionalLightComponent::Initialize()
	{
	}
	void DirectionalLightComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
			{
				if (Renderer::GetActiveCamera())
				{
					//myShouldSoftShadows = true;
					//myShadowResolutions = ShadowResolutions::res2048;

					DirLightPacket packet;
					packet.ColorAndIntensity = { myColor.x, myColor.y, myColor.z, myIntensity };
					packet.Direction = { myEntity->GetTransform().GetForward().x, myEntity->GetTransform().GetForward().y, myEntity->GetTransform().GetForward().z, 0 };

					auto& cameraTransform = Renderer::GetActiveCamera()->GetTransform();
					packet.dirLightInfo.x = static_cast<uint32_t>(myShouldCastShadows);
					packet.dirLightInfo.y = static_cast<uint32_t>(myShouldSoftShadows);
					Renderer::Submit(packet, myShadowResolutions);
					Renderer::SubmitDebugCircle(myEntity->GetTransform().GetPosition(), 15, 25, { myEntity->GetTransform().GetRotation().x + 90,myEntity->GetTransform().GetRotation().y, myEntity->GetTransform().GetRotation().z }, { myColor.x, myColor.y, myColor.z, 1 });
					Renderer::SubmitDebugCircle(myEntity->GetTransform().GetPosition() - myEntity->GetTransform().GetForward() * 50.f, 15, 25, { myEntity->GetTransform().GetRotation().x + 90,myEntity->GetTransform().GetRotation().y, myEntity->GetTransform().GetRotation().z }, { myColor.x, myColor.y, myColor.z, 1 });
				}

				Utils::Vector3f position = myEntity->GetTransform().GetPosition();
				Utils::Vector3f firstPos = { position.x, position.y, position.z };
				//Renderer::SubmitDebugLine(firstPos, firstPos - myEntity->GetTransform().GetForward() * 50.f, { myColor.x, myColor.y, myColor.z, 1 });

				return false;
			});
	}

	void DirectionalLightComponent::SetIntensity(float aIntensity)
	{
		myIntensity = aIntensity;
	}
}
