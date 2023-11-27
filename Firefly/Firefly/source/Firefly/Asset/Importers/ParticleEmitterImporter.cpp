#include "FFpch.h"
#include "ParticleEmitterImporter.h"
#include "nlohmann/json.hpp"

namespace Firefly
{
	bool ParticleEmitterImporter::ImportEmitterTemplate(Ref<ParticleEmitterTemplate> aAsset)
	{
		std::ifstream file(aAsset->GetPath().string());
		nlohmann::json json = nlohmann::json::parse(file);

		aAsset->TexturePath = json["TexturePath"].get<std::string>();

		if (json.contains("MeshPath"))
			aAsset->MeshPath = json["MeshPath"].get<std::string>();

		if (json.contains("MaterialPath"))
			aAsset->MaterialPath = json["MaterialPath"].get<std::string>();

		EmitterSettings& settings = aAsset->EmitterSettings;

		if (json.contains("BlendState"))
			settings.BlendState = json["BlendState"];

		settings.EmitterType = json["EmitterType"];

		settings.SpawnRate = json["SpawnRate"];
		settings.MinLifeTime = json["MinLifeTime"];
		settings.MaxLifeTime = json["MaxLifeTime"];

		settings.StartScale.x = json["StartScale"][0];
		settings.StartScale.y = json["StartScale"][1];
		settings.StartScale.z = json["StartScale"][2];
		settings.EndScale.x = json["EndScale"][0];
		settings.EndScale.y = json["EndScale"][1];
		settings.EndScale.z = json["EndScale"][2];
		settings.ScaleVariation = json["ScaleVariation"];

		settings.ColorGradient.Clear();

		if (json.contains("ColorGradient"))
		{
			auto& gradient = json["ColorGradient"];

			if (gradient.contains("ColorCount") && gradient.contains("AlphaCount"))
			{
				for (int i = 0; i < gradient["ColorCount"]; ++i)
				{
					if (gradient.contains("Colors"))
					{
						auto& colors = gradient["Colors"];

						if (colors.is_array() && i < colors.size())
						{
							auto& color = colors[i];

							ImGui::ImGradientHDRState::ColorMarker colorMarker{};

							if (color.contains("Position"))
								colorMarker.Position = color["Position"];

							if (color.contains("Color"))
							{
								auto& colorColor = color["Color"];

								colorMarker.Color[0] = colorColor[0];
								colorMarker.Color[1] = colorColor[1];
								colorMarker.Color[2] = colorColor[2];
							}

							if (color.contains("Intensity"))
								colorMarker.Intensity = color["Intensity"];

							settings.ColorGradient.AddColorMarker(colorMarker);
						}
					}
				}

				for (int i = 0; i < gradient["AlphaCount"]; ++i)
				{
					if (gradient.contains("Alphas"))
					{
						auto& alphas = gradient["Alphas"];

						if (alphas.is_array() && i < alphas.size())
						{
							if (alphas[i].contains("Position") && alphas[i].contains("Alpha"))
							{
								settings.ColorGradient.AddAlphaMarker(alphas[i]["Position"], alphas[i]["Alpha"]);
							}
						}
						else
						{
							settings.ColorGradient.AddAlphaMarker(0.0f, 1.0f);
							settings.ColorGradient.AddAlphaMarker(1.0f, 1.0f);
						}
					}
				}
			}
		}
		else if (json.contains("StartColor") && json.contains("EndColor"))
		{
			settings.ColorGradient.AddAlphaMarker(0.0f, json["StartColor"][3]);
			settings.ColorGradient.AddAlphaMarker(1.0f, json["EndColor"][3]);
			settings.ColorGradient.AddColorMarker(0.0f, { json["StartColor"][0], json["StartColor"][1], json["StartColor"][2] }, 1);
			settings.ColorGradient.AddColorMarker(1.0f, { json["EndColor"][0], json["EndColor"][1], json["EndColor"][2] }, 1);
		}

		settings.Speed = json["Speed"];
		settings.SpeedVariation = json["SpeedVariation"];

		settings.UseAcceleration = json["UseAcceleration"];
		settings.Acceleration.x = json["Acceleration"][0];
		settings.Acceleration.y = json["Acceleration"][1];
		settings.Acceleration.z = json["Acceleration"][2];
		settings.MaxSpeed = json["MaxSpeed"];

		if (json.contains("UseGlobalForceFields"))
			settings.UseGlobalForceFields = json["UseGlobalForceFields"];

		if (json.contains("ForceFields"))
		{
			settings.LocalForceFields.resize(json["ForceFields"].size());

			for (int i = 0; i < json["ForceFields"].size(); ++i)
			{
				const auto& forceJson = json["ForceFields"][i];
				auto& forceField = settings.LocalForceFields[i];

				if (forceJson.contains("ForceFieldType"))
					forceField.ForceFieldType = forceJson["ForceFieldType"];

				forceField.Position.x = forceJson["Position"][0];
				forceField.Position.y = forceJson["Position"][1];
				forceField.Position.z = forceJson["Position"][2];

				if (forceJson.contains("Direction"))
				{
					const auto& dirJson = forceJson["Direction"];

					if (dirJson.is_array())
					{
						if (dirJson.size() >= 3)
						{
							forceField.Direction.x = dirJson[0];
							forceField.Direction.y = dirJson[1];
							forceField.Direction.z = dirJson[2];
						}
					}
				}

				forceField.Range = forceJson["Range"];
				forceField.Force = forceJson["Force"];

				if (forceJson.contains("Lerp"))
					forceField.Lerp = forceJson["Lerp"];

				forceField.LerpType = forceJson["LerpType"];
				forceField.LerpPower = forceJson["LerpPower"];
			}
		}

		if (json.contains("RotationSpeed"))
			settings.RotationSpeed = json["RotationSpeed"];
		if (json.contains("RandomSpawnRotation"))
			settings.RandomSpawnRotation = json["RandomSpawnRotation"];
		settings.Global = json["Global"];
		settings.Looping = json["Looping"];
		if (json.contains("MaxEmitTime"))
			settings.MaxEmitTime = json["MaxEmitTime"];
		settings.ScaledDeltaTime = json["ScaledDeltaTime"];

		if (json.contains("IsFlipbook"))
			settings.IsFlipbook = json["IsFlipbook"];
		if (json.contains("Frames"))
			settings.Frames = json["Frames"];
		if (json.contains("Rows"))
			settings.Rows = json["Rows"];
		if (json.contains("Columns"))
			settings.Columns = json["Columns"];
		if (json.contains("FrameRate"))
			settings.FrameRate = json["FrameRate"];

		if (json.contains("LoopInnerChangeSpeed"))
			settings.LoopInnerChangeSpeed = json["LoopInnerChangeSpeed"];
		if (json.contains("LoopOuterChangeSpeed"))
			settings.LoopOuterChangeSpeed = json["LoopOuterChangeSpeed"];
		if (json.contains("BounceRadiusChange"))
			settings.BounceRadiusChange = json["BounceRadiusChange"];

		if (json.contains("MeshNormalOffset"))
			settings.MeshNormalOffset = json["MeshNormalOffset"];

		if (json.contains("Height"))
		{
			settings.Height = json["Height"];
		}
		else
		{
			settings.Height = settings.Speed;
		}

		settings.InnerRadius = json["InnerRadius"];
		if (json.contains("InnerRadiusChangeSpeed"))
			settings.InnerRadiusChangeSpeed = json["InnerRadiusChangeSpeed"];
		if (json.contains("InnerRadiusMax"))
			settings.InnerRadiusMax = json["InnerRadiusMax"];
		settings.OuterRadius = json["OuterRadius"];
		if (json.contains("OuterRadiusChangeSpeed"))
			settings.OuterRadiusChangeSpeed = json["OuterRadiusChangeSpeed"];
		if (json.contains("OuterRadiusMax"))
			settings.OuterRadiusMax = json["OuterRadiusMax"];
		settings.SpawnOnEdge = json["SpawnOnEdge"];
		settings.AimForEdge = json["AimForEdge"];

		settings.Radius = json["Radius"];
		if (json.contains("RadiusChangeSpeed"))
			settings.RadiusChangeSpeed = json["RadiusChangeSpeed"];
		if (json.contains("RadiusMax"))
			settings.RadiusMax = json["RadiusMax"];
		settings.SpawnOnSurface = json["SpawnOnSurface"];

		if (json.contains("InnerRectangleWidth"))
			settings.InnerRectangleWidth = json["InnerRectangleWidth"];
		if (json.contains("InnerRectangleHeight"))
			settings.InnerRectangleHeight = json["InnerRectangleHeight"];
		if (json.contains("OuterRectangleWidth"))
			settings.OuterRectangleWidth = json["OuterRectangleWidth"];
		if (json.contains("OuterRectangleHeight"))
			settings.OuterRectangleHeight = json["OuterRectangleHeight"];

		LOGINFO("Loaded Particle Emitter json " + aAsset->GetPath().filename().string() + " successfully");
		return true;
	}
}