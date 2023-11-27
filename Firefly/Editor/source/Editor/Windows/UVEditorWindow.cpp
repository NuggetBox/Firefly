#include "EditorPch.h"
#include "UVEditorWindow.h"
#include "Editor/Windows/WindowRegistry.h"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Components/Sprites/Sprite3DComponent.h"
#include "Editor/EditorLayer.h"
#include "Editor/Utilities/ImGuiUtils.h"

#include "Firefly/Asset/Texture/Texture2D.h"

#include "imgui/imgui_internal.h"

REGISTER_WINDOW(UVEditorWindow);

UVEditorWindow::UVEditorWindow()
	: EditorWindow("UVEditor")
{
	mySprite2DComponent = {};
	mySprite3DComponent = {};
	my2DFlag = true;

	myIsDragging = false;

	myWindowFlags |= ImGuiWindowFlags_MenuBar;
}

void UVEditorWindow::OnImGui()
{
	mySprite2DComponent = {};
	mySprite3DComponent = {};
	myTexture = nullptr;

	if (EditorLayer::GetSelectedEntities().size() == 1)
	{
		auto selectedEnt = EditorLayer::GetSelectedEntities().front().lock();
		if (selectedEnt)
		{
			if (selectedEnt->HasComponent<Sprite2DComponent>())
			{
				mySprite2DComponent = selectedEnt->GetComponent<Sprite2DComponent>();
				my2DFlag = true;
				if (!mySprite2DComponent.expired())
				{
					myTexture = mySprite2DComponent.lock()->GetTexture();
				}
			}
			else if (selectedEnt->HasComponent<Sprite3DComponent>())
			{
				mySprite3DComponent = selectedEnt->GetComponent<Sprite3DComponent>();
				my2DFlag = false;
				if (!mySprite3DComponent.expired())
				{
					myTexture = mySprite3DComponent.lock()->GetTexture();
				}
			}
		}
	}

	if (mySprite2DComponent.expired() && mySprite3DComponent.expired())
	{
		ImGui::Text("The Selected Entity doesnt have a sprite component");
		return;
	}

	if (!myTexture)
	{
		ImGui::Text("The selected entity has a sprite component but doesn't have any texture assigned.");
		return;
	}

	if (!myTexture->IsLoaded())
	{
		ImGui::Text("The selected entity has a sprite component with a text .");
		return;
	}

	//calc ratio of tetxure
	auto availSize = ImGui::GetContentRegionAvail();
	ImVec2 imageSize = { static_cast<float>(myTexture->GetWidth()), static_cast<float>(myTexture->GetHeight()) };

	auto ratio = availSize.x / imageSize.x;
	imageSize.x = imageSize.x * ratio;
	imageSize.y = imageSize.y * ratio;

	if (imageSize.y > availSize.y)
	{
		auto yRatio = availSize.y / imageSize.y;
		imageSize.x = imageSize.x * yRatio;
		imageSize.y = imageSize.y * yRatio;
	}

	auto drawList = ImGui::GetWindowDrawList();
	auto startCursorScreenPos = ImGui::GetCursorScreenPos();
	drawList->AddRectFilled(ImGui::GetCursorScreenPos(), { ImGui::GetCursorScreenPos().x + imageSize.x, ImGui::GetCursorScreenPos().y + imageSize.y },
		ImGui::GetColorU32({ myBackgrundColor.x, myBackgrundColor.y, myBackgrundColor.z, myBackgrundColor.w }));
	ImGui::Image(myTexture->GetSRV().Get(), imageSize);

	//draw uv points
	Utils::Vector2f* uvs;
	if (my2DFlag)
	{
		uvs = mySprite2DComponent.lock()->GetUV();
	}
	else
	{
		uvs = mySprite3DComponent.lock()->GetUV();
	}

	//Menu bar
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::Button("Copy"))
			{
				myCopiedUVs[0] = uvs[0];
				myCopiedUVs[1] = uvs[1];
				myCopiedUVs[2] = uvs[2];
				myCopiedUVs[3] = uvs[3];
			}
			ImGui::SameLine();
			if (ImGui::Button("Paste"))
			{
				uvs[0] = myCopiedUVs[0];
				uvs[1] = myCopiedUVs[1];
				uvs[2] = myCopiedUVs[2];
				uvs[3] = myCopiedUVs[3];
			}

			ImGui::EndMenuBar();
		}
	}


	const ImVec4 selectedColor = { 0.12f, 0.15f, 0.84f, 1 };
	auto drawCornerOfRect = [&](Utils::Vector2f& aCornerRef, bool aIsMinCorner)
	{
		auto selectType = (aIsMinCorner ? SelectType::MinCorner : SelectType::MaxCorner);
		bool isSelected = mySelectType == selectType;

		ImVec4 color = { 1,1,1,1 };
		if (isSelected && myIsDragging)
		{
			color = selectedColor;
		}
		ImVec2 cornerScreenPos = { startCursorScreenPos.x + aCornerRef.x, startCursorScreenPos.y + aCornerRef.y };

		drawList->AddCircleFilled(cornerScreenPos, 5.f, ImGui::GetColorU32(color));

		std::string id = std::string("RectCornerHandle") + (aIsMinCorner ? "MinCorner" : "MaxCorner");
		ImGui::ItemAdd({ { cornerScreenPos.x - 5.f, cornerScreenPos.y - 5.f}, { cornerScreenPos.x + 5.f, cornerScreenPos.y + 5.f} }, ImGui::GetID(id.c_str()));

		if (ImGui::IsItemClicked())
		{
			mySelectType = selectType;
			myIsDragging = true;
		}

		if (myIsDragging && isSelected)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				myIsDragging = false;
			}

			auto mousePos = ImGui::GetMousePos();

			//clamp mouse to Image

			mousePos.x = Utils::Clamp(mousePos.x, startCursorScreenPos.x, startCursorScreenPos.x + imageSize.x);
			mousePos.y = Utils::Clamp(mousePos.y, startCursorScreenPos.y, startCursorScreenPos.y + imageSize.y);

			mousePos.x -= startCursorScreenPos.x;
			mousePos.y -= startCursorScreenPos.y;

			aCornerRef.x = mousePos.x;
			aCornerRef.y = mousePos.y;
		}
	};


	Utils::Vector2f minCornerPos = Utils::Vector2f(uvs[0].x * imageSize.x, (1 - uvs[0].y) * imageSize.y);
	Utils::Vector2f maxCornerPos = Utils::Vector2f(uvs[2].x * imageSize.x, (1 - uvs[2].y) * imageSize.y);

	ImVec2 minCornerScreenPos = { startCursorScreenPos.x + minCornerPos.x, startCursorScreenPos.y + minCornerPos.y };
	ImVec2 maxCornerScreenPos = { startCursorScreenPos.x + maxCornerPos.x, startCursorScreenPos.y + maxCornerPos.y };

	ImVec4 rectColor = { 1,1,1,1 };
	if (mySelectType == SelectType::Rectangle && myIsDragging)
	{
		rectColor = selectedColor;
	}
	drawList->AddRect(minCornerScreenPos, maxCornerScreenPos, ImGui::GetColorU32(rectColor));

	std::string rectID = std::string("RectHandleUVEditor");
	ImGui::ItemAdd({ { minCornerScreenPos.x,maxCornerScreenPos.y }, { maxCornerScreenPos.x,minCornerScreenPos.y } }, ImGui::GetID(rectID.c_str()));

	if (ImGui::IsItemClicked())
	{
		if (!myIsDragging)
		{
			mySelectType = SelectType::Rectangle;
			myIsDragging = true;
			auto mousePos = ImGui::GetMousePos();
			myMinCornerDragRectOffset = { minCornerScreenPos.x - mousePos.x, minCornerScreenPos.y - mousePos.y };
			myMaxCornerDragRectOffset = { maxCornerScreenPos.x - mousePos.x, maxCornerScreenPos.y - mousePos.y };
		}
	}

	if (mySelectType == SelectType::Rectangle && myIsDragging)
	{
		auto mousePos = ImGui::GetMousePos();

		mousePos.x = Utils::Clamp(mousePos.x, startCursorScreenPos.x - myMinCornerDragRectOffset.x, startCursorScreenPos.x + imageSize.x - myMaxCornerDragRectOffset.x);
		mousePos.y = Utils::Clamp(mousePos.y, startCursorScreenPos.y - myMaxCornerDragRectOffset.y, startCursorScreenPos.y + imageSize.y - myMinCornerDragRectOffset.y);
		mousePos.x -= startCursorScreenPos.x;
		mousePos.y -= startCursorScreenPos.y;

		minCornerPos = { myMinCornerDragRectOffset.x + mousePos.x, myMinCornerDragRectOffset.y + mousePos.y };
		maxCornerPos = { myMaxCornerDragRectOffset.x + mousePos.x, myMaxCornerDragRectOffset.y + mousePos.y };

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			myIsDragging = false;
		}
	}

	drawCornerOfRect(minCornerPos, true);
	drawCornerOfRect(maxCornerPos, false);


	const float& minX = minCornerPos.x / imageSize.x;
	const float& minY = 1 - (minCornerPos.y / imageSize.y);
	const float& maxX = maxCornerPos.x / imageSize.x;
	const float& maxY = 1 - (maxCornerPos.y / imageSize.y);

	uvs[0] = { minX, minY };
	uvs[1] = { maxX, minY };
	uvs[2] = { maxX, maxY };
	uvs[3] = { minX, maxY };


	if (my2DFlag)
	{
		mySprite2DComponent.lock()->UpdateInfoUVs();
	}
	else
	{
		mySprite3DComponent.lock()->UpdateInfoUVs();
	}
}
