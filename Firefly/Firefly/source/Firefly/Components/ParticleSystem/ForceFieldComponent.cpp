#include "FFpch.h"
#include "ForceFieldComponent.h"

#include "ForceFieldManager.h"

#include "Firefly/Application/Application.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Event/ApplicationEvents.h"
#include "Firefly/Event/Event.h"
#include "Firefly/Rendering/Renderer.h"

namespace Firefly
{
	REGISTER_COMPONENT(ForceFieldComponent);

	ForceFieldComponent::ForceFieldComponent() : Component("ForceFieldComponent")
	{
		EditorVariable("Force Field Type", ParameterType::Enum, &myForceField.ForceFieldType, { "Point", "Cuboid" });
		EditorVariable("Range", ParameterType::Float, &myForceField.Range);
		EditorVariable("Cuboid Direction", ParameterType::Vec3, &myForceField.Direction);
		EditorVariable("Force", ParameterType::Float, &myForceField.Force);
		myForceField.Global = true;
	}

	void ForceFieldComponent::Initialize()
	{
		ForceFieldManager::Get().AddForceField(myEntity->GetID());

#ifndef FF_INLAMNING
		myForceFieldBillboard = ResourceCache::GetAsset<Texture2D>("Editor/Icons/icon_forcefield.dds", true);
		if (myForceFieldBillboard && myForceFieldBillboard->IsLoaded())
		{
			myForceFieldBillboardInfo.Texture = myForceFieldBillboard;
		}
#endif
	}

	void ForceFieldComponent::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent&)
		{
			myForceField.Position = myEntity->GetTransform().GetPosition();
			return false;
		});

		dispatcher.Dispatch<AppRenderEvent>([&](AppRenderEvent&)
		{
			if (!Application::Get().GetIsInPlayMode())
			{
				DrawDebugLinesAndBillboard();
			}

			return false;
		});
	}

	const ForceField& ForceFieldComponent::GetForceField() const
	{
		return myForceField;
	}

	void ForceFieldComponent::DrawDebugLinesAndBillboard()
	{
		const bool positive = myForceField.Force > 0.0f;
		const Utils::Vector4f color = positive ? Utils::Vector4f(0.85f, 0, 0, 1) : Utils::Vector4f(0.2f, 0, 0.75f, 1);

		if (myForceField.ForceFieldType == ForceFieldType::Point)
		{
			Renderer::SubmitDebugSphere(myForceField.Position, myForceField.Range, 25, color);

			constexpr float arrowFromCentrePercentage = 0.4f;
			const float startMult = (positive ? 1.0f : arrowFromCentrePercentage) * myForceField.Range;
			const float endMult = (positive ? arrowFromCentrePercentage : 1.0f) * myForceField.Range;

			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(1, 1, 1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(1, 1, 1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(-1, 1, 1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(-1, 1, 1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(1, -1, 1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(1, -1, 1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(1, 1, -1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(1, 1, -1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(-1, -1, 1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(-1, -1, 1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(1, -1, -1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(1, -1, -1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(-1, -1, -1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(-1, -1, -1).GetNormalized() * endMult, color);
			Renderer::SubmitDebugArrow(myForceField.Position + Utils::Vector3f(-1, 1, -1).GetNormalized() * startMult, myForceField.Position + Utils::Vector3f(-1, 1, -1).GetNormalized() * endMult, color);
		}
		else
		{
			Renderer::SubmitDebugCube(myForceField.Position, myForceField.Range * 2.0f, color);

			constexpr int arrowsPerFieldSingleAxis = 5;
			constexpr float cornerGap = 5.0f;

			const float diff = (2.0f * myForceField.Range) / arrowsPerFieldSingleAxis;

			for (float i = -myForceField.Range + cornerGap; i < myForceField.Range - cornerGap; i += diff)
			{
				for (float j = -myForceField.Range + cornerGap; j < myForceField.Range - cornerGap; j += diff)
				{
					for (float k = -myForceField.Range + cornerGap; k < myForceField.Range - cornerGap; k += diff)
					{
						const Utils::Vector3f pos = { myForceField.Position.x + i, myForceField.Position.y + j, myForceField.Position.z + k };
						Renderer::SubmitDebugArrow(pos - myForceField.Direction.GetNormalized() * (0.5f * myForceField.Range / arrowsPerFieldSingleAxis), pos + myForceField.Direction.GetNormalized() * (0.5f * myForceField.Range / arrowsPerFieldSingleAxis), color);
					}
				}
			}
		}

		if (myForceFieldBillboard && myForceFieldBillboard->IsLoaded())
		{
			myForceFieldBillboardInfo.Position = myForceField.Position;
			myForceFieldBillboardInfo.Color = color;
			myForceFieldBillboardInfo.EntityID = myEntity->GetID();
			Renderer::Submit(myForceFieldBillboardInfo);
		}
	}
}