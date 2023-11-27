#include "FFpch.h"
#include "PointLightComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Rendering/Renderer.h"
namespace Firefly
{
	REGISTER_COMPONENT(PointLightComponent);
	Firefly::PointLightComponent::PointLightComponent() : Component("PointLightComponent")
	{
		EditorVariable("Color", ParameterType::Color, &myColor);
		EditorVariable("Intensity", ParameterType::Float, &myIntensity);
		EditorVariable("Radius", ParameterType::Float, &myRadius);
		EditorVariable("Cast Shadows", ParameterType::Bool, &myCastShadows);
	}

	void Firefly::PointLightComponent::Initialize()
	{
		myIcon = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_pointlight.dds");
	}

	void Firefly::PointLightComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
			{
				PointLightPacket packet;
				myColor.w = myIntensity;
				packet.ColorAndIntensity = myColor;
				packet.Position = myEntity->GetTransform().GetPosition();
				packet.Radius = myRadius;
				packet.PointlightCustomData.w = static_cast<float>(myCastShadows);
				auto camera = Firefly::Renderer::GetActiveCamera();
				bool visable = true;
				if (camera)
				{
					Utils::Sphere<float> sphere;
					sphere.InitWithCenterAndRadius(packet.Position, packet.Radius);
					visable = camera->MeshIsVisible(sphere);
				}

				if (visable)
				{
					if (myCastShadows)
					{
						auto shadowProjection = Utils::Mat4::CreateLeftHandedProjectionMatrixPerspective(1, 1, 1.f, myRadius, 90.f);

						std::array<Utils::Mat4, 6> lookats;
						lookats[4] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(0, 0, -1.0), packet.Position, Utils::Vec3(0, 1, 0));
						lookats[5] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(0, 0, 1.0), packet.Position, Utils::Vec3(0, 1, 0));

						lookats[2] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(0, -1.0, 0), packet.Position, Utils::Vec3(0, 0, -1));
						lookats[3] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(0, 1.0, 0), packet.Position, Utils::Vec3(0, 0, 1));

						lookats[0] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(-1.0, 0, 0), packet.Position, Utils::Vec3(0, 1, 0));
						lookats[1] = Utils::Mat4::CreateLookAt(packet.Position + Utils::Vec3(1.0, 0, 0), packet.Position, Utils::Vec3(0, 1, 0));

						for (size_t i = 0; i < lookats.size(); ++i)
						{
							packet.Transforms[i] = lookats[i] * shadowProjection;
						}
					}


					Renderer::Submit(packet);

					BillboardInfo billboardInfo{};
					billboardInfo.Position = myEntity->GetTransform().GetPosition();
					billboardInfo.EntityID = myEntity->GetID();
					billboardInfo.Color = { myColor.x,myColor.y,myColor.z, 1 };
					billboardInfo.Texture = myIcon;
					Renderer::Submit(billboardInfo);

					Renderer::SubmitDebugSphere(packet.Position, myRadius, 25, { myColor.x,myColor.y,myColor.z, 1 });
				}

				return false;
			});
	}

	void PointLightComponent::SetIntensity(float aIntensity)
	{
		myIntensity = aIntensity;
	}

	void PointLightComponent::SetRadius(float aRadius)
	{
		myRadius = aRadius;
	}
}
