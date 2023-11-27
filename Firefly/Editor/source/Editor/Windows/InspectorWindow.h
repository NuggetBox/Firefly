#pragma once

#include <Utils/Math/Quaternion.h>

#include "Editor/Windows/EditorWindow.h"

#include "Firefly/Asset/Texture/Texture2D.h"

#include "Utils/Math/Vector3.hpp"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
	struct Variable;
	enum class ParameterType;
}

class InspectorWindow : public EditorWindow
{
public:
	InspectorWindow();
	virtual ~InspectorWindow() = default;

	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<InspectorWindow>(); }
	static std::string GetFactoryName() { return "Inspector"; }
	std::string GetName() const override { return GetFactoryName(); }

private:
	enum class TransformType
	{
		Position, Rotation, Scale
	};

	void DrawEntity(Ptr<Firefly::Entity> aEntity);
	bool DrawVariable(Firefly::Variable& aVariable);
	bool DrawListVariable(Firefly::Variable& aListVariable);

	void DrawTransformFloatField(const char* aButtonLabel, const char* aFieldLabel, ImU32 aColor, float aWidth, float& aValue, bool& aStart, bool& aStop, float aPressedValue = 0.0f, bool aWonkyBool = false);
	void DrawQuatCopyButton(const char* aButtonLabel, const Utils::Vector3f& aValueToCopy);
	void DrawCopyButton(const char* aButtonLabel, TransformType aTransformType, const Utils::Vector3f& aValueToCopy);
	void DrawQuatPasteButton(const char* aButtonLabel, bool& aStart, bool& aStop);
	void DrawPasteButton(const char* aButtonLabel, TransformType aTransformType, Utils::Vector3f& aValueToPasteTo, bool& aStart, bool& aStop);
	bool DrawResetButton(const char* aLabel, Utils::Vector3f& aValue, const Utils::Vector3f& aResetValue, bool& aStart, bool& aStop);
	void DrawPositionSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aPosition, bool& aStart, bool& aStop);
	void DrawRotationSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aRotation, bool& aStart, bool& aStop);
	void DrawScaleSlider(const Ref<Firefly::Entity>& aEntity, Utils::Vector3f& aScale, bool& aStart, bool& aStop);

	void UpdateTransformComponent(Ptr<Firefly::Entity> aEntity);

	Ref<Firefly::Texture2D> myResetIcon;
	Ref<Firefly::Texture2D> myCopyIcon;
	Ref<Firefly::Texture2D> myPasteIcon;
	Ref<Firefly::Texture2D> myEntityFolderIcon;
	Ref<Firefly::Texture2D> myMaterialSwapIcon;

	std::string mySwapMaterialpath;

	std::string mySearchComponentString;

	bool myFloatFieldManualEditBegin = false;
	bool myFloatFieldDragBegin = false;
	bool myFloatFieldEditEnd = false;
	bool myZeroButtonPressed = false;
	bool myPasteButtonPressed = true;

	Utils::Quaternion myInitialQuat;

	Utils::Vector3f myCopiedPosition;
	Utils::Quaternion myCopiedRotation;
	Utils::Vector3f myCopiedScale;

	static constexpr float ButtonWidth = 25;
	static constexpr ImU32 ColorRed = IM_COL32(201, 44, 3, 255);
	static constexpr ImU32 ColorGreen = IM_COL32(102, 167, 1, 255);
	static constexpr ImU32 ColorBlue = IM_COL32(45, 125, 238, 255);
};