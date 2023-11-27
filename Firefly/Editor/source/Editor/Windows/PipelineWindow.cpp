#include "EditorPch.h"
#include "PipelineWindow.h"
#include "WindowRegistry.h"
#include <Editor/Utilities/ImGuiUtils.h>
#include "Firefly/Asset/Importers/PipelineImporter.h"
REGISTER_WINDOW(PipelineWindow);
PipelineWindow::PipelineWindow() : EditorWindow("Pipeline editor")
{
	myHasPipeline = false;
}

void PipelineWindow::OnImGui()
{
	ImGuiUtils::BeginParameters();
	if (ImGuiUtils::FileParameter("Pipeline", myPipelinePath, ".ffpl"))
	{
		SetPipeline(myPipelinePath);
	}
	ImGuiUtils::EndParameters();

	if (myHasPipeline)
	{


		ImGuiUtils::BeginParameters();
		if (myGraphicsPipeline.HasShaderStage(Firefly::ShaderType::Vertex) && ImGuiUtils::Button("Open Vertex shader"))
		{
			std::filesystem::path fsPath(myGraphicsPipeline.shaderPaths[0]);
			ShellExecute(0, 0, fsPath.wstring().c_str(), 0, 0, SW_SHOW);
		}

		if (myGraphicsPipeline.HasShaderStage(Firefly::ShaderType::Geometry) && ImGuiUtils::Button("Open Geometry shader"))
		{
			std::filesystem::path fsPath(myGraphicsPipeline.shaderPaths[1]);
			ShellExecute(0, 0, fsPath.wstring().c_str(), 0, 0, SW_SHOW);
		}

		if (myGraphicsPipeline.HasShaderStage(Firefly::ShaderType::Pixel) && ImGuiUtils::Button("Open Pixel shader"))
		{
			std::filesystem::path fsPath(myGraphicsPipeline.shaderPaths[myGraphicsPipeline.HasShaderStage(Firefly::ShaderType::Geometry) ? 2 : 1]);
			ShellExecute(0, 0, fsPath.wstring().c_str(), 0, 0, SW_SHOW);
		}
		ImGuiUtils::EndParameters();
	}
}

void PipelineWindow::SetPipeline(const std::filesystem::path& aPath)
{
	myPipelinePath = aPath.string();
	auto info = Firefly::PipelineImporter::GetInfoFromPath(myPipelinePath);
	myGraphicsPipeline = info;
	myHasPipeline = true;
}
