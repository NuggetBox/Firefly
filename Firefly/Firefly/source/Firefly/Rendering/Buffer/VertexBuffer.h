#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Core/DXHelpers.h"

namespace Firefly
{
	namespace wrl = Microsoft::WRL;

	struct VertexBufferInfo
	{
		void* Data;
		size_t ObjectSize;
		size_t Count;
	};
	class VertexBuffer
	{
	public:
		VertexBuffer(const VertexBufferInfo& aInfo);

		void SetData(const void* aData, size_t aSize, ID3D11DeviceContext* aContext);
		void Bind(ID3D11DeviceContext* aContext);

		static Ref<VertexBuffer> Create(const VertexBufferInfo& aInfo);

		__forceinline UINT GetBufferSize() const { return myBufferSize; }
		__forceinline UINT GetStride() const { return *myStride; }
		__forceinline wrl::ComPtr<ID3D11Buffer> GetBuffer() const { return myBuffer.Get(); }
		
	private:
		wrl::ComPtr<ID3D11Buffer> myBuffer;
		Ref<UINT> myStride;
		UINT myBufferSize = 0;
	};
}