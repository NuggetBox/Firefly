#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Rendering/Shader/Shader.h"

namespace Firefly
{
	class ComputeBuffer
	{
	public:
		ComputeBuffer(uint32_t aCount, uint32_t aStride);

		template<typename T>
		void SetData(const std::vector<T>& aBuffer, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get());

		ComPtr<ID3D11Buffer> GetInput() { return myInputBuffer; }

		template<typename T>
		std::vector<T> GetData(ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) const;

		void Bind(uint32_t aBindPoint, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get());
		void UnBind(ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) const;

		template<typename T>
		static Ref<ComputeBuffer> Create(uint32_t aCount);

	private:
		ComPtr<ID3D11ShaderResourceView> mySRV;
		ComPtr<ID3D11UnorderedAccessView> myUAV;
		ComPtr<ID3D11Buffer> myInputBuffer;
		ComPtr<ID3D11Buffer> myOutputBuffer;
		ComPtr<ID3D11Buffer> myOutputResultBuffer;
		uint32_t myBindPoint = 0;
		uint32_t myCount = 0;
	};

	inline ComputeBuffer::ComputeBuffer(uint32_t aCount, uint32_t aStride)
	{
		D3D11_BUFFER_DESC inputDesc;
		inputDesc.Usage = D3D11_USAGE_DYNAMIC;
		inputDesc.ByteWidth = aCount * aStride;
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		inputDesc.StructureByteStride = aStride;
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&inputDesc, nullptr, myInputBuffer.GetAddressOf()));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = 0;
		srvDesc.BufferEx.NumElements = aCount;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myInputBuffer.Get(), &srvDesc, mySRV.GetAddressOf()));

		D3D11_BUFFER_DESC outputDesc;
		outputDesc.Usage = D3D11_USAGE_DEFAULT;
		outputDesc.ByteWidth = aCount * aStride;
		outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		outputDesc.CPUAccessFlags = 0;
		outputDesc.StructureByteStride = aStride;
		outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&outputDesc, nullptr, myOutputBuffer.GetAddressOf()));

		outputDesc.Usage = D3D11_USAGE_STAGING;
		outputDesc.BindFlags = 0;
		outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&outputDesc, nullptr, myOutputResultBuffer.GetAddressOf()));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = aCount;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateUnorderedAccessView(myOutputBuffer.Get(), &uavDesc, myUAV.GetAddressOf()));

		myCount = aCount;
	}

	template<typename T>
	void ComputeBuffer::SetData(const std::vector<T>& aBuffer, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		FF_DX_ASSERT(aContext->Map(myInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		const size_t sizeInBytes = sizeof(T) * aBuffer.size();
		memcpy_s(mappedResource.pData, sizeInBytes, aBuffer.data(), sizeInBytes);
		aContext->Unmap(myInputBuffer.Get(), 0);
	}

	template<typename T>
	std::vector<T> ComputeBuffer::GetData(ID3D11DeviceContext* aContext) const
	{
		std::vector<T> outData;
		outData.resize(myCount);

		aContext->CopyResource(myOutputResultBuffer.Get(), myOutputBuffer.Get());

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		FF_DX_ASSERT(aContext->Map(myOutputResultBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedResource));
		const T* data = static_cast<const T*>(mappedResource.pData);
		memcpy_s(outData.data(), myCount * sizeof(T), data, myCount * sizeof(T));
		aContext->Unmap(myOutputResultBuffer.Get(), 0);

		return outData;
	}

	inline void ComputeBuffer::Bind(uint32_t aBindPoint, ID3D11DeviceContext* aContext)
	{
		aContext->CSSetShaderResources(aBindPoint, 1, mySRV.GetAddressOf());
		aContext->CSSetUnorderedAccessViews(0, 1, myUAV.GetAddressOf(), nullptr);
		myBindPoint = aBindPoint;
	}

	inline void ComputeBuffer::UnBind(ID3D11DeviceContext* aContext) const
	{
		ID3D11ShaderResourceView* impostorSRV = nullptr;
		aContext->CSSetShaderResources(myBindPoint, 1, &impostorSRV);

		ID3D11UnorderedAccessView* impostorUAV = nullptr;
		aContext->CSSetUnorderedAccessViews(myBindPoint, 1, &impostorUAV, nullptr);
	}

	template<typename T>
	inline Ref<ComputeBuffer> ComputeBuffer::Create(uint32_t aCount)
	{
		return CreateRef<ComputeBuffer>(aCount, sizeof(T));
	}
}