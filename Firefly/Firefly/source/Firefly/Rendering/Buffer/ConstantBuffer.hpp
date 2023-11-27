#pragma once
#include "FFpch.h"
namespace Firefly
{
	template<class T>
	class ConstantBuffer
	{
	public:
		void Create(const std::string& aDebugName = "debug");
		void SetData(const T* aData, ID3D11DeviceContext* aContext);
		void SetName(const std::string& aDebugName);
		ID3D11Buffer* GetBuffer() { return myBuffer; }
		void Bind(uint8_t aSlot, ID3D11DeviceContext* aContext);
	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};

	template<class T>
	void ConstantBuffer<T>::Create(const std::string& aDebugName)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(T);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&bufferDesc, NULL, myBuffer.GetAddressOf()));
		myBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, aDebugName.size() * sizeof(char), aDebugName.c_str());
	}

	template <class T>
	void ConstantBuffer<T>::SetData(const T* aData, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE subResource = {};
		FF_DX_ASSERT(aContext->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
		memcpy(subResource.pData, aData, sizeof(T));
		aContext->Unmap(myBuffer.Get(), 0);
	}

	template<class T>
	inline void ConstantBuffer<T>::SetName(const std::string& aDebugName)
	{
		myBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, aDebugName.size() * sizeof(char), aDebugName.c_str());
	}

	template<class T>
	inline void ConstantBuffer<T>::Bind(uint8_t aSlot, ID3D11DeviceContext* aContext)
	{
		aContext->VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		aContext->PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		aContext->GSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
		aContext->CSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
	}
}