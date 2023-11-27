#include "FFpch.h"
#include "VertexBuffer.h"

#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/GraphicsContext.h"

namespace Firefly
{
	VertexBuffer::VertexBuffer(const VertexBufferInfo& aInfo)
	{
		myBufferSize = aInfo.ObjectSize * aInfo.Count;
		myStride = CreateRef<uint32_t>(aInfo.ObjectSize);

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = aInfo.ObjectSize * aInfo.Count;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexBufferData.pSysMem = aInfo.Data;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, myBuffer.GetAddressOf()));
	}
	void VertexBuffer::SetData(const void* data, size_t size, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE subResource = {};
		FF_DX_ASSERT(aContext->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
		memcpy_s(subResource.pData, size, data, size);
		aContext->Unmap(myBuffer.Get(), 0);
	}

	void VertexBuffer::Bind(ID3D11DeviceContext* aContext)
	{
		UINT offset = 0;
		aContext->IASetVertexBuffers(0, 1, myBuffer.GetAddressOf(), myStride.get(), &offset);
	}

	Ref<VertexBuffer> VertexBuffer::Create(const VertexBufferInfo& aInfo)
	{
		return CreateRef<VertexBuffer>(aInfo);
	}
}
