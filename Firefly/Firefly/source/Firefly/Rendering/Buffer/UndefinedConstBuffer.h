#pragma once
#include <d3d11.h>

#include "Firefly/Rendering/GraphicsContext.h"
namespace Firefly
{
	class UndefinedConstBuffer
	{
	public:
		void Create();
		void SetData(const void* data, size_t size, ID3D11DeviceContext* aContext);
		ID3D11Buffer* GetBuffer() { return myBuffer.Get(); }
		void Bind(uint8_t slot, ShaderType aShaderType = ShaderType::Vertex | ShaderType::Geometry | ShaderType::Pixel, ID3D11DeviceContext* aContext = nullptr);
	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};

	inline void UndefinedConstBuffer::Create()
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = 256;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&bufferDesc, NULL, myBuffer.GetAddressOf()));
	}

	inline void UndefinedConstBuffer::SetData(const void* data, size_t size, ID3D11DeviceContext* aContext)
	{
		if (size == 0)
		{
			return;
		}
		D3D11_MAPPED_SUBRESOURCE subResource = {};
		FF_DX_ASSERT(aContext->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
		memcpy(subResource.pData, data, size * sizeof(BYTE));
		aContext->Unmap(myBuffer.Get(), 0);
	}

	inline void UndefinedConstBuffer::Bind(uint8_t slot, ShaderType aShaderType, ID3D11DeviceContext* aContext)
	{
		if (aShaderType & ShaderType::Vertex)
		{
			aContext->VSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
		if (aShaderType & ShaderType::Geometry)
		{
			aContext->GSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
		if (aShaderType & ShaderType::Pixel)
		{
			aContext->PSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
		if (aShaderType & ShaderType::Compute)
		{
			aContext->CSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
		if (aShaderType & ShaderType::Hull)
		{
			aContext->HSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
		if (aShaderType & ShaderType::Domain)
		{
			aContext->DSSetConstantBuffers(slot, 1, myBuffer.GetAddressOf());
		}
	}
}