#pragma once
#include "Texture.h"
#include <wrl.h>
namespace Firefly
{
	namespace wrl = Microsoft::WRL;
	class Texture3D : public Texture
	{
	public:
		Texture3D(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth, ImageFormat aFormat, uint32_t levels);

		void Resize(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth);

		void Bind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext /* = GraphicsContext::Context().Get() */) override;
		void UnBind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext /* = GraphicsContext::Context().Get() */) override;

		void BindUAV(uint32_t slot, ID3D11DeviceContext* aContext);
		void UnbindUAV(uint32_t slot, ID3D11DeviceContext* aContext);

		Utils::Vector3<uint32_t> GetDimensions() { return {myWidth, myHeight, myDepth}; }

		static Ref<Texture3D> Create(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth, ImageFormat aFormat, uint32_t levels);

	private:

		void Validate();

		wrl::ComPtr<ID3D11Texture3D> myTexture;
		wrl::ComPtr<ID3D11UnorderedAccessView> myUAV;
		wrl::ComPtr<ID3D11ShaderResourceView> mySRV;

		uint32_t myHeight = 0;
		uint32_t myWidth = 0;
		uint32_t myDepth = 0;
		uint32_t myLevels = 0;
		ImageFormat myImageFormat = ImageFormat::R32F;
	};
}
