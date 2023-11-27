#include "FFpch.h"
#include "ShaderImporter.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"
#include "nlohmann/json.hpp"
namespace Firefly
{
	void ShaderImporter::Initialize()
	{
		{
			ShaderAssetInfo info{};
			info.shaderKey = "DeferredPBR";
			info.shaderKeyAnimation = "AnimatedDeferredPBR";
			info.pixelShaderPath = "Editor/Shaders/Engine/DeferredPBR_ps.hlsl";
			ShaderMap["DeferredPBR"] = info;
		}
		{
			ShaderAssetInfo info{};
			info.shaderKey = "ForwardPBR";
			info.shaderKeyAnimation = "AnimatedForwardPBR";
			info.pixelShaderPath = "Editor/Shaders/Engine/ForwardPBR_ps.hlsl";
			ShaderMap["ForwardPBR"] = info;
		} 
		{
			ShaderAssetInfo info{};
			info.shaderKey = "Tiling";
			info.shaderKeyAnimation = "TilingAnimation";
			info.pixelShaderPath = "Editor/Shaders/Engine/TiledDeferredPBR_ps.hlsl";
			ShaderMap["Tiling"] = info;
		}
		LoadCustomShaders("assets/");
	}
	ShaderAssetInfo ShaderImporter::LoadDataFromFile(std::filesystem::path aPath)
	{
		std::ifstream fin(aPath);
		nlohmann::json data = nlohmann::json::parse(fin);
		ShaderAssetInfo info;
		info.shaderKey = data["ShaderKey"];
		if (data.contains("ShaderKeyAnimation"))
		{
			info.shaderKeyAnimation = data["ShaderKeyAnimation"];
		}
		else
		{
			info.shaderKeyAnimation = info.shaderKey + "Animation";
		}
		info.pixelShaderPath = (std::string)data["PSpath"];
		ShaderMap[aPath.stem().string()] = info;
		return info;
	}
	std::vector<std::string> ShaderImporter::GetKeys()
	{
		std::vector<std::string> keys;
		for (auto& key : ShaderMap)
		{
			keys.push_back(key.first);
		}
		return keys;
	}
	void ShaderImporter::LoadCustomShaders(const std::filesystem::path& aPath)
	{
		for (auto currDirectory : std::filesystem::directory_iterator(aPath))
		{
			if (currDirectory.is_directory())
			{
				LoadCustomShaders(currDirectory);
			}
			else
			{
				auto file = currDirectory.path();
				if (file.extension() == ".ffs")
				{
					ShaderImporter::RegistryKeys(ShaderImporter::LoadDataFromFile(file));
					continue;
				}
			}
		}
	}
	void ShaderImporter::UpdateFileFromData(const std::filesystem::path& aPath, const ShaderAssetInfo& aInfo)
	{
		nlohmann::json js;
		js["ShaderKey"] = aInfo.shaderKey;
		js["ShaderKeyAnimation"] = aInfo.shaderKeyAnimation;
		js["PSpath"] = aInfo.pixelShaderPath;
		std::ofstream fout(aPath);
		fout << js;
		ShaderMap[aPath.stem().string()] = aInfo;
	}
	void ShaderImporter::RegistryKeys(ShaderAssetInfo info)
	{
		ShaderLibrary::Add(info.shaderKey, {
			Shader::Create("Editor/Shaders/Engine/StaticMesh_vs.hlsl", ShaderType::Vertex),
			Shader::Create(info.pixelShaderPath, ShaderType::Pixel),
			});
		ShaderLibrary::Add(info.shaderKeyAnimation, {
			Shader::Create("Editor/Shaders/Engine/AnimatedMesh_vs.hlsl", ShaderType::Vertex),
			Shader::Create(info.pixelShaderPath, ShaderType::Pixel),
			});
	}
	void ShaderImporter::RemoveKeys(ShaderAssetInfo info)
	{
		ShaderLibrary::Remove(info.shaderKey);
		ShaderLibrary::Remove(info.shaderKeyAnimation);
	}
}