#include "FFpch.h"
#include "SpotLightComponent.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Rendering/Renderer.h"
namespace Firefly
{
	REGISTER_COMPONENT(SpotLightComponent);
	SpotLightComponent::SpotLightComponent() : Component("SpotLightComponent")
	{
		EditorVariable("Color", ParameterType::Color, &myColor, false);
		EditorVariable("Intensity", ParameterType::Float, &myIntensity);
		EditorVariable("Range", ParameterType::Float, &myRange);
		EditorVariable("Inner Angle", ParameterType::Float, &myNearRadius);
		EditorVariable("Outer Angle", ParameterType::Float, &myFarRadius);
		EditorVariable("Cast Shadows", ParameterType::Bool, &myShouldCastShadows);
	}

	void SpotLightComponent::Initialize()
	{
	}

	void SpotLightComponent::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent& e)
			{
				myFarRadius = Utils::Max(myFarRadius, myNearRadius);

				SpotLightPacket packet;
				myColor.w = myIntensity * 100.f;
				packet.ColorAndIntensity = myColor;
				packet.Position = { myEntity->GetTransform().GetPosition().x, myEntity->GetTransform().GetPosition().y, myEntity->GetTransform().GetPosition().z, 0 };
				packet.Direction = { myEntity->GetTransform().GetForward().x, myEntity->GetTransform().GetForward().y, myEntity->GetTransform().GetForward().z, 0 };
				packet.Range_Inner_Outer_ShouldCastShadow = { myRange, DEGTORAD(myNearRadius),DEGTORAD(myFarRadius), static_cast<float>(myShouldCastShadows)};


				Utils::Mat4 projMat = Utils::Mat4::CreateLeftHandedProjectionMatrixPerspective(512, 512, 1, std::max(myRange, 1.f), myFarRadius * 2);
				Utils::Mat4 viewMat = Utils::Mat4::GetFastInverse(myEntity->GetTransform().GetMatrix());

				packet.ViewProjMatrix = viewMat * projMat;

				Renderer::Submit(packet);


				Renderer::SubmitDebugArrow({ packet.Position.x, packet.Position.y,  packet.Position.z }, { packet.Position.x + packet.Direction.x * myRange, packet.Position.y + packet.Direction.y * myRange,  packet.Position.z + packet.Direction.z * myRange }, { myColor.x,myColor.y,myColor.z, 1 });


				// calc radius of ring

				const float outerRingRadius = myRange * tanf(DEGTORAD(myFarRadius));
				const float innerRingRadius = myRange * tanf(DEGTORAD(myNearRadius));

				const Utils::Quaternion rotQuat = Utils::Quaternion::CreateFromEulerAngles({90.f, 0.f, 0.f });


				Renderer::SubmitDebugCircle(Utils::Vec4ToVec3(packet.Position + packet.Direction * myRange), outerRingRadius, 16, myEntity->GetTransform().GetRotation() + Utils::Vec3(90, 0, 0), myColor);
				Renderer::SubmitDebugCircle(Utils::Vec4ToVec3(packet.Position + packet.Direction * myRange), innerRingRadius, 16, myEntity->GetTransform().GetRotation() + Utils::Vec3(90, 0, 0), myColor);

				constexpr int32_t numLines = 16;

				// stolen from SubmitDebugCircle
				constexpr float rotationStep = 2 * PI / static_cast<float>(numLines);

				for (int i = 0; i < numLines; ++i)
				{
					float outerX = cos(rotationStep * i) * outerRingRadius;
					float outerY = sin(rotationStep * i) * outerRingRadius;
					float innerX = cos(rotationStep * i) * innerRingRadius;
					float innerY = sin(rotationStep * i) * innerRingRadius;

					Vector4f outerStart = Utils::Vector4f(outerX, 0, outerY, 1);
					Vector4f innerStart = Utils::Vector4f(innerX, 0, innerY, 1);

					const Utils::Matrix4f rotation = Utils::Matrix4f::CreateRotationMatrix(myEntity->GetTransform().GetQuaternion() * rotQuat);

					outerStart = outerStart * rotation;
					outerStart += packet.Position;

					innerStart = innerStart * rotation;
					innerStart += packet.Position;

					Renderer::SubmitDebugLine({ packet.Position.x, packet.Position.y,  packet.Position.z }, { outerStart.x + packet.Direction.x * myRange, outerStart.y + packet.Direction.y * myRange, outerStart.z + packet.Direction.z * myRange }, myColor);
					Renderer::SubmitDebugLine({ packet.Position.x, packet.Position.y,  packet.Position.z }, { innerStart.x + packet.Direction.x * myRange, innerStart.y + packet.Direction.y * myRange, innerStart.z + packet.Direction.z * myRange }, myColor);
				}

				return false;
			});
	}

	void SpotLightComponent::SetIntensity(float aIntensity)
	{
		myIntensity = aIntensity;
	}
}
