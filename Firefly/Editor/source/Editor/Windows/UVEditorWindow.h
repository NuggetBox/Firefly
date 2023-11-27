#pragma once
#include "Editor/Windows/EditorWindow.h"
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector.h"
namespace Firefly
{
	class Entity;
	class Texture2D;
}
class Sprite2DComponent;
class Sprite3DComponent;
class UVEditorWindow : public EditorWindow
{
public:
	UVEditorWindow();
	virtual ~UVEditorWindow() = default;
	void OnImGui() override;

	static std::shared_ptr<EditorWindow> Create() { return std::make_shared<UVEditorWindow>(); }
	static std::string GetFactoryName() { return "UVEditorWindow"; }
	std::string GetName() const override { return GetFactoryName(); }
private:
	const Utils::Vector4f myBackgrundColor = { 0.271f, 0.267f, 0.366f, 1.f };

	std::array<Utils::Vector2f, 4> myCopiedUVs;

	Ptr<Sprite2DComponent> mySprite2DComponent;
	Ptr<Sprite3DComponent> mySprite3DComponent;
	Ref<Firefly::Texture2D> myTexture;

	bool my2DFlag;


	enum class SelectType
	{
		MinCorner,
		MaxCorner,
		Rectangle
	};
	SelectType mySelectType;
	bool myIsDragging;

	Utils::Vector2f myMinCornerDragRectOffset;
	Utils::Vector2f myMaxCornerDragRectOffset;



};

