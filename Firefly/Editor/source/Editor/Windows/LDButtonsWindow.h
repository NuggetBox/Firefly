#pragma once

#include "Editor/Windows/EditorWindow.h"
#include "Utils/Math/Vector3.hpp"
#include <vector>
#include "Utils/Math/Transform.h"
#include "Firefly/Core/Core.h"


namespace Firefly
{
	class Entity;
	struct Parameter;
	class Mesh;
}

class LDButtonsWindow : public EditorWindow
{
public:
	LDButtonsWindow();
	virtual ~LDButtonsWindow() = default;

	void OnImGui() override;

	void FindMeshComponentAndExtract(Ptr<Firefly::Entity> selectedEntity, std::vector<std::pair<Ref<Firefly::Mesh>, Utils::Transform>>& vector);

	bool ExtractMesh(std::shared_ptr<Firefly::Entity>& entityPtr, std::vector<std::pair<Ref<Firefly::Mesh>, Utils::Transform>>& vector);

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<LDButtonsWindow>(); }
	static std::string GetFactoryName() { return "LDButtonsWindow"; }
	std::string GetName() const override { return GetFactoryName(); }


private:
	std::vector<std::string> myPrefabsToRandomlyOffset;
	Utils::Vector3f myOffsetMin;
	Utils::Vector3f myOffsetMax;

	struct ReplacePrefabsStruct
	{
		std::string PrefabToReplace;
		std::string PrefabToReplaceWith;

	} myReplacePrefabData;

	struct ReplaceMeshObjectStruct
	{
		std::string MeshToReplace;
		std::string PrefabToReplaceWith;

	} myReplaceMeshObjectData;
};