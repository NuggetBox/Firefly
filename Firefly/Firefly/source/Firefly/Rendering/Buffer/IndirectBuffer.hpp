#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/GraphicsContext.h"

namespace Firefly
{
	class IndirectBuffer
	{
	public:
		IndirectBuffer(uint32_t aCount, uint32_t aStride);

		template<typename T>
		void SetData(const std::vector<T>& aBuffer, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get());

		ComPtr<ID3D11Buffer> GetBuffer() { return myInputBuffer; }

	

		void Bind(uint32_t aBindPoint, ID3D11DeviceContext* aContext = GraphicsContext::Context().Get());
		void UnBind(ID3D11DeviceContext* aContext = GraphicsContext::Context().Get()) const;

		template<typename T>
		static Ref<IndirectBuffer> Create(uint32_t aCount);

	private:
		ComPtr<ID3D11ShaderResourceView> mySRV;
		ComPtr<ID3D11UnorderedAccessView> myUAV;
		ComPtr<ID3D11Buffer> myInputBuffer;
		ComPtr<ID3D11Buffer> myStagingBuffer;
		uint32_t myBindPoint = 0;
		uint32_t myCount = 0;
	};

	inline IndirectBuffer::IndirectBuffer(uint32_t aCount, uint32_t aStride)
	{
		D3D11_BUFFER_DESC inputDesc{};
		inputDesc.Usage = D3D11_USAGE_DEFAULT;
		inputDesc.ByteWidth = aCount * aStride;
		inputDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		inputDesc.CPUAccessFlags = 0;
		inputDesc.StructureByteStride = aStride;
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&inputDesc, nullptr, myInputBuffer.GetAddressOf()));

		/*D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = 0;
		srvDesc.BufferEx.NumElements = aCount;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myInputBuffer.Get(), &srvDesc, mySRV.GetAddressOf()));*/
		inputDesc.Usage = D3D11_USAGE_DYNAMIC;
		inputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&inputDesc, nullptr, myStagingBuffer.GetAddressOf()));

	
		myCount = aCount;
	}

	template<typename T>
	void IndirectBuffer::SetData(const std::vector<T>& aBuffer, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		FF_DX_ASSERT(aContext->Map(myStagingBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		const size_t sizeInBytes = sizeof(T) * aBuffer.size();
		memcpy_s(mappedResource.pData, sizeInBytes, aBuffer.data(), sizeInBytes);
		aContext->Unmap(myStagingBuffer.Get(), 0);

		aContext->CopyResource(myInputBuffer.Get(), myStagingBuffer.Get());
	}

	inline void IndirectBuffer::Bind(uint32_t aBindPoint, ID3D11DeviceContext* aContext)
	{
		aContext->CSSetShaderResources(aBindPoint, 1, mySRV.GetAddressOf());
		aContext->CSSetUnorderedAccessViews(0, 1, myUAV.GetAddressOf(), nullptr);
		myBindPoint = aBindPoint;
	}

	inline void IndirectBuffer::UnBind(ID3D11DeviceContext* aContext) const
	{
		ID3D11ShaderResourceView* impostorSRV = nullptr;
		aContext->CSSetShaderResources(myBindPoint, 1, &impostorSRV);

		ID3D11UnorderedAccessView* impostorUAV = nullptr;
		aContext->CSSetUnorderedAccessViews(myBindPoint, 1, &impostorUAV, nullptr);
	}

	template<typename T>
	inline Ref<IndirectBuffer> IndirectBuffer::Create(uint32_t aCount)
	{
		return CreateRef<IndirectBuffer>(aCount, sizeof(T));
	}
}