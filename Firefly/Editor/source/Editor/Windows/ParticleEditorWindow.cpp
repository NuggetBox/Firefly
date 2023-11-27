#include "EditorPch.h"
#include "ParticleEditorWindow.h"

#include <Firefly/Application/Application.h>
#include <Firefly/Event/EditorEvents.h>
#include <Utils/InputHandler.h>

#include "ContentBrowser.h"
#include "LerpVisualizerWindow.h"

#include "Editor/EditorLayer.h"
#include "Editor/Utilities/EditorUtils.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Rendering/Renderer.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/ParticleSystem/MeshEmissionController.h"

#include <format>
#include <Firefly/Asset/Animation.h>
#include <Firefly/Components/Animation/AnimationPlayerComponent.h>
#include <Firefly/Components/Mesh/AnimatedMeshComponent.h>
#include <Utils/Timer.h>

REGISTER_WINDOW(ParticleEditorWindow);

ParticleEditorWindow::ParticleEditorWindow() : EditorWindow("Particle Editor")
{
	myEditorCamera = CreateRef<EditorCamera>();
	myParticleEmitter = CreateRef<Firefly::ParticleEmitter>();

	myEditorCamera->Initialize(Firefly::CameraInfo());

	mySceneID = Firefly::Renderer::InitializeScene();

	ResetEditor();

	Firefly::ParticleEmitterTemplate temp;
	temp.MaterialPath = myMaterialPath;
	temp.TexturePath = myTexturePath;
	temp.EmitterSettings = myEmitterSettings;
	myParticleEmitter->Initialize(temp, true, false, true, 65536);

	myForceFieldBillboard = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_forcefield.dds", true);
	if (myForceFieldBillboard && myForceFieldBillboard->IsLoaded())
	{
		myForceFieldBillboardInfo.Texture = myForceFieldBillboard;
	}

	myEmitterSettings.ColorGradient.AddColorMarker(0.0f, { 1.0f, 1.0f, 1.0f }, 1.0f);
	myEmitterSettings.ColorGradient.AddColorMarker(1.0f, { 1.0f, 1.0f, 1.0f }, 1.0f);
	myEmitterSettings.ColorGradient.AddAlphaMarker(0.0f, 1.0f);
	myEmitterSettings.ColorGradient.AddAlphaMarker(1.0f, 1.0f);
}

void ParticleEditorWindow::OnImGui()
{
	//Update Editor Camera Input
	myEditorCamera->Update();
	//

	CalcActualPosAndSize();

	//Resize to content region
	if (Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(mySceneID)->GetSpecs().Width) - myActualWindowSize.x) > 1.0f
		|| Utils::Abs(static_cast<float>(Firefly::Renderer::GetSceneFrameBuffer(mySceneID)->GetSpecs().Height) - myActualWindowSize.y) > 1.0f)
	{
		const auto newSize = Utils::Vector2<uint32_t>(myActualWindowSize.x, myActualWindowSize.y);
		Firefly::Renderer::GetSceneFrameBuffer(mySceneID)->Resize(newSize);
		myEditorCamera->GetCamera()->SetSizeX(static_cast<float>(newSize.x));
		myEditorCamera->GetCamera()->SetSizeY(static_cast<float>(newSize.y));
	}
	//
	ImGui::SetCursorScreenPos(myActualWindowPos);
	ImGui::Image(Firefly::Renderer::GetSceneFrameBuffer(mySceneID)->GetColorAttachment(0).Get(), myActualWindowSize);

	//Drag&drop on viewport
	if (auto payload = ImGuiUtils::DragDropWindow("FILE", ".emitter", false))
	{
		SetFocused();

		std::filesystem::path droppedFile = static_cast<const char*>(payload->Data);

		SetEmitter(droppedFile);
	}
	//

	UpdateProperties(myTexturePath, myMaterialPath, myEmitterSettings);

	Firefly::Renderer::BeginScene(mySceneID);

	MousePick();

	UpdateAnimatedMeshEmission();

	SubmitRenderData();

	//Fix rotation, position scale in editor
	myParticleEmitter->Update(Utils::Transform());

	Firefly::ParticleEmitterCommand renderCommand;
	renderCommand.ParentTransform = Utils::Matrix4f();
	renderCommand.Emitter = myParticleEmitter;
	Firefly::Renderer::Submit(renderCommand);

	RenderDebugLines();

	myEditorCamera->SetActiveCamera();
	Firefly::Renderer::EndScene();
}

void ParticleEditorWindow::SetEmitter(const std::filesystem::path& aPath)
{
	auto emitter = Firefly::ResourceCache::GetAsset<Firefly::ParticleEmitterTemplate>(aPath);
	myMaterialPath = emitter->MaterialPath.string();
	myTexturePath = emitter->TexturePath.string();
	myEmitterSettings = emitter->EmitterSettings;
	myParticleEmitter->LoadNewMaterial(myMaterialPath);
	myParticleEmitter->LoadNewTexture(myTexturePath);
	myParticleEmitter->SetEmitterSettings(myEmitterSettings);
	myParticleEmitter->ClearParticles();
}

void ParticleEditorWindow::MousePick()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsUsing())
	{
		auto id = static_cast<uint32_t>(Firefly::Renderer::GetEntityFromScreenPos(mySceneID, myActualMousePos.x, myActualMousePos.y));
		mySelectedForceField = id - 1;
	}
}

void ParticleEditorWindow::SubmitRenderData()
{
	//TODO: Use same bloom as in game
	Firefly::PostProcessInfo inf{};
	inf.Enable = true;
	inf.Data.Padding.z = 0.1f;
	Firefly::Renderer::Submit(inf);

	Firefly::EnvironmentData envData{};
	if (myDrawAtmosphere)
	{
		Firefly::DirLightPacket dirPacket{};
		dirPacket.Direction = { -0.71f, 0.71f, -0.71f, 1.0f };
		dirPacket.ColorAndIntensity = { 1,1,1,1 };
		dirPacket.dirLightInfo.x = 0;
		Firefly::Renderer::Submit(dirPacket, Firefly::ShadowResolutions::res1024);

		envData.EnvironmentMap = nullptr;
		envData.Intensity = 1;
	}
	Firefly::Renderer::Submit(envData);
}

bool ParticleEditorWindow::UpdateGuizmo()
{
	ImGuizmo::Enable(true);
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

	const auto& view = myEditorCamera->GetCamera()->GetViewMatrix();
	const auto& proj = myEditorCamera->GetCamera()->GetProjectionMatrixPerspective();

	Utils::Matrix4f matrix;
	matrix(4, 1) = myEmitterSettings.LocalForceFields[mySelectedForceField].Position.x;
	matrix(4, 2) = myEmitterSettings.LocalForceFields[mySelectedForceField].Position.y;
	matrix(4, 3) = myEmitterSettings.LocalForceFields[mySelectedForceField].Position.z;

	bool manipulated = false;

	if (ImGuizmo::Manipulate(view.data(), proj.data(), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, matrix.data()))
	{
		myEmitterSettings.LocalForceFields[mySelectedForceField].Position.x = matrix(4, 1);
		myEmitterSettings.LocalForceFields[mySelectedForceField].Position.y = matrix(4, 2);
		myEmitterSettings.LocalForceFields[mySelectedForceField].Position.z = matrix(4, 3);

		manipulated = true;
	}

	return manipulated;
}

void ParticleEditorWindow::RenderDebugLines()
{
	Firefly::Renderer::SetGridActive(myDrawGrid);

	if (myDrawDebugLines)
	{
		switch (myEmitterSettings.EmitterType)
		{
		case EmitterType::Cone:
		{
			Firefly::Renderer::SubmitDebugCircle(Utils::Vector3f::Zero(), myParticleEmitter->GetInnerRadius(), 25, Utils::Vector3f::Zero(), myEmitterSettings.ColorGradient.GetCombinedColor(0));
			Firefly::Renderer::SubmitDebugCircle({ 0, myEmitterSettings.Height, 0 }, myParticleEmitter->GetOuterRadius(), 25, Utils::Vector3f::Zero(), myEmitterSettings.ColorGradient.GetCombinedColor(1));
			break;
		}
		case EmitterType::Sphere:
		{
			Firefly::Renderer::SubmitDebugSphere(Utils::Vector3f::Zero(), myParticleEmitter->GetSphereRadius(), 25, myEmitterSettings.ColorGradient.GetCombinedColor(0));
			break;
		}
		case EmitterType::Rectangle:
		{
			Utils::Vector3f innerHalfSize = { myEmitterSettings.InnerRectangleWidth * 0.5f, 0.0f, myEmitterSettings.InnerRectangleHeight * 0.5f };
			Utils::Vector3f outerHalfSize = { myEmitterSettings.OuterRectangleWidth * 0.5f, myEmitterSettings.Height, myEmitterSettings.OuterRectangleHeight * 0.5f };

			Firefly::Renderer::SubmitDebugLine({ -innerHalfSize.x, 0.0f, -innerHalfSize.z }, { -innerHalfSize.x, 0.0f, innerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(0));
			Firefly::Renderer::SubmitDebugLine({ -innerHalfSize.x, 0.0f, innerHalfSize.z }, { innerHalfSize.x, 0.0f, innerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(0));
			Firefly::Renderer::SubmitDebugLine({ innerHalfSize.x, 0.0f, innerHalfSize.z }, { innerHalfSize.x, 0.0f, -innerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(0));
			Firefly::Renderer::SubmitDebugLine({ innerHalfSize.x, 0.0f, -innerHalfSize.z }, { -innerHalfSize.x, 0.0f, -innerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(0));

			Firefly::Renderer::SubmitDebugLine({ -outerHalfSize.x, outerHalfSize.y, -outerHalfSize.z }, { -outerHalfSize.x, outerHalfSize.y, outerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(1));
			Firefly::Renderer::SubmitDebugLine({ -outerHalfSize.x, outerHalfSize.y, outerHalfSize.z }, { outerHalfSize.x, outerHalfSize.y, outerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(1));
			Firefly::Renderer::SubmitDebugLine({ outerHalfSize.x, outerHalfSize.y, outerHalfSize.z }, { outerHalfSize.x, outerHalfSize.y, -outerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(1));
			Firefly::Renderer::SubmitDebugLine({ outerHalfSize.x, outerHalfSize.y, -outerHalfSize.z }, { -outerHalfSize.x, outerHalfSize.y, -outerHalfSize.z }, myEmitterSettings.ColorGradient.GetCombinedColor(1));
			break;
		}
		default: break;
		}

		for (auto i = 0; i < myEmitterSettings.LocalForceFields.size(); ++i)
		{
			const auto& forceField = myEmitterSettings.LocalForceFields[i];

			const bool positive = forceField.Force > 0.0f;
			const Utils::Vector4f color = positive ? Utils::Vector4f(0.85f, 0, 0, 1) : Utils::Vector4f(0.2f, 0, 0.75f, 1);

			if (forceField.ForceFieldType == ForceFieldType::Point)
			{
				myForceFieldBillboardInfo.Position = forceField.Position;
				myForceFieldBillboardInfo.Color = color;
				myForceFieldBillboardInfo.EntityID = static_cast<uint64_t>(i + 1);
				Firefly::Renderer::Submit(myForceFieldBillboardInfo);

				Firefly::Renderer::SubmitDebugSphere(forceField.Position, forceField.Range, 25, color);

				constexpr float arrowFromCentrePercentage = 0.4f;
				const float startMult = (positive ? 1.0f : arrowFromCentrePercentage) * forceField.Range;
				const float endMult = (positive ? arrowFromCentrePercentage : 1.0f) * forceField.Range;

				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(1, 1, 1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(1, 1, 1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(-1, 1, 1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(-1, 1, 1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(1, -1, 1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(1, -1, 1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(1, 1, -1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(1, 1, -1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(-1, -1, 1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(-1, -1, 1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(1, -1, -1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(1, -1, -1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(-1, -1, -1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(-1, -1, -1).GetNormalized() * endMult, color);
				Firefly::Renderer::SubmitDebugArrow(forceField.Position + Utils::Vector3f(-1, 1, -1).GetNormalized() * startMult, forceField.Position + Utils::Vector3f(-1, 1, -1).GetNormalized() * endMult, color);
			}
			else if (forceField.ForceFieldType == ForceFieldType::Cuboid)
			{
				myForceFieldBillboardInfo.Position = forceField.Position;
				myForceFieldBillboardInfo.Color = color;
				myForceFieldBillboardInfo.EntityID = static_cast<uint64_t>(i + 1);
				Firefly::Renderer::Submit(myForceFieldBillboardInfo);

				Firefly::Renderer::SubmitDebugCube(forceField.Position, forceField.Range * 2.0f, color);

				constexpr int arrowsPerFieldSingleAxis = 5;
				constexpr float cornerGap = 5.0f;

				const float diff = (2.0f * forceField.Range) / arrowsPerFieldSingleAxis;

				for (float i = -forceField.Range + cornerGap; i < forceField.Range - cornerGap; i += diff)
				{
					for (float j = -forceField.Range + cornerGap; j < forceField.Range - cornerGap; j += diff)
					{
						for (float k = -forceField.Range + cornerGap; k < forceField.Range - cornerGap; k += diff)
						{
							const Utils::Vector3f pos = { forceField.Position.x + i, forceField.Position.y + j, forceField.Position.z + k };
							Firefly::Renderer::SubmitDebugArrow(pos - forceField.Direction.GetNormalized() * (0.5f * forceField.Range / arrowsPerFieldSingleAxis), pos + forceField.Direction.GetNormalized() * (0.5f * forceField.Range / arrowsPerFieldSingleAxis), color);
						}
					}
				}
			}
		}
	}
}

void ParticleEditorWindow::UpdateProperties(std::string& aTexturePath, std::string& aMaterialPath, EmitterSettings& aSettings)
{
	bool settingsChanged = false;

	if (ImGui::Begin("Particle Emitter Settings"))
	{
		myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

		bool textureChanged = false;
		bool materialChanged = false;
		bool meshChanged = false;
		bool animationChanged = false;

		if (ImGui::Button("New Emitter"))
		{
			ResetEditor();
		}

		if (ImGui::Button("Save Emitter"))
		{
			const std::string fullSavePath = EditorUtils::GetSaveFilePath("Particle Emitter (*.emitter)\0*.emitter\0", "emitter").string();

			if (!fullSavePath.empty())
			{
				SaveEmitter(fullSavePath);

				const std::filesystem::path assetPath = EditorUtils::GetRelativePath(fullSavePath);

				Firefly::ResourceCache::UnloadAsset(assetPath);

				EmitterUpdatedEvent emitterUpdatedEvent(assetPath.string());
				Firefly::Application::Get().OnEvent(emitterUpdatedEvent);

				EditorLayer::GetWindow<ContentBrowser>()->RegenerateEntries();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Emitter"))
		{
			const auto loadPath = EditorUtils::GetOpenFilePath("Particle Emitter (*.emitter)\0*.emitter\0");
			myParticleEmitter->ClearParticles();

			if (loadPath != "")
			{
				const Ref<Firefly::ParticleEmitterTemplate> loaded = Firefly::ResourceCache::GetAsset<Firefly::ParticleEmitterTemplate>(loadPath, true);

				if (loaded)
				{
					aTexturePath = loaded->TexturePath.string();
					aMaterialPath = loaded->MaterialPath.string();
					aSettings = loaded->EmitterSettings;
					textureChanged = true;
					materialChanged = true;
					settingsChanged = true;
				}
			}
		}

		if (ImGui::TreeNodeEx("Particle Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//Particle Sprite input field
			std::string inputTexturePath = myTexturePath;
			textureChanged |= ImGuiUtils::FileParameter("Particle Sprite", inputTexturePath, ".dds");
			//

			//Particle Material input field
			std::string inputMaterialPath = myMaterialPath;
			materialChanged |= ImGuiUtils::FileParameter("Particle Material", inputMaterialPath, ".mat");

			ImGui::SameLine();
			if (ImGui::Button("Reset To Default"))
			{
				inputMaterialPath = Firefly::DefaultParticleMaterialPath;
				materialChanged = true;
			}
			//

			// Flipbook
			ImGuiUtils::BeginParameters("Is Flipbook");
			settingsChanged |= ImGuiUtils::Parameter("Is Flipbook", aSettings.IsFlipbook, "Should the texture be interpreted as a flipbook");
			ImGuiUtils::EndParameters();

			if (aSettings.IsFlipbook)
			{
				if (ImGui::TreeNode("Flipbook settings"))
				{
					ImGuiUtils::BeginParameters("FlipBookSettings");
					settingsChanged |= ImGuiUtils::Parameter("Frames", aSettings.Frames, 1, 1, 8, "How many frames in total is the flipbook texture", false);
					settingsChanged |= ImGuiUtils::Parameter("Rows", aSettings.Rows, 1, 1, 8, "How many rows of sprites does the flipbook texture have", false);
					settingsChanged |= ImGuiUtils::Parameter("Columns", aSettings.Columns, 1, 1, 8, "How many columns of sprites does the flipbook texture have", false);
					settingsChanged |= ImGuiUtils::Parameter("Framerate", aSettings.FrameRate, 1, 1, 8, "How many frames of the flipbook should be played per second, 0 for one iteration of flipbook", false);
					ImGuiUtils::EndParameters();

					ImGui::TreePop();
				}
			}
			//

			//Blend state
			const auto blendstateNames = Firefly::BlendStateNamesToStringVector();
			settingsChanged |= ImGuiUtils::EnumParameter("Blend mode", (uint32_t&)aSettings.BlendState, blendstateNames);
			ImGuiUtils::ToolTip("Changes how the particles are rendered on top of each other");

			//Emitter Type
			ImGui::Text("Emitter Type");
			ImGuiUtils::ToolTip("Changes the emission type of the emitter");
			if (ImGui::RadioButton("Cone", aSettings.EmitterType == EmitterType::Cone))
			{
				aSettings.EmitterType = EmitterType::Cone;
				settingsChanged = true;
			}
			ImGuiUtils::ToolTip("Particles emit from a cone");
			if (ImGui::RadioButton("Sphere", aSettings.EmitterType == EmitterType::Sphere))
			{
				aSettings.EmitterType = EmitterType::Sphere;
				settingsChanged = true;
			}
			ImGuiUtils::ToolTip("Particles emit from a sphere");
			if (ImGui::RadioButton("Rectangle", aSettings.EmitterType == EmitterType::Rectangle))
			{
				aSettings.EmitterType = EmitterType::Rectangle;
				settingsChanged = true;
			}
			ImGuiUtils::ToolTip("Particles emit from a rectangle");
			if (ImGui::RadioButton("Mesh", aSettings.EmitterType == EmitterType::Mesh))
			{
				aSettings.EmitterType = EmitterType::Mesh;
				settingsChanged = true;
			}
			ImGuiUtils::ToolTip("Particles emit from a given mesh");
			if (ImGui::RadioButton("Animated Mesh", aSettings.EmitterType == EmitterType::AnimatedMesh))
			{
				aSettings.EmitterType = EmitterType::AnimatedMesh;
				settingsChanged = true;
			}
			ImGuiUtils::ToolTip("Particles emit from a given animated mesh");
			//

			//Cone settings
			if (aSettings.EmitterType == EmitterType::Cone)
			{
				if (ImGui::TreeNodeEx("Cone Emitter Settings"))
				{
					ImGuiUtils::BeginParameters("ConeRadii");
					settingsChanged |= ImGuiUtils::Parameter("Height", aSettings.Height, 1.0f, FLT_MIN, FLT_MAX, "The height of the cone-shaped emission, previously controlled by Speed (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Inner Radius", aSettings.InnerRadius, 1.0f, 0.0f, FLT_MAX, "The inner radius of the cone-shaped emission (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Inner Radius Change Speed", aSettings.InnerRadiusChangeSpeed, 1.0f, FLT_MIN, FLT_MAX, "The speed at which the inner radius changes (centimeters/second)");
					settingsChanged |= ImGuiUtils::Parameter("Max Inner Radius", aSettings.InnerRadiusMax, 1.0f, 0.0f, FLT_MAX, "The maximum absolute value for the inner radius");
					settingsChanged |= ImGuiUtils::Parameter("Loop Inner Change Speed", aSettings.LoopInnerChangeSpeed, "Should the inner change speed loop and go back to starting radius when it's done?");
					settingsChanged |= ImGuiUtils::Parameter("Outer Radius", aSettings.OuterRadius, 1.0f, 0.0f, FLT_MAX, "The outer radius of the cone-shaped emission (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Outer Radius Change Speed", aSettings.OuterRadiusChangeSpeed, 1.0f, FLT_MIN, FLT_MAX, "The speed at which the outer radius changes (centimeters/second)");
					settingsChanged |= ImGuiUtils::Parameter("Max Outer Radius", aSettings.OuterRadiusMax, 1.0f, 0.0f, FLT_MAX, "The maximum absolute value for the outer radius");
					settingsChanged |= ImGuiUtils::Parameter("Loop Outer Change Speed", aSettings.LoopOuterChangeSpeed, "Should the outer change speed loop and go back to starting radius when it's done?");
					settingsChanged |= ImGuiUtils::Parameter("Bounce Change In Radius", aSettings.BounceRadiusChange, "Should the radius switch, and move back towards starting radius or become starting radius instantly?");
					ImGuiUtils::EndParameters();

					ImGuiUtils::BeginParameters("SpawnOnEdge");
					settingsChanged |= ImGuiUtils::Parameter("Spawn on edge", aSettings.SpawnOnEdge, "Should the particles spawn on the edge of the inner circle, or inside the circle");
					ImGuiUtils::EndParameters();

					ImGuiUtils::BeginParameters("AimForEdge");
					settingsChanged |= ImGuiUtils::Parameter("Aim for edge", aSettings.AimForEdge, "Should the particles aim for the edge of the outer circle, or just aim inside the outer circle");
					ImGuiUtils::EndParameters();

					ImGui::TreePop();
				}
			}
			//

			//Sphere Settings
			if (aSettings.EmitterType == EmitterType::Sphere)
			{
				if (ImGui::TreeNodeEx("Sphere Emitter Settings"))
				{
					ImGuiUtils::BeginParameters("SphereRadius");
					settingsChanged |= ImGuiUtils::Parameter("Sphere Radius", aSettings.Radius, 1.0f, 0.0f, FLT_MAX, "The radius of the sphere-shaped emission (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Radius Change Speed", aSettings.RadiusChangeSpeed, 1.0f, 0.0f, FLT_MAX, "The speed at which the sphere radius changes (centimeters/second)");
					settingsChanged |= ImGuiUtils::Parameter("Max Radius", aSettings.RadiusMax, 1.0f, 0.0f, FLT_MAX, "The maximum absolute value for the radius");
					settingsChanged |= ImGuiUtils::Parameter("Loop Change Speed", aSettings.LoopInnerChangeSpeed, "Should the change speed loop and go back to starting radius when it's done?");
					settingsChanged |= ImGuiUtils::Parameter("Bounce Change In Radius", aSettings.BounceRadiusChange, "Should the radius switch, and move back towards starting radius or become starting radius instantly?");
					ImGuiUtils::EndParameters();

					ImGuiUtils::BeginParameters("SpawnOnSurface");
					settingsChanged |= ImGuiUtils::Parameter("Spawn on surface", aSettings.SpawnOnSurface, "Should the particles spawn on the surface of the sphere, or inside the sphere");
					ImGuiUtils::EndParameters();

					ImGui::TreePop();
				}
			}
			//

			//Rectangle Settings
			if (aSettings.EmitterType == EmitterType::Rectangle)
			{
				if (ImGui::TreeNode("Rectangle Emitter Settings"))
				{
					ImGuiUtils::BeginParameters("RectangleSize");
					settingsChanged |= ImGuiUtils::Parameter("Emission Height", aSettings.Height, 1.0f, FLT_MIN, FLT_MAX, "The height of the rectabgle-shaped emission, previously controlled by Speed (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Width", aSettings.InnerRectangleWidth, 1.0f, 0.0f, FLT_MAX, "The inner rectangle width (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Height", aSettings.InnerRectangleHeight, 1.0f, 0.0f, FLT_MAX, "The inner rectangle height (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Outer Width", aSettings.OuterRectangleWidth, 1.0f, 0.0f, FLT_MAX, "The outer rectangle width (centimeters)");
					settingsChanged |= ImGuiUtils::Parameter("Outer Height", aSettings.OuterRectangleHeight, 1.0f, 0.0f, FLT_MAX, "The outer rectangle height (centimeters)");
					//settingsChanged = ImGuiUtils::Parameter("Width Change Speed", aSettings.InnerRadiusChangeSpeed, 1.0f, 0.0f, FLT_MAX, "The speed at which the rectangle width changes (centimeters/second)") || settingsChanged;
					//settingsChanged = ImGuiUtils::Parameter("Height Change Speed", aSettings.OuterRadiusChangeSpeed, 1.0f, 0.0f, FLT_MAX, "The speed at which the rectangle height changes (centimeters/second)") || settingsChanged;
					//settingsChanged = ImGuiUtils::Parameter("Max Width", aSettings.InnerRadiusMax, 1.0f, 0.0f, FLT_MAX, "The maximum absolute value for the width") || settingsChanged;
					//settingsChanged = ImGuiUtils::Parameter("Max Height", aSettings.OuterRadiusMax, 1.0f, 0.0f, FLT_MAX, "The maximum absolute value for the height") || settingsChanged;
					ImGuiUtils::EndParameters();

					ImGuiUtils::BeginParameters("SpawnOnEdgeRectangle");
					settingsChanged |= ImGuiUtils::Parameter("Spawn on edge", aSettings.SpawnOnEdge, "Should the particles spawn on the edge of the rectangle, or inside the rectangle");
					ImGuiUtils::EndParameters();

					/*ImGuiUtils::BeginParameters("AimForEdgeRectangle");
					settingsChanged = ImGuiUtils::Parameter("Aim for edge", aSettings.AimForEdge, "Should the particles aim for the edge of the outer rectangle, or just aim inside the outer rectangle") || settingsChanged;
					ImGuiUtils::EndParameters();*/

					ImGui::TreePop();
				}
			}
			//

			//Mesh Settings
			std::string inputMeshPath;
			if (aSettings.EmitterType == EmitterType::Mesh)
			{
				if (ImGui::TreeNodeEx("Mesh Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen))
				{
					inputMeshPath = myMeshPath;
					meshChanged = ImGuiUtils::FileParameter("Mesh Emitter", inputMeshPath, ".mesh");
					settingsChanged |= meshChanged;

					settingsChanged |= ImGuiUtils::Parameter("Normal Offset", aSettings.MeshNormalOffset, 1.0f, -FLT_MAX, FLT_MAX, "When spawning on a mesh, what offset should they have from the surface, along the normal");

					ImGui::TreePop();
				}
			}
			//

			//Animated Mesh Settings
			std::string inputAnimationPath;
			if (aSettings.EmitterType == EmitterType::AnimatedMesh)
			{
				if (ImGui::TreeNodeEx("Animated Mesh Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen))
				{
					inputAnimationPath = myAnimationPath;
					animationChanged = ImGuiUtils::FileParameter("Animation Path", inputAnimationPath, ".anim");
					settingsChanged |= animationChanged;

					settingsChanged |= ImGuiUtils::Parameter("Normal Offset", aSettings.MeshNormalOffset, 1.0f, -FLT_MAX, FLT_MAX, "When spawning on an animated mesh, what offset should they have from the surface, along the normal");

					ImGui::TreePop();
				}
			}
			//

			//Spawn rate
			ImGuiUtils::BeginParameters("SpawnRate");
			settingsChanged |= ImGuiUtils::Parameter("Spawn Rate", aSettings.SpawnRate, 1.0f, 0.0f, FLT_MAX, "How many particles spawn per second");
			ImGuiUtils::EndParameters();
			//

			//Life Time
			bool lifeTimeOpen = ImGui::TreeNode("LifeTime");
			ImGuiUtils::ToolTip("The total lifetime for any given particle will be randomly generated between Min and Max (seconds)");

			if (lifeTimeOpen)
			{
				ImGuiUtils::BeginParameters("LifeTime");
				settingsChanged |= ImGuiUtils::Parameter("Min", aSettings.MinLifeTime, 0.1f, 0, aSettings.MaxLifeTime, "Minimum randomizable lifetime for any particle in the emitter (seconds)");
				settingsChanged |= ImGuiUtils::Parameter("Max", aSettings.MaxLifeTime, 0.1f, aSettings.MinLifeTime, FLT_MAX, "Maximum randomizable lifetime for any particle in the emitter (seconds)");
				ImGuiUtils::EndParameters();

				ImGui::TreePop();
			}
			//

			//Scale
			bool scaleOpen = ImGui::TreeNode("Scale");
			ImGuiUtils::ToolTip("The scale of a particle will go from Start to End during its lifetime");

			if (scaleOpen)
			{
				ImGuiUtils::BeginParameters("Scale");
				settingsChanged |= ImGuiUtils::Parameter("StartScale", aSettings.StartScale, 0.1f, 0.0f, FLT_MAX, "The scale of the particle when it spawns");
				settingsChanged |= ImGuiUtils::Parameter("EndScale", aSettings.EndScale, 0.1f, 0.0f, FLT_MAX, "The scale of the particle before it disappears");
				settingsChanged |= ImGuiUtils::Parameter("ScaleVariation", aSettings.ScaleVariation, 0.05f, 0.0f, FLT_MAX, "Introduces a variation in scale between the particles. For each particle, a number between 1 and Scale Variation will be generated and multiplied by the Start- and Endscale of that particle") || settingsChanged;
				ImGuiUtils::EndParameters();

				ImGui::TreePop();
			}
			//

			//Color
			bool colorOpen = ImGui::TreeNode("Color##ParticleEditorTree");
			ImGuiUtils::ToolTip("The color of a particle will interpolate between StartColor and EndColor during its lifetime");

			if (colorOpen)
			{
				bool isMarkerShown = true;
				settingsChanged |= ImGui::ImGradientHDR("Gradient##ParticleColorGradient", aSettings.ColorGradient, myColorGradientEditorState, isMarkerShown);

				if (myColorGradientEditorState.selectedMarkerType == ImGui::ImGradientHDRMarkerType::Color)
				{
					auto selectedColorMarker = aSettings.ColorGradient.GetColorMarker(myColorGradientEditorState.selectedIndex);
					if (selectedColorMarker != nullptr)
					{
						ImGui::Unindent();
						settingsChanged |= ImGui::ColorEdit3("Color##ParticleEditorGradient", selectedColorMarker->Color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueBar);
					}
				}

				if (myColorGradientEditorState.selectedMarkerType == ImGui::ImGradientHDRMarkerType::Alpha)
				{
					auto selectedAlphaMarker = aSettings.ColorGradient.GetAlphaMarker(myColorGradientEditorState.selectedIndex);
					if (selectedAlphaMarker != nullptr)
					{
						settingsChanged |= ImGui::DragFloat("Alpha##ParticleEditor", &selectedAlphaMarker->Alpha, 0.01f, 0.0f, 1.0f, "%f", 1.0f);
					}
				}

				if (myColorGradientEditorState.selectedMarkerType != ImGui::ImGradientHDRMarkerType::Unknown)
				{
					if (ImGui::Button("Delete##ParticleEditor") || ImGui::IsKeyPressed(ImGuiKey_Delete, false))
					{
						if (myColorGradientEditorState.selectedMarkerType == ImGui::ImGradientHDRMarkerType::Color)
						{
							aSettings.ColorGradient.RemoveColorMarker(myColorGradientEditorState.selectedIndex);
							myColorGradientEditorState = ImGui::ImGradientHDRTemporaryState{};
							settingsChanged = true;
						}
						else if (myColorGradientEditorState.selectedMarkerType == ImGui::ImGradientHDRMarkerType::Alpha)
						{
							aSettings.ColorGradient.RemoveAlphaMarker(myColorGradientEditorState.selectedIndex);
							myColorGradientEditorState = ImGui::ImGradientHDRTemporaryState{};
							settingsChanged = true;
						}
					}
				}

				ImGui::TreePop();
			}
			else
			{
				myColorGradientEditorState = {};
			}
			//

			//Speed
			if (ImGui::TreeNode("Speed"))
			{
				ImGuiUtils::BeginParameters("Speed");
				settingsChanged |= ImGuiUtils::Parameter("Speed", aSettings.Speed, 0.5f, -FLT_MAX, FLT_MAX, "The speed of the particle when it spawns (centimeters/second)");
				settingsChanged |= ImGuiUtils::Parameter("Speed Variation", aSettings.SpeedVariation, 0.05f, 0.0f, FLT_MAX, "Introduces a variation in speed between the particles. For each particle, a number between 1 and Speed Variation will be generated and multiplied by the Speed of that particle");
				ImGuiUtils::EndParameters();

				ImGui::TreePop();
			}
			//

			//Acceleration
			bool accelerationOpen = ImGui::TreeNode("Acceleration");
			ImGuiUtils::ToolTip("The acceleration of a particle will affect the speed of that particle over time, like gravity (centimeters/second^2)");

			if (accelerationOpen)
			{
				ImGuiUtils::BeginParameters("Acceleration");
				settingsChanged |= ImGuiUtils::Parameter("Use Acceleration", aSettings.UseAcceleration);

				if (aSettings.UseAcceleration)
				{
					settingsChanged |= ImGuiUtils::Parameter("Acceleration", aSettings.Acceleration, 0.5f, -FLT_MAX, FLT_MAX, "The acceleration of the particles in each direction (centimeters/second^2)");
					settingsChanged |= ImGuiUtils::Parameter("Maximum Speed", aSettings.MaxSpeed, 0.5f, 0.0f, FLT_MAX, "The maximum speed allowed for any particle (centimeters/second)");
				}

				ImGuiUtils::EndParameters();

				ImGui::TreePop();
			}
			//

			//Rotation
			bool rotationOpen = ImGui::TreeNode("Rotation");

			if (rotationOpen)
			{
				//Rotationspeed
				ImGuiUtils::BeginParameters("Rotation Speed");
				settingsChanged |= ImGuiUtils::Parameter("Rotation Speed", aSettings.RotationSpeed, 0.05f, -FLT_MAX, FLT_MAX, "How fast should the particles spin clockwise");
				ImGuiUtils::EndParameters();

				//Global or local particles
				ImGuiUtils::BeginParameters("Random Spawn Rotation");
				settingsChanged |= ImGuiUtils::Parameter("Random Spawn Rotation", aSettings.RandomSpawnRotation, "Should the particles spawn with a random rotation");
				ImGuiUtils::EndParameters();

				ImGui::TreePop();
			}
			//

			//Global or local particles
			ImGuiUtils::BeginParameters("Global Particles");
			settingsChanged |= ImGuiUtils::Parameter("Global Particles", aSettings.Global, "Should the particle emitter spawn the particles in global space, or should the particles follow the emitters local space") || settingsChanged;
			ImGuiUtils::EndParameters();
			//

			ImGuiUtils::BeginParameters("Force Fields");
			settingsChanged |= ImGuiUtils::Parameter("Use Global Force Fields", myEmitterSettings.UseGlobalForceFields, "Should the particles be affected by global force fields");
			ImGuiUtils::EndParameters();

			//Looping
			ImGuiUtils::BeginParameters("Looping");
			settingsChanged |= ImGuiUtils::Parameter("Looping", aSettings.Looping, "Should the particle emitter loop or stop after one cycle");

			if (!aSettings.Looping)
			{
				settingsChanged |= ImGuiUtils::Parameter("Max Emit Time", aSettings.MaxEmitTime, 0.1f, 0.0f, FLT_MAX, "How long should a Non-looping emitter run for");
			}

			ImGuiUtils::EndParameters();
			//

			//Scaled Delta Time
			ImGuiUtils::BeginParameters("UseScaleDeltaTime");
			settingsChanged |= ImGuiUtils::Parameter("Use Scaled DelaTime", aSettings.ScaledDeltaTime, "Should the particle emitter use Scaled Delta Time (affected by time manipulation) or Unscaled Delta Time (Even if time is slowed, the particle emitter runs at full speed)");
			ImGuiUtils::EndParameters();
			//

			if (textureChanged)
			{
				myTexturePath = inputTexturePath;
				myParticleEmitter->LoadNewTexture(myTexturePath);
			}

			if (materialChanged)
			{
				myMaterialPath = inputMaterialPath;
				myParticleEmitter->LoadNewMaterial(myMaterialPath);
			}

			if (meshChanged)
			{
				myMeshPath = inputMeshPath;

				if (myParticleEmitter->GetMeshEmissionController())
				{
					myParticleEmitter->GetMeshEmissionController()->SetNewMesh(myMeshPath, true);
				}
			}

			if (animationChanged)
			{
				myAnimationPath = inputAnimationPath;

				if (myParticleEmitter->GetMeshEmissionController())
				{
					myAnimationTime = 0.0f;
					myAnimation = Firefly::ResourceCache::GetAsset<Firefly::Animation>(myAnimationPath, true);
					myParticleEmitter->GetMeshEmissionController()->SetNewAnimatedMesh(myAnimation->GetAnimatedMeshPath(), true);
				}
			}

			if (animationChanged)
			{
				myAnimationPath = inputAnimationPath;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Editor Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiUtils::BeginParameters("DebugLines");
			ImGuiUtils::Parameter("Debug Lines", myDrawDebugLines);
			ImGuiUtils::EndParameters();

			ImGuiUtils::BeginParameters("DrawGrid");
			ImGuiUtils::Parameter("Draw Grid", myDrawGrid);
			ImGuiUtils::EndParameters();

			ImGuiUtils::BeginParameters("DrawAtmosphere");
			ImGuiUtils::Parameter("Draw Atmosphere", myDrawAtmosphere);
			ImGuiUtils::EndParameters();

			ImGui::TreePop();
		}

		if (ImGui::Button("Start/Pause") || Utils::InputHandler::GetKeyDown('P'))
		{
			myParticleEmitter->StartPause();
		}
		ImGuiUtils::ToolTip("Starts or Pauses the emitter, Can also press 'P'");
		ImGui::SameLine();
		if (ImGui::Button("Reset") || Utils::InputHandler::GetKeyDown('O'))
		{
			myParticleEmitter->ClearParticles();
			myParticleEmitter->Start();
		}
		ImGuiUtils::ToolTip("Resets the emitter, Can also press 'O'");
	}

	ImGui::End();

	ImGui::SetNextWindowSize({ 100, 400 }, ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Force Fields"))
	{
		if (ImGui::Button("Add Force Field", { 110, 24 }))
		{
			ForceField forceField;
			forceField.ForceFieldType = ForceFieldType::Point;
			forceField.Position = { 0.0f, 0.0f, 0.0f };
			forceField.Range = 50;
			forceField.Force = 1000;
			forceField.Lerp = false;
			forceField.LerpType = Utils::LerpType::COUNT;
			forceField.LerpPower = 2.0f;
			forceField.Direction = { 1.0f, 0.0f, 0.0f };
			myEmitterSettings.LocalForceFields.push_back(forceField);

			mySelectedForceField = myEmitterSettings.LocalForceFields.size() - 1;

			settingsChanged = true;
		}

		if (!myEmitterSettings.LocalForceFields.empty())
		{
			for (int i = 0; i < myEmitterSettings.LocalForceFields.size(); ++i)
			{
				int flags = 0;

				if (i == mySelectedForceField)
				{
					flags |= ImGuiTreeNodeFlags_Selected;
					ImGui::SetNextItemOpen(true);
				}
				else 
				{
					ImGui::SetNextItemOpen(false);
				}

				auto& forceField = myEmitterSettings.LocalForceFields[i];

				if (ImGui::CollapsingHeader((std::string(forceField.ForceFieldType == ForceFieldType::Point ? "Point Field " : "Cuboid Field ") + "F: " + std::format("{:.3f}", forceField.Force) + " R: " + std::format("{:.3f}", forceField.Range) + "##" + std::to_string(i)).c_str(), flags))
				{
					mySelectedForceField = i;

					settingsChanged |= ImGui::Combo("##ForceFieldType", (int*)&forceField.ForceFieldType, GlobalForceFieldTypes);

					if (forceField.ForceFieldType == ForceFieldType::Point)
					{
						settingsChanged |= ImGui::DragFloat("Range##point", &forceField.Range, 10, 0.1f, FLT_MAX);
					}
					else if (forceField.ForceFieldType == ForceFieldType::Cuboid)
					{
						settingsChanged |= ImGui::DragFloat("Cube Half Width", &forceField.Range, 10.0f, 0.1f, FLT_MAX);
						settingsChanged |= ImGui::InputFloat3("Force Direction##cuboid", &forceField.Direction.x, "%.2f");
					}

					settingsChanged |= ImGui::DragFloat("Force", &forceField.Force, 100.0f, -FLT_MAX, FLT_MAX);

					settingsChanged |= ImGui::Checkbox("Lerp Force through field", &forceField.Lerp);

					if (forceField.Lerp)
					{
						settingsChanged |= ImGui::Combo("Lerp Type", (int*)&forceField.LerpType, LerpVisualizerWindow::LerpsCharCombo);
						settingsChanged |= ImGui::DragFloat("Lerp Power", &forceField.LerpPower, 0.05f, 0.01f, FLT_MAX);
						ImGuiUtils::ToolTip("See Lerp Visualizer for details");
					}
				}
				else
				{
					if (mySelectedForceField == i)
					{
						mySelectedForceField = -1;
					}
				}
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
			{
				if (mySelectedForceField >= 0 && mySelectedForceField < myEmitterSettings.LocalForceFields.size())
				{
					myEmitterSettings.LocalForceFields.erase(myEmitterSettings.LocalForceFields.begin() + mySelectedForceField);
					mySelectedForceField = -1;
					settingsChanged = true;
				}
			}
		}
	}

	ImGui::End();

	if (mySelectedForceField >= 0 && mySelectedForceField < myEmitterSettings.LocalForceFields.size())
	{
		if (UpdateGuizmo())
		{
			settingsChanged = true;
		}
	}

	if (settingsChanged)
	{
		myParticleEmitter->SetEmitterSettings(myEmitterSettings);
	}
}

void ParticleEditorWindow::OnEvent(Firefly::Event& aEvent)
{
}

void ParticleEditorWindow::WindowsMessages(UINT message, WPARAM wParam, LPARAM lParam)
{
}

void ParticleEditorWindow::SaveEmitter(const std::filesystem::path& aPath)
{
	const EmitterSettings& settings = myEmitterSettings;

	nlohmann::json json;
	json["TexturePath"] = myTexturePath;
	json["MeshPath"] = myMeshPath;
	json["MaterialPath"] = myMaterialPath;

	json["BlendState"] = settings.BlendState;

	json["EmitterType"] = settings.EmitterType;

	json["SpawnRate"] = settings.SpawnRate;
	json["MinLifeTime"] = settings.MinLifeTime;
	json["MaxLifeTime"] = settings.MaxLifeTime;

	json["StartScale"][0] = settings.StartScale.x;
	json["StartScale"][1] = settings.StartScale.y;
	json["StartScale"][2] = settings.StartScale.z;
	json["EndScale"][0] = settings.EndScale.x;
	json["EndScale"][1] = settings.EndScale.y;
	json["EndScale"][2] = settings.EndScale.z;
	json["ScaleVariation"] = settings.ScaleVariation;

	json["ColorGradient"]["ColorCount"] = settings.ColorGradient.ColorCount;
	json["ColorGradient"]["AlphaCount"] = settings.ColorGradient.AlphaCount;

	for (int i = 0; i < settings.ColorGradient.ColorCount; ++i)
	{
		json["ColorGradient"]["Colors"][i]["Position"] = settings.ColorGradient.Colors[i].Position;
		json["ColorGradient"]["Colors"][i]["Color"][0] = settings.ColorGradient.Colors[i].Color[0];
		json["ColorGradient"]["Colors"][i]["Color"][1] = settings.ColorGradient.Colors[i].Color[1];
		json["ColorGradient"]["Colors"][i]["Color"][2] = settings.ColorGradient.Colors[i].Color[2];
		json["ColorGradient"]["Colors"][i]["Intensity"] = settings.ColorGradient.Colors[i].Intensity;

	}
	for (int i = 0; i < settings.ColorGradient.AlphaCount; ++i)
	{
		json["ColorGradient"]["Alphas"][i]["Position"] = settings.ColorGradient.Alphas[i].Position;
		json["ColorGradient"]["Alphas"][i]["Alpha"] = settings.ColorGradient.Alphas[i].Alpha;
	}

	json["Speed"] = settings.Speed;
	json["SpeedVariation"] = settings.SpeedVariation;

	json["UseAcceleration"] = settings.UseAcceleration;
	json["Acceleration"][0] = settings.Acceleration.x;
	json["Acceleration"][1] = settings.Acceleration.y;
	json["Acceleration"][2] = settings.Acceleration.z;
	json["MaxSpeed"] = settings.MaxSpeed;

	json["UseGlobalForceFields"] = settings.UseGlobalForceFields;
	for (int i = 0; i < settings.LocalForceFields.size(); ++i)
	{
		const auto& forceField = settings.LocalForceFields[i];
		auto& forceJson = json["ForceFields"][i];

		forceJson["ForceFieldType"] = (int)forceField.ForceFieldType;

		forceJson["Position"][0] = forceField.Position.x;
		forceJson["Position"][1] = forceField.Position.y;
		forceJson["Position"][2] = forceField.Position.z;

		forceJson["Direction"][0] = forceField.Direction.x;
		forceJson["Direction"][1] = forceField.Direction.y;
		forceJson["Direction"][2] = forceField.Direction.z;

		forceJson["Range"] = forceField.Range;
		forceJson["Force"] = forceField.Force;

		forceJson["Lerp"] = forceField.Lerp;
		forceJson["LerpType"] = (int)forceField.LerpType;
		forceJson["LerpPower"] = forceField.LerpPower;
	}

	json["RotationSpeed"] = settings.RotationSpeed;
	json["RandomSpawnRotation"] = settings.RandomSpawnRotation;
	json["Global"] = settings.Global;
	json["Looping"] = settings.Looping;
	json["MaxEmitTime"] = settings.MaxEmitTime;
	json["ScaledDeltaTime"] = settings.ScaledDeltaTime;

	json["IsFlipbook"] = settings.IsFlipbook;
	json["Frames"] = settings.Frames;
	json["Rows"] = settings.Rows;
	json["Columns"] = settings.Columns;
	json["FrameRate"] = settings.FrameRate;

	json["LoopInnerChangeSpeed"] = settings.LoopInnerChangeSpeed;
	json["LoopOuterChangeSpeed"] = settings.LoopOuterChangeSpeed;
	json["BounceRadiusChange"] = settings.BounceRadiusChange;

	json["Height"] = settings.Height;

	json["InnerRadius"] = settings.InnerRadius;
	json["InnerRadiusChangeSpeed"] = settings.InnerRadiusChangeSpeed;
	json["InnerRadiusMax"] = settings.InnerRadiusMax;
	json["OuterRadius"] = settings.OuterRadius;
	json["OuterRadiusChangeSpeed"] = settings.OuterRadiusChangeSpeed;
	json["OuterRadiusMax"] = settings.OuterRadiusMax;
	json["SpawnOnEdge"] = settings.SpawnOnEdge;
	json["AimForEdge"] = settings.AimForEdge;

	json["Radius"] = settings.Radius;
	json["RadiusChangeSpeed"] = settings.RadiusChangeSpeed;
	json["RadiusMax"] = settings.RadiusMax;
	json["SpawnOnSurface"] = settings.SpawnOnSurface;

	json["InnerRectangleWidth"] = settings.InnerRectangleWidth;
	json["InnerRectangleHeight"] = settings.InnerRectangleHeight;
	json["OuterRectangleWidth"] = settings.OuterRectangleWidth;
	json["OuterRectangleHeight"] = settings.OuterRectangleHeight;

	std::ofstream file(aPath.string());
	file << std::setw(4) << json;
	file.close();
}

void ParticleEditorWindow::CalcActualPosAndSize()
{
	const auto initialWindowPos = ImGui::GetWindowPos();
	const auto initialWindowSize = ImGui::GetWindowSize();
	const auto initialMousePos = ImGui::GetMousePos();

	auto finalWindowPos = initialWindowPos;
	auto finalWindowSize = initialWindowSize;
	auto finalMousePos = initialMousePos;

	float expectedX = initialWindowSize.y;
	float expectedY = initialWindowSize.x;

	expectedX *= 16.0f / 9.0f;
	expectedY *= 9.0f / 16.0f;

	ImVec2 imageOffset = { 0.0f, 0.0f };

	if (expectedX < initialWindowSize.x)
	{
		imageOffset.x = (initialWindowSize.x - expectedX) / 2.0f;
		finalWindowSize.x = expectedX;
	}
	else if (expectedY < initialWindowSize.y)
	{
		imageOffset.y = (initialWindowSize.y - expectedY) / 2.0f;
		finalWindowSize.y = expectedY;
	}

	finalWindowPos.x += imageOffset.x;
	finalWindowPos.y += imageOffset.y;

	finalMousePos.x -= finalWindowPos.x;
	finalMousePos.y -= finalWindowPos.y;

	myActualWindowPos = finalWindowPos;
	myActualWindowSize = finalWindowSize;
	myActualMousePos = finalMousePos;
}

void ParticleEditorWindow::UpdateAnimatedMeshEmission()
{
	if (myAnimation)
	{
		myAnimationTime += Utils::Timer::GetUnscaledDeltaTime();
		while (myAnimationTime > myAnimation->GetDuration())
		{
			myAnimationTime -= myAnimation->GetDuration();
		}

		myBoneTransforms.resize(128);

		myAnimation->GetFrame(myAnimationTime, true).CalculateTransforms(myAnimation->GetAnimatedMesh()->GetSkeleton(), myBoneTransforms);

		myParticleEmitter->GetMeshEmissionController()->UpdateAnimatedBoneTransforms(myBoneTransforms.data());
	}
}

void ParticleEditorWindow::ResetEditor()
{
	myParticleEmitter->ClearParticles();
	myTexturePath = myDefaultTexturePath;
	myEmitterSettings = {};

	Firefly::ParticleEmitterTemplate temp;
	temp.TexturePath = myTexturePath;
	temp.EmitterSettings = myEmitterSettings;
	myParticleEmitter->Initialize(temp, true, false, true, 65536);
}
