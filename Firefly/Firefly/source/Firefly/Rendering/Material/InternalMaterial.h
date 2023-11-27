#pragma once
#include "Firefly/Core/DXHelpers.h"

#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Rendering/Pipeline/GraphicsPipeline.h"

namespace Firefly
{
	constexpr size_t DefaultDeferredPipeline = 16382186296439120301;
	constexpr size_t DefaultForwardPipeline = 16665830735797483037;
	constexpr size_t DefaultParticlePipeline = 6955484806259956861;

	struct PerMeshMaterialData
	{
		Utils::Vec4 CustomColor = Utils::Vec4(1,1,1,1);
		float Custom0 = 1.f;
		float Custom1 = 1.f;
		float Custom2 = 1.f;
		float Custom3 = 1.f;
	};
	struct TexturePacket
	{
		TexturePacket() {};
		TexturePacket(const TexturePacket& other)
		{
			VariableName = other.VariableName;
			BindPoint = other.BindPoint;
			ShaderStage = other.ShaderStage;
			Texture = other.Texture;
			TexturePath = other.TexturePath;
		}


		std::string VariableName;
		uint32_t BindPoint{};
		ShaderType ShaderStage;
		Ref<Texture2D> Texture;
		std::filesystem::path TexturePath;
		bool operator==(TexturePacket& other)
		{
			return TexturePath.stem().string() == other.TexturePath.stem().string();
		}
		bool operator<(TexturePacket& other)
		{
			std::string str = TexturePath.stem().string();
			auto id = std::hash<std::string>{}(str);
			std::string cmpStr = other.TexturePath.stem().string();
			auto otherId = std::hash<std::string>{}(cmpStr);
			return (id < otherId);
		}
		bool operator!=(TexturePacket& other)
		{
			return !operator==(other);
		}
	};

	struct Materialbuffer
	{
		Materialbuffer() {};
		Materialbuffer(const Materialbuffer& other)
		{
			bindpoint = other.bindpoint;

			varibles.resize(other.varibles.size());
			for (size_t i = 0; auto& copyVars : other.varibles)
			{
				varibles[i].DefaultData = (copyVars.DefaultData);
				varibles[i].Name = copyVars.Name;
				varibles[i].Type = copyVars.Type;
				varibles[i].VariableType = copyVars.VariableType;
			}
			data = other.data;
		}

		uint32_t bindpoint{};
		std::vector<ReflectedValue> varibles; 
		std::vector<uint8_t> data;
	};

	struct InternalMaterial
	{
		InternalMaterial() {};
		InternalMaterial(const InternalMaterial& other)
		{
			PipelineID = other.PipelineID;
			BlendMode = other.BlendMode;
			CullMode = other.CullMode;
			DepthMode = other.DepthMode;
			Name = other.Name;
			Textures = other.Textures;
			MaterialData = other.MaterialData;
			ShouldBlend = other.ShouldBlend;
			ShouldBeFPS = other.ShouldBeFPS;
			Pipeline = other.Pipeline;
		}

		size_t PipelineID = DefaultDeferredPipeline;
		BlendState BlendMode = BlendState::Opaque;
		CullState CullMode = CullState::Back;
		DepthStencilState DepthMode = DepthStencilState::ReadWrite;
		std::string Name;
		std::vector<TexturePacket> Textures;
		Materialbuffer MaterialData;
		bool ShouldBlend = false;
		bool ShouldBeFPS = false; // Will be hardcoded for this game.
		GraphicsPipeline Pipeline;
	};
}
