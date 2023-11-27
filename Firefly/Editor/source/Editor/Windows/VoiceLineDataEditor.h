#pragma once
#include "Editor/Windows/EditorWindow.h"
#include "Utils/Math/Vector3.hpp"
#include "Firefly/Core/Core.h"
namespace Firefly
{
	class VoiceLineData;
}
class VoiceLineDataEditor : public EditorWindow
{
public:
	VoiceLineDataEditor();
	virtual ~VoiceLineDataEditor() = default;

	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<VoiceLineDataEditor>(); }
	static std::string GetFactoryName() { return "VoiceLineDataEditor"; }
	std::string GetName() const override { return GetFactoryName(); }

	void Load(const std::filesystem::path& aPath);

private:

	void Save();

	Ref<Firefly::VoiceLineData> myVoiceLineDataRef;
	Ref<Firefly::VoiceLineData> myVoiceLineDataCopy;

	std::unordered_map<std::string, std::string> myInputTexts;
	std::string myAddGroupInputText;
};