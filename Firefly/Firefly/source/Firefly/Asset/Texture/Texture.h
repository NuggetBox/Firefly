#pragma once
#include <cstdint>
#include "Firefly/Rendering/Shader/Shader.h"
#include "Firefly/Asset/Asset.h"
#include "Firefly/Rendering/GraphicsContext.h"
namespace Firefly
{
	class Texture : public Asset
	{
	public:
		virtual ~Texture() override = default;

		virtual void Bind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) = 0;
		virtual void UnBind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) = 0;

		static AssetType GetStaticType() { return AssetType::Texture; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

	};
}
