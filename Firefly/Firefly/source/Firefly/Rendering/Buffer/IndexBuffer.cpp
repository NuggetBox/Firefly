#include "FFpch.h"
#include "IndexBuffer.h"

#include "Firefly/Rendering/GraphicsContext.h"

namespace Firefly
{
	IndexBuffer::IndexBuffer(uint32_t* aData, size_t aNumberOfIndices)
	{
		myBufferSize = aNumberOfIndices ;

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = myBufferSize * sizeof(uint32_t);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData = {};
		iinitData.pSysMem = aData;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateBuffer(&indexBufferDesc, &iinitData, &myIndexsBuffer));
	}
	void IndexBuffer::Bind(ID3D11DeviceContext* aContext)
	{
		aContext->IASetIndexBuffer(myIndexsBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	}
	void IndexBuffer::Bind(size_t aOffset, ID3D11DeviceContext* aContext)
	{
		aContext->IASetIndexBuffer(myIndexsBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, aOffset);
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* aData, size_t aNumberOfIndices)
	{
		return CreateRef<IndexBuffer>(aData, aNumberOfIndices);
	}
}
