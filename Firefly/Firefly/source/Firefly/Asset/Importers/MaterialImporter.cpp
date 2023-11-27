#include "FFpch.h"
#include "MaterialImporter.h"
#include "nlohmann/json.hpp"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"
#include <Firefly/Rendering/Material/InternalMaterial.h>
using json = nlohmann::json;
namespace Firefly
{
	Ref<MaterialAsset> MaterialImporter::ImportMaterial(const std::filesystem::path& aPath) 
	{
		// Parse the material info from the JSON file
		std::ifstream fin(aPath);
		json data = json::parse(fin);

		InternalMaterial internalMaterial{};
		
		internalMaterial.PipelineID = data["PipelineID"];

		if (data.contains("ShouldBlend"))
		{
			internalMaterial.ShouldBlend = data["ShouldBlend"];
		}

		if (data.contains("IsFPS"))
		{
			internalMaterial.ShouldBeFPS = data["IsFPS"];
		}

		if (data.contains("DepthState"))
		{
			std::string state = (std::string)data["DepthState"];
			if (state == "ReadWrite")
			{
				internalMaterial.DepthMode = DepthStencilState::ReadWrite;
			}
			else if (state == "ReadOnly")
			{
				internalMaterial.DepthMode = DepthStencilState::ReadOnly;
			}
			else if (state == "Off")
			{
				internalMaterial.DepthMode = DepthStencilState::Off;
			}
			else
			{
				LOGWARNING("Depth mode could not be loaded uses default state {}", aPath.string());
			}
		}

		if (data.contains("CullState"))
		{
			std::string state = (std::string)data["CullState"];
			if (state == "Back")
			{
				internalMaterial.CullMode = CullState::Back;
			}
			else if (state == "Front")
			{
				internalMaterial.CullMode = CullState::Front;
			}
			else if (state == "None")
			{
				internalMaterial.CullMode = CullState::None;
			}
			else if (state == "Wireframe")
			{
				internalMaterial.CullMode = CullState::Wireframe;
			}
			else
			{
				LOGWARNING("Cull mode could not be loaded uses default state {}", aPath.string());
			}
		}

		if (data.contains("BlendState"))
		{
			std::string state = (std::string)data["BlendState"];
			if (state == "Opaque")
			{
				internalMaterial.BlendMode = BlendState::Opaque;
			}
			else if (state == "AlphaBlend")
			{
				internalMaterial.BlendMode = BlendState::Translucent;
			}
			else if (state == "Additive")
			{
				internalMaterial.BlendMode = BlendState::Additive;
			}
		}
		
		// this is due to more blendstates were added. We keep the old loading above for backwards compatablity.
		// This is also more scalable than string comparecings.
		if (data.contains("BlendState2"))
		{
			const auto index = (int32_t)data["BlendState2"];

			internalMaterial.BlendMode = static_cast<BlendState>(index);
		}

		// Load the shader and get its bound resources
		Ref<Shader> shader = ShaderLibrary::GetShader(internalMaterial.PipelineID, ShaderType::Pixel);
		if (!shader || !shader->IsValid())
		{
			LOGERROR("Could not Load Material {}!", aPath.stem().string());
			LOGERROR("Setting material to default.");
			return ResourceCache::GetAsset<MaterialAsset>("Default");
		}
		auto& rs = shader->GetBoundResources();

		// Extract the textures and constant buffer from the bound resources
		std::vector<TexturePacket> textures;
		for (auto & i : rs)
		{
			if (i.type == BoundType::Texture)
			{
				auto& r = textures.emplace_back();
				r.BindPoint = i.bindPoint;
				r.VariableName = i.name;
			}
			if (i.type == BoundType::ContantBuffer && i.name == "MaterialInfo")
			{
				internalMaterial.MaterialData.bindpoint = i.bindPoint;
				internalMaterial.MaterialData.varibles = i.varibles;
				internalMaterial.MaterialData.data.resize(i.size);
				size_t byteOffset = 0;
				// Set Default data from the shader.
				for (auto& var : internalMaterial.MaterialData.varibles)
				{
					if (var.DefaultData)
					{
						memcpy(&internalMaterial.MaterialData.data[byteOffset], var.DefaultData, SizeOfReflectedValue(var.VariableType));
					}
					byteOffset += SizeOfReflectedValue(var.VariableType);
				}
			}
		}

		if (data.contains("MaterialData"))
		{

			auto& fileMatData = data["MaterialData"];
			std::vector<uint8_t> materialDataString(fileMatData.size());

			for (size_t i = 0; i < fileMatData.size(); i++)
			{
				materialDataString[i] = fileMatData[i];
			};

			const size_t CpySize = materialDataString.size() > internalMaterial.MaterialData.data.size() ? internalMaterial.MaterialData.data.size() : materialDataString.size();

			memcpy(internalMaterial.MaterialData.data.data(), materialDataString.data(), CpySize);
		}

		const size_t textureSize = textures.size() < data["Textures"].size() ? textures.size() : data["Textures"].size();
		// Set the textures for the material

		for (size_t i = 0; i < textureSize; ++i)
		{
			auto tex = data["Textures"].at(i);
			textures[i].TexturePath = (std::string)tex["TexturePath"];
			textures[i].ShaderStage = StringToShaderStage((std::string)tex["ShaderStage"]);
			textures[i].Texture = ResourceCache::GetAsset<Texture2D>(textures[i].TexturePath);
		}
		internalMaterial.Textures = textures;

		// Create the material and initialize it with the extracted material info
		Ref<MaterialAsset> mat = CreateRef<MaterialAsset>();
		mat->Init(internalMaterial);
		std::string str = aPath.stem().string();
		mat->AssignID(std::hash<std::string>{}(str));
		return mat;
	}
	bool MaterialImporter::ImportMaterial(Ref<MaterialAsset> aMat)
	{
		*aMat = *ImportMaterial(aMat->GetPath());
		return true;
	}
	void MaterialImporter::ExportMaterial(Ref<MaterialAsset> aMaterial, const std::filesystem::path& aPath)
	{
		json js;
		js["PipelineID"] = aMaterial->GetInfo().PipelineID;
		js["ShouldBlend"] = aMaterial->GetInfo().ShouldBlend;
		js["IsFPS"] = aMaterial->GetInfo().ShouldBeFPS;

		switch (aMaterial->GetInfo().DepthMode)
		{
		case DepthStencilState::ReadWrite:
			js["DepthState"] = "ReadWrite";
			break;
		case DepthStencilState::ReadOnly:
			js["DepthState"] = "ReadOnly";
			break;
		case DepthStencilState::Off:
			js["DepthState"] = "Off";
			break;
		default:
			break;
		}

		switch (aMaterial->GetInfo().CullMode)
		{
		case CullState::Back:
			js["CullState"] = "Back";
			break;
		case CullState::Front:
			js["CullState"] = "Front";
			break;
		case CullState::None:
			js["CullState"] = "None";
			break;
		case CullState::Wireframe:
			js["CullState"] = "Wireframe";
			break;
		default:
			break;
		}

		switch (aMaterial->GetInfo().BlendMode)
		{
		case BlendState::Opaque:
			js["BlendState"] = "Opaque";
			break;
		case BlendState::Translucent:
			js["BlendState"] = "AlphaBlend";
			break;
		case BlendState::Additive:
			js["BlendState"] = "Additive";
			break;
		default:
			break;
		}
		js["BlendState2"] = static_cast<int32_t>(aMaterial->GetInfo().BlendMode);

		for (size_t i = 0; i < aMaterial->GetInfo().Textures.size(); ++i)
		{
			auto& tx = aMaterial->GetInfo().Textures[i];
			js["Textures"][i]["BindPoint"] = tx.BindPoint;
			js["Textures"][i]["ShaderStage"] = ShaderStageToString(tx.ShaderStage);
			js["Textures"][i]["TexturePath"] = tx.TexturePath.string();
		}

		
		js["MaterialData"] = aMaterial->GetInfo().MaterialData.data;
		std::ofstream fout(aPath);
		fout << js;
	}

	void MaterialImporter::RefreshMaterial(Ref<MaterialAsset> aMaterial, const std::filesystem::path& aPath)
	{
		std::ifstream fin(aPath);
		json data = json::parse(fin);
		InternalMaterial info{};
		info.PipelineID = aMaterial->GetInfo().PipelineID;
		Ref<Shader> shader = ShaderLibrary::GetShader(info.PipelineID, ShaderType::Pixel);
		if (!shader) return;
		auto& rs = shader->GetBoundResources();
		info.BlendMode = aMaterial->GetInfo().BlendMode;
		info.CullMode = aMaterial->GetInfo().CullMode;
		info.DepthMode = aMaterial->GetInfo().DepthMode;
		info.ShouldBeFPS = aMaterial->GetInfo().ShouldBeFPS;
		info.MaterialData.data = aMaterial->GetInfo().MaterialData.data;
		std::vector<TexturePacket> resource;
		for (auto& i : rs)
		{
			if (i.type == BoundType::Texture)
			{
				auto& r = resource.emplace_back();
				r.BindPoint = i.bindPoint;
				r.ShaderStage = ShaderType::Pixel;
				r.VariableName = i.name;

			}
			if (i.type == BoundType::ContantBuffer)
			{
				if (i.name == "MaterialInfo")
				{
					info.MaterialData.bindpoint = i.bindPoint;
					info.MaterialData.varibles = i.varibles;
					info.MaterialData.data.resize(i.size);
				}
			}
		}

		info.ShouldBlend = aMaterial->GetInfo().ShouldBlend;
		for (size_t i = 0; i < resource.size(); ++i)
		{
			if (aMaterial->GetInfo().Textures.size() <= i)
			{
				aMaterial->GetInfo().Textures.emplace_back();
			}
			resource[i].TexturePath = aMaterial->GetInfo().Textures[i].TexturePath;
			resource[i].Texture = ResourceCache::GetAsset<Texture2D>(resource[i].TexturePath, true);
		}

		info.Textures = resource;
		aMaterial->Init(info);
	}
}