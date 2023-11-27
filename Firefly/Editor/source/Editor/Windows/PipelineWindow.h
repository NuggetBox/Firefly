#pragma once
#include "EditorWindow.h"
#include "Firefly/Rendering/Pipeline/GraphicsPipeline.h"
class PipelineWindow : public EditorWindow
{
public:
	PipelineWindow();
	static std::string GetFactoryName() { return "Pipeline editor"; }
	std::string GetName() const override { return GetFactoryName(); }
	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<PipelineWindow>(); }
	void OnImGui() override;
	static void SetPipeline(const std::filesystem::path& aPath);
private:
	inline static Firefly::GraphicsPipelineInfo myGraphicsPipeline;
	inline static std::string myPipelinePath;
	inline static bool myHasPipeline;
};

