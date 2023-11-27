#include "EditorPch.h"
#include "MaterialEditorWindow.h"
#include "Firefly/Asset/Material/MaterialAsset.h"
#include "Firefly/Asset/ResourceCache.h"
#include <Firefly/Rendering/Shader/ShaderLibrary.h>
#include <Firefly/Asset/Importers/ShaderImporter.h>
#include "Editor/Windows/WindowRegistry.h"
#include <Firefly/Rendering/Renderer.h>
#include "Firefly/Core/DXHelpers.h"
#include <Utils/Timer.h>
#include <Editor/Utilities/ImGuiUtils.h>
#include "Firefly/Rendering/Framebuffer.h"

#include "Utils/InputHandler.h"
#include <Firefly/Asset/Importers/PipelineImporter.h>
#include <Firefly/Rendering/Pipeline/PipelineLibrary.h>
#include <Firefly/Event/EditorEvents.h>

REGISTER_WINDOW(MaterialEditorWindow);
MaterialEditorWindow::MaterialEditorWindow() : EditorWindow("Material Editor")
{
	myRenderSceneId = Firefly::Renderer::InitializeScene();
	myMesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>("FireflyEngine/Defaults/FireFly_MatBall.mesh");
	Firefly::CameraInfo info{};
	info.FarPlane = 10000;
	info.NearPlane = 10.f;
	info.Fov = 90;
	info.ResolutionX = 1280;
	info.ResolutionY = 720;
	myCamera = Firefly::Camera::Create(info);
	myCamera->GetTransform().SetPosition({ 0,0,-300 });
	mySkyLightPath = "FireflyEngine/Defaults/skansen_cubemap.dds";
	mySkyLight = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(mySkyLightPath);
	myZoom = 200;
	myCameraSpeed = 100.f;

	mySaveIcon = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>("Editor/Icons/icon_save.dds", true);
}

void MaterialEditorWindow::OnImGui()
{
	if (myWaitOnShaderCompletedCompiling)
	{
		if (Firefly::ShaderLibrary::KeyHasCompiled(myCurrentShader))
		{
			auto shader = Firefly::ShaderLibrary::GetShader(myCurrentShader, Firefly::ShaderType::Pixel);
			shader->GetBoundResources().clear();
			shader->ReflectShader();
			std::vector<Firefly::TexturePacket> resource;
			auto& materialInfo = myMaterial->GetInfo();
			for (auto& i : shader->GetBoundResources())
			{
				if (i.type == Firefly::BoundType::Texture)
				{
					auto& r = resource.emplace_back();
					r.BindPoint = i.bindPoint;
					r.ShaderStage = Firefly::ShaderType::Pixel;
					r.VariableName = i.name;

				}
				if (i.type == Firefly::BoundType::ContantBuffer)
				{
					if (i.name == "MaterialInfo")
					{
						materialInfo.MaterialData.bindpoint = i.bindPoint;
						materialInfo.MaterialData.varibles = i.varibles;
						materialInfo.MaterialData.data.resize(i.size);
						LOGINFO("Amount of Bound resources {}", i.varibles.size());
					}
				}
			}
			for (size_t i = 0; i < resource.size(); ++i)
			{
				if (materialInfo.Textures.size() <= i)
				{
					materialInfo.Textures.emplace_back();
				}
				resource[i].TexturePath = materialInfo.Textures[i].TexturePath;
				resource[i].Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(resource[i].TexturePath, true);
			}

			materialInfo.Textures = resource;
			myWaitOnShaderCompletedCompiling = false;
		}
	}

	ImGui::Begin("Material preview");
	myIsFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

	RenderScene();

	auto newSize = Utils::Vector2<uint32_t>(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
	ImVec2 contentRegion = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
	Firefly::Renderer::GetSceneFrameBuffer(myRenderSceneId)->Resize(newSize);
	myCamera->SetSizeX(contentRegion.x);
	myCamera->SetSizeY(contentRegion.y);

	ImGui::Image(Firefly::Renderer::GetSceneFrameBuffer(myRenderSceneId)->GetColorAttachment(0).Get(), contentRegion);
	ImGui::SetCursorPos({ contentRegion.x - (contentRegion.x - 16), contentRegion.y - (contentRegion.y - 38) });
	if (ImGui::ImageButton(mySaveIcon->GetSRV().Get(), { 32, 32 }))
	{
		if (myMaterial)
		{
			if (myMaterial->IsLoaded())
			{
				FILE* file_handle;
				errno_t file_open_error;
				if ((file_open_error = fopen_s(&file_handle, myInputPath.string().c_str(), "a")) != 0)
				{
					ImGuiUtils::NotifyError("Could not save material: {}. Have you checked out the file?", myInputPath.string());
				}
				else
				{
					ImGuiUtils::NotifySuccess("Material: {} Has been saved!", myInputPath.string());
					fclose(file_handle);
					Refire(myInputPath.string(), myMaterial);
				}
			}
		}
	}
	if (ImGui::IsWindowHovered() || ImGui::IsWindowFocused())
	{
		ArcBallCamera();

		float speed = myZoomSpeed;

		if (Utils::InputHandler::GetKeyHeld(VK_SHIFT))
		{
			speed *= 2;
		}

		myZoom -= Utils::InputHandler::GetScrollWheelDelta() * speed;
		if (Utils::InputHandler::GetMiddleClickHeld())
		{
			Utils::Vector2f deltaPos = { (float)Utils::InputHandler::GetMouseDelta().x, (float)Utils::InputHandler::GetMouseDelta().y };
			myAnkerPoint += myCamera->GetTransform().GetUp() * deltaPos.y * Utils::Timer::GetDeltaTime() * myCameraSpeed;
			myAnkerPoint -= myCamera->GetTransform().GetRight() * deltaPos.x * Utils::Timer::GetDeltaTime() * myCameraSpeed;
		}
	}

	myCamera->GetTransform().SetPosition(myAnkerPoint);
	const Utils::Vector3f focusPoint = myAnkerPoint - myCamera->GetTransform().GetForward() * myZoom;
	myCamera->GetTransform().SetPosition(focusPoint);
	ImGui::End();


	if (ImGui::CollapsingHeader("Editor Settings", ImGuiTreeNodeFlags_NoTreePushOnOpen))
	{
		ImGui::TreePush();
		if (ImGui::CollapsingHeader("Environment Settings"))
		{
			ImGui::Indent();
			ImGuiUtils::BeginParameters("EnvironmentSettings");
			ImGuiUtils::SliderParameter("Bloom strength", myBloomStrength, 0.f, 1.f);
			std::string path = mySkyLightPath;
			if (ImGuiUtils::FileParameter("Skylight", path, ".dds"))
			{
				auto skylight = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(path, true);
				if (skylight)
				{
					mySkyLight = skylight;
					mySkyLightPath = path;
				}
			}
			ImGuiUtils::EndParameters();
			ImGui::Unindent();
		}
		if (ImGui::CollapsingHeader("Camera Settings"))
		{
			ImGuiUtils::BeginParameters("asfasfas");
			myFOV = myCamera->GetFov();
			if (ImGuiUtils::SliderParameter("FOV", myFOV, 70.f, 130.f))
			{
				myCamera->SetFov(myFOV);
			}
			ImGuiUtils::Parameter("Move speed", myCameraSpeed, 0.5f, 10.f, 1000.f);
			ImGuiUtils::Parameter("Zoom speed", myZoomSpeed, 0.2f);

			ImGuiUtils::EndParameters();
		}
		if (ImGui::CollapsingHeader("Model Settings"))
		{
			ImGui::Indent();
			ImGuiUtils::BeginParameters("ModelSettings");
			std::string path = myCurrentMesh.string();
			if (ImGuiUtils::FileParameter("Custom model", path, ".mesh"))
			{
				auto mesh = Firefly::ResourceCache::GetAsset<Firefly::Mesh>(path, true);
				if (mesh)
				{
					myMesh = mesh;
					myCurrentMesh = path;
				}
			}

			auto rotation = myTransform.GetRotation();
			if (ImGuiUtils::SliderParameter("Rotation", rotation, -180.f, 180.f))
			{
				myTransform.SetRotation(rotation);
			}
			ImGuiUtils::EndParameters();
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Material Settings", ImGuiTreeNodeFlags_NoTreePushOnOpen))
	{
		ImGui::TreePush();
		RenderMaterialProperties();
		ImGui::TreePop();
	}
}

void MaterialEditorWindow::RenderScene()
{
	switch (myMaterialType)
	{
	case Firefly::PipelineType::Graphics:
		RenderMeshMaterial();
		break;
	case Firefly::PipelineType::Postprocess:
		RenderPostProcessMaterial();
		break;
	case Firefly::PipelineType::Decal:
		RenderDecalMaterial();
		break;
	default:
		break;
	}
}

void MaterialEditorWindow::RenderMaterialProperties()
{
	if (ImGui::CollapsingHeader("Material General"))
	{
		ImGui::Indent();
		ImGuiUtils::BeginParameters("material Input");
		std::string imInput = myInputPath.string();
		std::string pipeinput = myPipelinePath.string();
		if (ImGuiUtils::FileParameter("Material", imInput, ".mat"))
		{
			myInputPath = imInput;

			SetMaterial(myInputPath);
		}
		if (myMaterial)
		{
			if (myMaterial->IsLoaded())
			{
				if (ImGuiUtils::Button("Reset to Default Opaque"))
				{
					myMaterial->GetInfo().PipelineID = Firefly::DefaultDeferredPipeline;
					myCurrentShader = myMaterial->GetInfo().PipelineID;
					Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
					Refire(myInputPath.string(), myMaterial);
					myPipelinePath = "Default Opaque";
				}

				if (ImGuiUtils::Button("Reset to Default Transparent"))
				{
					myMaterial->GetInfo().PipelineID = Firefly::DefaultForwardPipeline;
					myCurrentShader = myMaterial->GetInfo().PipelineID;
					Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
					Refire(myInputPath.string(), myMaterial);
					myPipelinePath = "Default Transparent";
				}

				if (ImGuiUtils::Button("Reset to Default Particle"))
				{
					myMaterial->GetInfo().PipelineID = Firefly::DefaultParticlePipeline;
					myCurrentShader = myMaterial->GetInfo().PipelineID;
					Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
					Refire(myInputPath.string(), myMaterial);
					myPipelinePath = "Default Particle";
				}

				if (ImGuiUtils::FileParameter("Pipeline", pipeinput, ".ffpl"))
				{
					myPipelinePath = pipeinput;
					myMaterial->GetInfo().PipelineID = Firefly::PipelineImporter::GetInfoFromPath(myPipelinePath).Hash;
					myCurrentShader = myMaterial->GetInfo().PipelineID;
					Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
					Refire(myInputPath.string(), myMaterial);
				}

				if (ImGuiUtils::Button("Refresh pipline"))
				{
					Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
					Refire(myInputPath.string(), myMaterial);
				}
			}
		}
		ImGuiUtils::EndParameters();
		ImGui::Unindent();
	}
	if (myMaterial)
	{

		if (myMaterial->IsLoaded())
		{
			auto cullstateNames = Firefly::CullStateNamesToStringVector();
			auto blendstateNames = Firefly::BlendStateNamesToStringVector();
			auto depthstateNames = Firefly::DepthStencilStateNamesToStringVector();

			if (ImGui::CollapsingHeader("Dynamic states"))
			{
				ImGui::Indent();
				ImGuiUtils::BeginParameters("Dynamic states");
				ImGuiUtils::EnumParameter("Cull mode", myCullStateIndex, cullstateNames);
				ImGuiUtils::EnumParameter("Blend mode", myBlendStateIndex, blendstateNames);
				ImGuiUtils::EnumParameter("Depth read mode", myDepthStateIndex, depthstateNames);
				ImGuiUtils::EndParameters();
				ImGui::Unindent();
			}

			myMaterial->GetInfo().BlendMode = (Firefly::BlendState)myBlendStateIndex;
			myMaterial->GetInfo().CullMode = (Firefly::CullState)myCullStateIndex;
			myMaterial->GetInfo().DepthMode = (Firefly::DepthStencilState)myDepthStateIndex;

			if (ImGui::CollapsingHeader("Textures"))
			{
				ImGui::Indent();
				ImGuiUtils::BeginParameters("Texture");
				for (size_t i = 0; i < myMaterial->GetInfo().Textures.size(); ++i)
				{
					auto& texture = myMaterial->GetInfo().Textures[i];
					bool changed = (ImGuiUtils::FileParameter(texture.VariableName, myTexturePath[i], ".dds"));
					texture.TexturePath = myTexturePath[i];
					texture.ShaderStage = Firefly::ShaderType::Pixel;
					if (changed)
					{
						texture.Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(myTexturePath[i], true);
						Refire(myInputPath.string(), myMaterial);
					}
				}
				ImGuiUtils::EndParameters();
				ImGui::Unindent();
			}

			if (ImGui::CollapsingHeader("Material properties"))
			{
				ImGui::Indent();
				ImGuiUtils::BeginParameters("Material properties");

				ImGuiUtils::Parameter("Should Blend", myMaterial->GetInfo().ShouldBlend, "This is for when you want a transparent object to blend with other objects");

				ImGuiUtils::Parameter("Is First person", myMaterial->GetInfo().ShouldBeFPS, "This is for objects that are in the FPS camera like fps gun and arms");


				auto& bytes = myMaterial->GetInfo().MaterialData.data;
				size_t byteOffset = 0;
				for (auto& var : myMaterial->GetInfo().MaterialData.varibles)
				{
					switch (var.VariableType)
					{
					case Firefly::Value::Bool:
					{
						ImGuiUtils::Parameter(var.Name, *(bool*)&bytes[byteOffset]);
						byteOffset += 4;
						break;
					}
					case Firefly::Value::Float:
					{
						if (var.Type == Firefly::VarType::Slider)
						{
							ImGuiUtils::SliderParameter(var.Name, *(float*)&bytes[byteOffset], 0.f, 1.f);
						}
						else
						{
							ImGuiUtils::Parameter(var.Name, *(float*)&bytes[byteOffset]);
						}
						byteOffset += sizeof(float);
						break;
					}
					case Firefly::Value::Float2:
					{
						ImGuiUtils::Parameter(var.Name, *(Utils::Vector2f*)&bytes[byteOffset]);
						byteOffset += sizeof(Utils::Vector2f);
						break;
					}
					case Firefly::Value::Float3:
					{
						ImGuiUtils::Parameter(var.Name, *(Utils::Vector3f*)&bytes[byteOffset]);
						byteOffset += sizeof(Utils::Vector3f);
						break;
					}
					case Firefly::Value::Float4:
					{
						ImGuiUtils::Parameter(var.Name, *(Utils::Vector4f*)&bytes[byteOffset]);
						byteOffset += sizeof(Utils::Vector4f);
						break;
					}
					case Firefly::Value::Color4:
					{
						ImGuiUtils::ColorParameter(var.Name, *(Utils::Vector4f*)&bytes[byteOffset], true);
						byteOffset += sizeof(Utils::Vector4f);
						break;
					}
					case Firefly::Value::UInt:
					{
						ImGuiUtils::Parameter(var.Name, *(int*)&bytes[byteOffset]);
						byteOffset += sizeof(int);
						break;
					}

					default:
						break;
					}
				}
				ImGuiUtils::EndParameters();
				ImGui::Unindent();
			}


			if (ImGui::CollapsingHeader("Stats for nerds"))
			{
				ImGui::Indent();
				ImGui::PushID("statis for nerds");
				ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadInnerX);
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Pipeline ID");
				ImGui::PushItemWidth(ImGui::GetColumnWidth());
				ImGui::TableNextColumn();
				ImGui::Text("%zu", myMaterial->GetInfo().PipelineID);
				ImGui::EndTable();

				if (ImGui::CollapsingHeader("Material data"))
				{
					ImGui::Indent();
					ImGui::BeginTable("##dfadf", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadInnerX);
					for (size_t i = 0; i < myMaterial->GetInfo().MaterialData.data.size(); i++)
					{
						ImGui::TableNextColumn();
						ImGui::Text("[%d]", i);
						ImGui::PushItemWidth(ImGui::GetColumnWidth());
						ImGui::TableNextColumn();
						ImGui::Text("%d", myMaterial->GetInfo().MaterialData.data[i]);
					}
					ImGui::EndTable();
					ImGui::Unindent();
				}
				ImGui::PopID();
				ImGui::Unindent();
			}
		}
	}
}

void MaterialEditorWindow::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);
	dispatcher.Dispatch<EditorFileEvent>([&](EditorFileEvent& aEvent)
		{
			auto& file = aEvent.GetFilePath();
			if (myMaterial == nullptr)
			{
				return false;
			}
			if (file.extension() == ".hlsl")
			{
				auto key = Firefly::ShaderLibrary::GetKeyFromPath(file);
				myWaitOnShaderCompletedCompiling = true;
				if (Firefly::ShaderLibrary::IsHash(key))
				{
					auto hash = Firefly::ShaderLibrary::GetHash(key);

					if (myCurrentShader == hash)
					{
						Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
						Refire(myInputPath.string(), myMaterial);
					}
				}
			}

			return false;
		});
}

void MaterialEditorWindow::Reset()
{
	myMaterial = nullptr;
	myInputPath = "";
}

void MaterialEditorWindow::SetMaterial(const std::filesystem::path& aPath)
{
	if (aPath.empty())
	{
		LOGERROR("Material path given were empty!");
		return;
	}

	myInputPath = aPath;

	myMaterial = Firefly::ResourceCache::GetAsset<Firefly::MaterialAsset>(myInputPath, true);
	while (!myMaterial->IsLoaded()) {}
	{
		myCullStateIndex = (uint32_t)myMaterial->GetInfo().CullMode;
		myDepthStateIndex = (uint32_t)myMaterial->GetInfo().DepthMode;
		myBlendStateIndex = (uint32_t)myMaterial->GetInfo().BlendMode;
		myCurrentShader = myMaterial->GetInfo().PipelineID;
		myPipelinePath = Firefly::PipelineLibrary::Get(myCurrentShader).GetInfo().Path;
		for (size_t i = 0; i < myMaterial->GetInfo().Textures.size(); ++i)
		{
			auto& texture = myMaterial->GetInfo().Textures[i];
			myTexturePath[i] = texture.TexturePath.string();
			texture.Texture = Firefly::ResourceCache::GetAsset<Firefly::Texture2D>(myTexturePath[i], true);
		}


		Firefly::PipelineLibrary::Get(myCurrentShader).Reload();
		myMaterialType = Firefly::PipelineLibrary::Get(myCurrentShader).GetInfo().PipeType;
		Refire(aPath.string(), myMaterial);
	}
}

void MaterialEditorWindow::Refire(const std::string path, Ref<Firefly::MaterialAsset> aMat)
{
	Firefly::MaterialImporter::RefreshMaterial(aMat, path);
	Firefly::MaterialImporter::ExportMaterial(aMat, path);
}

void MaterialEditorWindow::ArcBallCamera()
{
	if (Utils::InputHandler::GetRightClickHeld())
	{
		auto& transform = myCamera->GetTransform();
		Utils::Vector2f deltaPos = { (float)Utils::InputHandler::GetMouseDelta().x, (float)Utils::InputHandler::GetMouseDelta().y };

		const float yawSign = transform.GetUp().y < 0.f ? -1.f : 1.f;

		myYawdelta = yawSign * deltaPos.x;
		myPitchDelta = deltaPos.y;

		transform.AddRotation({ 0.0f, myYawdelta, 0.f });
		transform.AddLocalRotation({ myPitchDelta, 0.f, 0.f });

	}

}

void MaterialEditorWindow::RenderMeshMaterial()
{
	Firefly::Renderer::BeginScene(myRenderSceneId);
	Firefly::PostProcessInfo inf{};
	inf.Enable = true;
	inf.Data.Padding.z = myBloomStrength;
	Firefly::Renderer::Submit(inf);

	Firefly::EnvironmentData data;
	data.EnvironmentMap = mySkyLight;
	data.Intensity = 1;
	Firefly::Renderer::Submit(data);

	Firefly::MeshSubmitInfo cmd(myTransform.GetMatrix());
	if (myMesh)
	{
		if (myMaterial)
		{
			cmd.Material = (myMaterial);
		}
		for (auto& submesh : myMesh->GetSubMeshes())
		{
			cmd.Mesh = &submesh;
			Firefly::Renderer::Submit(cmd);
		}
	}


	Firefly::Renderer::SubmitActiveCamera(myCamera);
	Firefly::DirLightPacket pack{};
	pack.Direction = { -0.71, 0.71, -0.71, 1 };
	pack.ColorAndIntensity = { 1,1,1,1 };
	pack.dirLightInfo.x = 0;
	Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res1024);
	Firefly::Renderer::EndScene();
}

void MaterialEditorWindow::RenderPostProcessMaterial()
{
	Firefly::Renderer::BeginScene(myRenderSceneId);
	Firefly::PostProcessInfo inf{};
	inf.Enable = true;
	inf.Data.Padding.z = myBloomStrength;
	Firefly::Renderer::Submit(inf);

	Firefly::EnvironmentData data;
	data.EnvironmentMap = mySkyLight;
	data.Intensity = 1;
	Firefly::Renderer::Submit(data);

	Firefly::MeshSubmitInfo cmd(myTransform.GetMatrix());
	if (myMesh)
	{
		if (myMaterial)
		{
			cmd.Material = Firefly::Renderer::GetDefaultMaterial();
		}
		for (auto& submesh : myMesh->GetSubMeshes())
		{
			cmd.Mesh = &submesh;
			Firefly::Renderer::Submit(cmd);
		}
	}

	Firefly::CustomPostprocessInfo customPostprocessInfo = {};

	customPostprocessInfo.Passorder = Firefly::PassOrder::First;
	customPostprocessInfo.Material = myMaterial;
	Firefly::Renderer::Submit(customPostprocessInfo);

	Firefly::Renderer::SubmitActiveCamera(myCamera);
	Firefly::DirLightPacket pack{};
	pack.Direction = { -0.71, 0.71, -0.71, 1 };
	pack.ColorAndIntensity = { 1,1,1,1 };
	pack.dirLightInfo.x = 0;
	Firefly::Renderer::Submit(pack, Firefly::ShadowResolutions::res1024);
	Firefly::Renderer::EndScene();
}

void MaterialEditorWindow::RenderDecalMaterial()
{
}
