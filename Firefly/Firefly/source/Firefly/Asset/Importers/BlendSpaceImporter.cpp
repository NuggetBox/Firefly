#include "FFpch.h"
#include "BlendSpaceImporter.h"
#include "Firefly/Asset/Animations/BlendSpace.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"
#include "Firefly/Asset/ResourceCache.h"
#include "nlohmann/json.hpp"

namespace Firefly
{
	Ref<BlendSpace> Firefly::BlendSpaceImporter::ImportBlendSpace(const std::filesystem::path& aPath)
	{
		std::ifstream file(aPath);
		if (!file.is_open())
		{
			LOGERROR("Failed to open blend space file: {}", aPath.string());
			return Ref<BlendSpace>();
		};

		nlohmann::json json;
		file >> json;
		file.close();

		Ref<BlendSpace> blendSpace = std::make_shared<BlendSpace>();

		std::string blendspaceType = json["Type"];
		if (blendspaceType == "1D")
		{
			blendSpace->myBlendspaceType = BlendspaceType::OneDimensional;
		}
		else if (blendspaceType == "2D")
		{
			blendSpace->myBlendspaceType = BlendspaceType::TwoDimensional;
		}

		blendSpace->myMeshPath = json["AnimatedMeshPath"];
		blendSpace->myMesh = ResourceCache::GetAsset<AnimatedMesh>(blendSpace->myMeshPath);

		blendSpace->myHorizontalAxis.Name = json["HorizontalAxis"]["Name"];
		blendSpace->myHorizontalAxis.Min = json["HorizontalAxis"]["Min"];
		blendSpace->myHorizontalAxis.Max = json["HorizontalAxis"]["Max"];

		if (blendSpace->myBlendspaceType == BlendspaceType::TwoDimensional)
		{
			blendSpace->myVerticalAxis.Name = json["VerticalAxis"]["Name"];
			blendSpace->myVerticalAxis.Min = json["VerticalAxis"]["Min"];
			blendSpace->myVerticalAxis.Max = json["VerticalAxis"]["Max"];
		}
		//if the blendspace has no entries then it will be null
		if (!json["Entries"].is_null())
		{
			for (auto& entry : json["Entries"])
			{
				BlendSpaceEntry blendSpaceEntry;
				blendSpaceEntry.Animation = ResourceCache::GetAsset<Animation>(entry["AnimationPath"].get<std::string>());
				blendSpaceEntry.HorizontalAxisPosition = entry["HorizontalAxisPosition"];
				blendSpaceEntry.VerticalAxisPosition = entry["VerticalAxisPosition"];
				blendSpaceEntry.Speed = entry["Speed"];
				blendSpace->myEntries.push_back(blendSpaceEntry);

			}
		}

		return blendSpace;
	}
	bool BlendSpaceImporter::ImportBlendSpace(Ref<BlendSpace> blendSpace)
	{
		std::ifstream file(blendSpace->GetPath());
		if (!file.is_open())
		{
			LOGERROR("Failed to open blend space file: {}", blendSpace->GetPath().string());
			return false;
		};

		nlohmann::json json;
		file >> json;
		file.close();

		std::string blendspaceType = json["Type"];
		if (blendspaceType == "1D")
		{
			blendSpace->myBlendspaceType = BlendspaceType::OneDimensional;
		}
		else if (blendspaceType == "2D")
		{
			blendSpace->myBlendspaceType = BlendspaceType::TwoDimensional;
		}

		blendSpace->myMeshPath = json["AnimatedMeshPath"];
		blendSpace->myMesh = ResourceCache::GetAsset<AnimatedMesh>(blendSpace->myMeshPath);

		blendSpace->myHorizontalAxis.Name = json["HorizontalAxis"]["Name"];
		blendSpace->myHorizontalAxis.Min = json["HorizontalAxis"]["Min"];
		blendSpace->myHorizontalAxis.Max = json["HorizontalAxis"]["Max"];

		if (blendSpace->myBlendspaceType == BlendspaceType::TwoDimensional)
		{
			blendSpace->myVerticalAxis.Name = json["VerticalAxis"]["Name"];
			blendSpace->myVerticalAxis.Min = json["VerticalAxis"]["Min"];
			blendSpace->myVerticalAxis.Max = json["VerticalAxis"]["Max"];

			//load triangles aswell
			if (json.find("Triangles") != json.end())
			{
				if (!json["Triangles"].is_null())
				{
					for (auto& triangle : json["Triangles"])
					{
						blendSpace->myTriangles.push_back({ triangle[0], triangle[1], triangle[2]});
					}
				}
			}
		}
		//if the blendspace has no entries then it will be null
		if (!json["Entries"].is_null())
		{
			for (auto& entry : json["Entries"])
			{
				BlendSpaceEntry blendSpaceEntry;
				blendSpaceEntry.Animation = ResourceCache::GetAsset<Animation>(entry["AnimationPath"].get<std::string>());
				blendSpaceEntry.HorizontalAxisPosition = entry["HorizontalAxisPosition"];
				blendSpaceEntry.VerticalAxisPosition = entry["VerticalAxisPosition"];
				blendSpaceEntry.Speed = entry["Speed"];
				blendSpace->myEntries.push_back(blendSpaceEntry);

			}
		}

		return true;
	}
}