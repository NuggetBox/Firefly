#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Core/DXHelpers.h"

namespace Firefly
{
	namespace wrl = Microsoft::WRL;

	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t* aData, size_t aNumberOfIndices);
		void Bind(ID3D11DeviceContext* aContext);
		void Bind(size_t aOffset, ID3D11DeviceContext* aContext);
		uint32_t GetBufferSize() { return myBufferSize; }
		static Ref<IndexBuffer> Create(uint32_t* aData, size_t aNumberOfIndices);

		wrl::ComPtr<ID3D11Buffer> GetBuffer() const { return myIndexsBuffer; }
		
	private:
		wrl::ComPtr<ID3D11Buffer> myIndexsBuffer;
		uint32_t myBufferSize = 0;
	};
}