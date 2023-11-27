#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include <Firefly/Rendering/Shader/Shader.h>
namespace Firefly
{

	class StructuredBuffer
	{
	public:
		StructuredBuffer(size_t aSizeOfBuffer, size_t aSizeOfObject, UINT cpuAccessFlag);
		template<typename T>
		void SetData(std::vector<T>& aBuffer, ID3D11DeviceContext* aContext);
		void Bind(uint32_t aBindPoint, ShaderType type, ID3D11DeviceContext* aContext);
		void UnBind(ID3D11DeviceContext* aContext);

		template<typename T>
		static Ref<StructuredBuffer> Create(size_t sizeOfBuffer, UINT cpuAccessFlag = D3D11_CPU_ACCESS_WRITE);
	private:
		uint32_t myBindPoint = 0;
		ShaderType myBoundShaderType = ShaderType::Vertex;
		ComPtr<ID3D11Buffer> myBuffer;
		ComPtr<ID3D11ShaderResourceView> myResource;
	};

	inline StructuredBuffer::StructuredBuffer(size_t aSizeOfBuffer, size_t aSizeOfObject, UINT cpuAccessFlag)
	{
		D3D11_BUFFER_DESC sbDesc;
		sbDesc.ByteWidth = aSizeOfObject * aSizeOfBuffer;
		sbDesc.Usage = D3D11_USAGE_DYNAMIC;
		sbDesc.CPUAccessFlags = cpuAccessFlag;
		sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		sbDesc.StructureByteStride = aSizeOfObject;

		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&sbDesc, nullptr, myBuffer.GetAddressOf()));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = aSizeOfBuffer;

		FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myBuffer.Get(), &srvDesc, myResource.GetAddressOf()));

		
	}
	template<typename T>
	inline void StructuredBuffer::SetData(std::vector<T>& aBuffer, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		FF_DX_ASSERT(aContext->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		const size_t sizeInBytes = sizeof(T) * aBuffer.size();
		memcpy_s(mappedResource.pData, sizeInBytes, aBuffer.data(), sizeInBytes);
		aContext->Unmap(myBuffer.Get(), 0);
	}
	inline void Firefly::StructuredBuffer::Bind(uint32_t aBindPoint, ShaderType type, ID3D11DeviceContext* aContext)
	{
		myBindPoint = aBindPoint;
		myBoundShaderType = type;
		switch (type)
		{
		case Firefly::ShaderType::Vertex:
			aContext->VSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		case Firefly::ShaderType::Pixel:
			aContext->PSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		case Firefly::ShaderType::Geometry:
			aContext->GSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		case Firefly::ShaderType::Hull:
			aContext->HSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		case Firefly::ShaderType::Domain:
			aContext->DSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		case Firefly::ShaderType::Compute:
			aContext->CSSetShaderResources(aBindPoint, 1, myResource.GetAddressOf());
			break;
		default:
			break;
		}
	}
	inline void StructuredBuffer::UnBind(ID3D11DeviceContext* aContext)
	{
		ID3D11ShaderResourceView* dummyView = nullptr;
		switch (myBoundShaderType)
		{
		case Firefly::ShaderType::Vertex:
			aContext->VSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		case Firefly::ShaderType::Pixel:
			aContext->PSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		case Firefly::ShaderType::Geometry:
			aContext->GSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		case Firefly::ShaderType::Hull:
			aContext->HSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		case Firefly::ShaderType::Domain:
			aContext->DSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		case Firefly::ShaderType::Compute:
			aContext->CSSetShaderResources(myBindPoint, 1, &dummyView);
			break;
		default:
			break;
		}
	}
	template<typename T>
	inline Ref<StructuredBuffer> StructuredBuffer::Create(size_t sizeOfBuffer, UINT cpuAccessFlag)
	{
		return CreateRef<StructuredBuffer>(sizeOfBuffer, sizeof(T), cpuAccessFlag);
	}
}