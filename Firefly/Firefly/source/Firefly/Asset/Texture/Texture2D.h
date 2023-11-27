#pragma once
#include "Texture.h"
#include <wrl.h>

namespace Firefly
{
	namespace wrl = Microsoft::WRL;

	class Texture2D : public Texture
	{
	public:
		Texture2D() = default;
		~Texture2D() final;

		bool operator==(Texture2D& other);
		Texture2D(uint32_t width, uint32_t height, ImageFormat format, uint32_t levels, void* data, bool aIsFloat = true, bool aCreateUAV = false);
		Texture2D(const unsigned char* data, uint32_t size);
		bool Init(const std::filesystem::path& aPath, const bool& isSRGB = true);
		int GetHeight() { return m_Height; }
		int GetWidth() { return m_Width; }
		void Bind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) override;
		void UnBind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) override;
		void BindUAV(uint32_t slot, ID3D11DeviceContext* aContext);
		void UnbindUAV(uint32_t slot, ID3D11DeviceContext* aContext);
		void* Pixels() { return myTexturePixels.data(); }
		wrl::ComPtr<ID3D11ShaderResourceView> GetSRV() { return myShaderResourceView; }
		wrl::ComPtr<ID3D11UnorderedAccessView> GetUAV() { return myUAV; }
		
		static bool IsPOW2(const std::filesystem::path& aPath);
		// Functions are only for creating textures from code. This is for easier readablity when creating the Ref.
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, ImageFormat format, uint32_t levels, void* data, bool aIsFloat = true, bool aCreateUAV = false);
		static Ref<Texture2D> Create(const unsigned char* data, uint32_t size);
	private:
		wrl::ComPtr<ID3D11Texture2D> myTexture;
		wrl::ComPtr<ID3D11UnorderedAccessView> myUAV;
		wrl::ComPtr<ID3D11ShaderResourceView> myShaderResourceView;
		std::vector<uint8_t> myTexturePixels;
		int m_Height = 0;
		int m_Width = 0;
	};
}