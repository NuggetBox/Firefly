#pragma once
#include "Utils/Math/Vector.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Core/DXHelpers.h"
#include <set>

#include "GraphicsContext.h"
#include "Shader/Shader.h"

namespace Firefly
{
	struct FramebufferSpecs
	{
		uint32_t Width, Height;
		uint32_t Depth = 1;
		bool Is2DArray = false;
		bool Is3D = false;
		bool IsCube = false;
		uint32_t Samples = 1;
		bool ShouldResize = true;
		uint32_t Mips = 1;
		uint32_t DepthMips = 1;
		std::vector<Firefly::ImageFormat> Formats;
	};
	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecs& aSpecs);
	
		bool Resize(const Utils::Vector2<uint32_t>& aSize, bool aForceResize = false);
		
		void Bind(ID3D11DeviceContext* aContext);
		void BindSpecificRenderTargets(std::vector<uint32_t> aIndex, ID3D11DeviceContext* aContext);
		void BindWithDifferentDepth(Ref<Framebuffer> aDepthBuffer, ID3D11DeviceContext* aContext);
		void BindSRV(uint32_t aSlot, uint32_t aResourceView, ShaderType aShaderType, ID3D11DeviceContext* aContext);
		void UnBindShaderResource(uint32_t aSlot, ShaderType aShaderType, ID3D11DeviceContext* aContext);
		void BindDepthSRV(uint32_t aSlot, ShaderType aShaderType, ID3D11DeviceContext* aContext);
		void TransferDepth(Ref<Framebuffer> aFB);
		void UnBind(ID3D11DeviceContext* aContext);

		template<typename T>
		T ReadPixel(uint32_t aIndex, uint32_t aX, uint32_t aY);

		template<typename T>
		[[deprecated]] std::set<T> ReadPixels(uint32_t index, uint32_t leftX, uint32_t leftY, uint32_t rightX, uint32_t rightY);
		
		void Clear(const Vector4f& color = { 0.1f, 0.1f,0.1f, 1.f }, ID3D11DeviceContext* aContext = nullptr);
		
		ComPtr<ID3D11ShaderResourceView> GetColorAttachment(uint32_t index) { return myShaderResourceViews[index].Get(); };
		ComPtr<ID3D11RenderTargetView> GetRenderTarget(uint32_t index) { return myRenderTargetViews[index].Get(); };
		ComPtr<ID3D11Texture2D> GetTextures2D(uint32_t index) { return (ID3D11Texture2D*)myColorBuffers[index].Get(); };
		ComPtr<ID3D11Texture3D> GetTextures3D(uint32_t index) { return (ID3D11Texture3D*)myColorBuffers[index].Get(); };
		ComPtr<ID3D11ShaderResourceView> GetDepthAttachment() { return myDepthShaderResource; }
		inline FramebufferSpecs& GetSpecs() { return myData; }

		static void FlushResizes();
		static Ref<Framebuffer> Create(const FramebufferSpecs& specs); 
	private:
		void Validate();

		std::vector<ComPtr<ID3D11Resource>> myColorBuffers;
		std::vector<ComPtr<ID3D11RenderTargetView>> myRenderTargetViews;
		std::vector<ComPtr<ID3D11ShaderResourceView>> myShaderResourceViews;
		ComPtr<ID3D11DepthStencilView> myDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> myDepthShaderResource;
		ComPtr<ID3D11Resource> myDepthStencilBuffer;
		D3D11_VIEWPORT myViewport = {};
		FramebufferSpecs myData;
		std::mutex myValidationLock; // Only used for the multi threaded rendering inorder to not try to use the framebuffer when it is being rebuilt.

		// this is a sloppy way to do it but it will work for now.
		inline static std::mutex myQueueLock;
		inline static std::vector<std::function<void()>> myStaticResizeQueue;
	};

	template<typename T>
	inline T Framebuffer::ReadPixel(uint32_t aIndex, uint32_t aX, uint32_t aY)
	{
		if (aX < 0 || aY < 0 || aX > myData.Width || aY > myData.Height)
		{
			return 0;
		}

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = myData.Width;
		desc.Height = myData.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = FormatToDXFormat(myData.Formats[aIndex]);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		ID3D11Texture2D* texture;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, &texture));

		auto context = GraphicsContext::Context();
		context->CopyResource(texture, myColorBuffers[aIndex].Get());

		D3D11_MAPPED_SUBRESOURCE subresource = {};

		context->Map(texture, 0, D3D11_MAP_READ, 0, &subresource);

		T* data = reinterpret_cast<T*>(subresource.pData);

		uint32_t loc = (aX + static_cast<unsigned long long>(aY) * subresource.RowPitch / sizeof(T));
		T value = data[loc];

		context->Unmap(texture, 0);

		texture->Release();
		texture = nullptr;

		return value;
	}

	template<typename T>
	inline std::set<T> Framebuffer::ReadPixels(uint32_t index, uint32_t leftX, uint32_t leftY, uint32_t rightX, uint32_t rightY)
	{
		assert(leftX <= myData.Width);
		assert(leftY <= myData.Height);
		assert(rightX <= myData.Width);
		assert(rightY <= myData.Height);

		if (leftX > rightX)
		{
			Utils::Swap(leftX, rightX);
		}

		if (leftY > rightY)
		{
			Utils::Swap(leftY, rightY);
		}

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = ((uint32_t)myData.Width);
		desc.Height = ((uint32_t)myData.Height);
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = FormatToDXFormat(myData.Formats[index]);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		ID3D11Texture2D* texture;
		FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, &texture));

		auto context = GraphicsContext::Context();
		context->CopyResource(texture, myColorBuffers[index].Get());

		D3D11_MAPPED_SUBRESOURCE subresource = {};

		context->Map(texture, 0, D3D11_MAP_READ, 0, &subresource);

		T* data = reinterpret_cast<T*>(subresource.pData);

		std::set<T> setOfEntities;

		for (int i = leftX; i < rightX; ++i)
		{
			for (int j = leftY; j < rightY; ++j)
			{
				uint32_t loc = (i + static_cast<unsigned long long>(j) * subresource.RowPitch / sizeof(T));
				setOfEntities.insert(data[loc]);
			}
		}

		context->Unmap(texture, 0);

		texture->Release();
		texture = nullptr;

		return setOfEntities;
	}
}
