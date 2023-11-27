#include "FFpch.h"
#include "Framebuffer.h"

#include "GraphicsContext.h"

namespace Firefly
{
	DXGI_FORMAT GetResourceDepthFormat(ImageFormat format)
	{
		switch (format)
		{

		case Firefly::ImageFormat::Depth32: return DXGI_FORMAT_D32_FLOAT;

		case Firefly::ImageFormat::Depth24: return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case Firefly::ImageFormat::Depth16: return DXGI_FORMAT_D16_UNORM;
		default:
			LOGERROR("Format chosen in as depth is not a depth format!");
			FF_ASSERT(false);
			break;
		}
		return DXGI_FORMAT::DXGI_FORMAT_420_OPAQUE;
	}

	DXGI_FORMAT GetResourceFormat(ImageFormat format)
	{
		switch (format)
		{

		case Firefly::ImageFormat::Depth32: return DXGI_FORMAT_R32_FLOAT;
			break;
		case Firefly::ImageFormat::Depth24: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case Firefly::ImageFormat::Depth16: return DXGI_FORMAT_R16_UNORM;
			break;
		default:
			LOGERROR("Format chosen in as depth is not a depth format!");
			FF_ASSERT(false);
			break;
		}
		return DXGI_FORMAT::DXGI_FORMAT_420_OPAQUE;
	}


	Framebuffer::Framebuffer(const FramebufferSpecs& aSpecs) : myData(aSpecs)
	{
		myDepthStencilBuffer = nullptr;
		myDepthStencilView = nullptr;
		myRenderTargetViews.resize(aSpecs.Formats.size());
		myShaderResourceViews.resize(aSpecs.Formats.size());
		Validate();
	}

	bool Framebuffer::Resize(const Utils::Vector2<uint32_t>& aSize, bool aForceResize)
	{
		//if (myData.Is3D) return;

		if (myData.Width == aSize.x && myData.Height == aSize.y)
		{
			return false;
		}
		if (aSize.x <= 0 || aSize.y <= 0)
		{
			return false;
		}
		myData.Width = aSize.x;
		myData.Height = aSize.y;
		if (aForceResize)
		{
			if (aSize.x > 0 || aSize.y > 0)
			{
				Validate();
			}
		}
		else
		{
			myQueueLock.lock();
			myStaticResizeQueue.emplace_back([&, aSize]()
				{
					if (aSize.x > 0 || aSize.y > 0)
					{
						Validate();
					}
				});
			myQueueLock.unlock();
		}
		return true;
	}

	void Framebuffer::Bind(ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		aContext->OMSetRenderTargets(myRenderTargetViews.size(), myRenderTargetViews.data()->GetAddressOf(), myDepthStencilView.Get());
		aContext->RSSetViewports(1, &myViewport);
	}

	void Framebuffer::BindSpecificRenderTargets(std::vector<uint32_t> aIndex, ID3D11DeviceContext* aContext)
	{
		std::vector<ID3D11RenderTargetView*> targets(aIndex.size());

		for (size_t i = 0; auto& target : targets)
		{
			target = myRenderTargetViews[aIndex[i]].Get();
			i++;
		}

		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		aContext->OMSetRenderTargets(targets.size(), targets.data(), nullptr);
		aContext->RSSetViewports(1, &myViewport);
	}

	void Framebuffer::BindWithDifferentDepth(Ref<Framebuffer> aDepthBuffer, ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		aContext->OMSetRenderTargets(myRenderTargetViews.size(), myRenderTargetViews.data()->GetAddressOf(), aDepthBuffer->myDepthStencilView.Get());
		aContext->RSSetViewports(1, &myViewport);
	}

	void Framebuffer::BindSRV(uint32_t aSlot, uint32_t aResourceView, ShaderType aShaderType, ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.

		if (aShaderType & ShaderType::Vertex)
		{
			aContext->VSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}
		if (aShaderType & ShaderType::Geometry)
		{
			aContext->GSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}
		if (aShaderType & ShaderType::Pixel)
		{
			aContext->PSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}
		if (aShaderType & ShaderType::Compute)
		{
			aContext->CSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}
		if (aShaderType & ShaderType::Hull)
		{
			aContext->HSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}
		if (aShaderType & ShaderType::Domain)
		{
			aContext->DSSetShaderResources(aSlot, 1, myShaderResourceViews[aResourceView].GetAddressOf());
		}

	}

	void Framebuffer::UnBindShaderResource(uint32_t aSlot, ShaderType aShaderType, ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		ID3D11ShaderResourceView* dummySRV = nullptr;
		if (aShaderType & ShaderType::Vertex)
		{
			aContext->VSSetShaderResources(aSlot, 1, &dummySRV);
		}
		if (aShaderType & ShaderType::Geometry)
		{
			aContext->GSSetShaderResources(aSlot, 1, &dummySRV);
		}
		if (aShaderType & ShaderType::Pixel)
		{
			aContext->PSSetShaderResources(aSlot, 1, &dummySRV);
		}
		if (aShaderType & ShaderType::Compute)
		{
			aContext->CSSetShaderResources(aSlot, 1, &dummySRV);
		}
		if (aShaderType & ShaderType::Hull)
		{
			aContext->HSSetShaderResources(aSlot, 1, &dummySRV);
		}
		if (aShaderType & ShaderType::Domain)
		{
			aContext->DSSetShaderResources(aSlot, 1, &dummySRV);
		}
	}

	void Framebuffer::BindDepthSRV(uint32_t aSlot, ShaderType aShaderType, ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		if (aShaderType & ShaderType::Vertex)
		{
			aContext->VSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
		if (aShaderType & ShaderType::Geometry)
		{
			aContext->GSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
		if (aShaderType & ShaderType::Pixel)
		{
			aContext->PSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
		if (aShaderType & ShaderType::Compute)
		{
			aContext->CSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
		if (aShaderType & ShaderType::Hull)
		{
			aContext->HSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
		if (aShaderType & ShaderType::Domain)
		{
			aContext->DSSetShaderResources(aSlot, 1, myDepthShaderResource.GetAddressOf());
		}
	}

	void Framebuffer::TransferDepth(Ref<Framebuffer> aFB)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		myDepthShaderResource = aFB->myDepthShaderResource.Get();
		myDepthStencilBuffer = aFB->myDepthStencilBuffer.Get();
		myDepthStencilView = aFB->myDepthStencilView.Get();
	}

	void Framebuffer::UnBind(ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		std::vector<ID3D11RenderTargetView*> nullViews;
		ID3D11DepthStencilView* dummyView = nullptr;
		nullViews.resize(myRenderTargetViews.size());
		for (auto& rtv : nullViews)
		{
			rtv = nullptr;
		}
		aContext->OMSetRenderTargets(nullViews.size(), nullViews.data(), dummyView);
	}



	void Framebuffer::Clear(const Vector4f& aColor, ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		float clr[4] = { aColor.x, aColor.y, aColor.z, aColor.w };
		for (auto& view : myRenderTargetViews)
		{
			if (view)
				aContext->ClearRenderTargetView(view.Get(), clr);
		}
		if (myDepthStencilView)
		{
			aContext->ClearDepthStencilView(myDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}

	void Framebuffer::FlushResizes()
	{
		myQueueLock.lock();
		for (auto& func : myStaticResizeQueue)
		{
			func();
		}
		myStaticResizeQueue.clear();
		myQueueLock.unlock();
	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecs& aSpecs)
	{
		return CreateRef<Framebuffer>(aSpecs);
	}

	void Framebuffer::Validate()
	{
		std::scoped_lock lock(myValidationLock); // look in Framebuffer.h to understand why.
		if (myDepthStencilBuffer) myDepthStencilBuffer->Release();
		if (myDepthStencilView) myDepthStencilView->Release();
		if (myDepthShaderResource) myDepthShaderResource->Release();
		for (auto& cb : myColorBuffers)
		{
			if (cb) cb->Release();
		}
		myColorBuffers.clear();
		for (auto& rtv : myRenderTargetViews)
		{
			if (rtv) rtv->Release();
		}
		for (auto& srv : myShaderResourceViews)
		{
			if (srv) srv->Release();
		}

		for (size_t i = 0; i < myData.Formats.size(); i++)
		{
			if (!IsDepth(myData.Formats[i]))
			{
				if (myData.Is2DArray)
				{
					ID3D11Texture2D* ptrSurface;
					D3D11_TEXTURE2D_DESC desc = {};
					desc.Width = myData.Width;
					desc.Height = myData.Height;

					desc.MipLevels = myData.Mips;
					desc.Format = FormatToDXFormat(myData.Formats[i]);
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, &ptrSurface));
					// map ptrSurface
					myColorBuffers.emplace_back(ptrSurface);
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = FormatToDXFormat(myData.Formats[i]);
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDecs.Texture2DArray.ArraySize = myData.Depth;
					srvDecs.Texture2DArray.MipLevels = 1;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateRenderTargetView(myColorBuffers[i].Get(), nullptr, myRenderTargetViews[i].GetAddressOf()));
					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myColorBuffers[i].Get(), &srvDecs, myShaderResourceViews[i].GetAddressOf()));
				}
				else if (myData.Is3D)
				{
					ID3D11Texture3D* ptrSurface;
					D3D11_TEXTURE3D_DESC desc = {};
					desc.Width = myData.Width;
					desc.Height = myData.Height;
					desc.Depth = myData.Depth;
					desc.MipLevels = myData.Mips;
					desc.Format = FormatToDXFormat(myData.Formats[i]);
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture3D(&desc, nullptr, &ptrSurface));
					// map ptrSurface
					myColorBuffers.emplace_back(ptrSurface);
					D3D11_RENDER_TARGET_VIEW_DESC rtvDecs{};
					rtvDecs.Format = FormatToDXFormat(myData.Formats[i]);
					rtvDecs.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
					rtvDecs.Texture3D.WSize = myData.Depth;
					rtvDecs.Texture3D.FirstWSlice = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateRenderTargetView(myColorBuffers[i].Get(), nullptr, myRenderTargetViews[i].GetAddressOf()));

					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = FormatToDXFormat(myData.Formats[i]);
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
					srvDecs.Texture3D.MipLevels = 1;
					srvDecs.Texture3D.MostDetailedMip = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myColorBuffers[i].Get(), &srvDecs, myShaderResourceViews[i].GetAddressOf()));
				}
				else if (myData.IsCube)
				{
					ID3D11Texture2D* ptrSurface;
					D3D11_TEXTURE2D_DESC desc = {};
					desc.Width = myData.Width;
					desc.Height = myData.Height;
					desc.ArraySize = 6;
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
					desc.MipLevels = myData.Mips;
					desc.Format = FormatToDXFormat(myData.Formats[i]);
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, &ptrSurface));
					// map ptrSurface
					myColorBuffers.emplace_back(ptrSurface);
					D3D11_RENDER_TARGET_VIEW_DESC rtvDecs{};
					rtvDecs.Format = FormatToDXFormat(myData.Formats[i]);
					rtvDecs.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDecs.Texture2DArray.ArraySize = 6;
					rtvDecs.Texture2DArray.MipSlice = 0;
					rtvDecs.Texture2DArray.FirstArraySlice = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateRenderTargetView(myColorBuffers[i].Get(), nullptr, myRenderTargetViews[i].GetAddressOf()));

					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = FormatToDXFormat(myData.Formats[i]);
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					srvDecs.TextureCube.MipLevels = 1;
					srvDecs.TextureCube.MostDetailedMip = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myColorBuffers[i].Get(), &srvDecs, myShaderResourceViews[i].GetAddressOf()));
				}
				else
				{
					ID3D11Texture2D* ptrSurface;
					D3D11_TEXTURE2D_DESC desc = {};
					desc.Width = myData.Width;
					desc.Height = myData.Height;
					bool isTooSmall = (myData.Width < 32 || myData.Height < 32);
					desc.MipLevels = isTooSmall ? 1 : myData.Mips;
					desc.ArraySize = 1;
					desc.Format = FormatToDXFormat(myData.Formats[i]);
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&desc, nullptr, &ptrSurface));
					// map ptrSurface
					myColorBuffers.emplace_back(ptrSurface);
					FF_DX_ASSERT(GraphicsContext::Device()->CreateRenderTargetView(myColorBuffers[i].Get(), nullptr, myRenderTargetViews[i].GetAddressOf()));
					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myColorBuffers[i].Get(), nullptr, myShaderResourceViews[i].GetAddressOf()));
				}
			}
			else
			{
				if (myData.IsCube)
				{
					D3D11_TEXTURE2D_DESC depthStencilDesc = {};
					depthStencilDesc.Width = myData.Width;
					depthStencilDesc.Height = myData.Height;
					depthStencilDesc.MipLevels = myData.DepthMips;
					depthStencilDesc.ArraySize = myData.Depth;
					depthStencilDesc.Format = FormatToDXFormat(myData.Formats[i]);
					depthStencilDesc.SampleDesc.Count = myData.Samples;
					depthStencilDesc.SampleDesc.Quality = 0;
					depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
					depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

					depthStencilDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&depthStencilDesc, nullptr, (ID3D11Texture2D**)myDepthStencilBuffer.GetAddressOf()));
					D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
					desc.Format = GetResourceDepthFormat(myData.Formats[i]);
					desc.Texture2D.MipSlice = 0;
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.ArraySize = myData.Depth;
					desc.Texture2DArray.MipSlice = 0;
					desc.Texture2DArray.FirstArraySlice = 0;

					FF_DX_ASSERT(GraphicsContext::Device()->CreateDepthStencilView(myDepthStencilBuffer.Get(), &desc, myDepthStencilView.GetAddressOf()));
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = GetResourceFormat(myData.Formats[i]);
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					srvDecs.TextureCube.MipLevels = 1;
					srvDecs.TextureCube.MostDetailedMip = 0;

					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myDepthStencilBuffer.Get(), &srvDecs, myDepthShaderResource.GetAddressOf()));
				}
				else if (myData.Is2DArray)
				{
					D3D11_TEXTURE2D_DESC depthStencilDesc = {};
					depthStencilDesc.Width = myData.Width;
					depthStencilDesc.Height = myData.Height;
					depthStencilDesc.MipLevels = myData.DepthMips;
					depthStencilDesc.ArraySize = myData.Depth;
					depthStencilDesc.Format = FormatToDXFormat(myData.Formats[i]);
					depthStencilDesc.SampleDesc.Count = myData.Samples;
					depthStencilDesc.SampleDesc.Quality = 0;
					depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
					depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
					depthStencilDesc.MiscFlags = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&depthStencilDesc, nullptr, (ID3D11Texture2D**)myDepthStencilBuffer.GetAddressOf()));
					D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
					desc.Format = GetResourceDepthFormat(myData.Formats[i]);;
					desc.Texture2D.MipSlice = 0;
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.ArraySize = myData.Depth;
					desc.Texture2DArray.MipSlice = 0;
					desc.Texture2DArray.FirstArraySlice = 0;

					FF_DX_ASSERT(GraphicsContext::Device()->CreateDepthStencilView(myDepthStencilBuffer.Get(), &desc, myDepthStencilView.GetAddressOf()));
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = GetResourceFormat(myData.Formats[i]);;
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDecs.Texture2DArray.ArraySize = myData.Depth;
					srvDecs.Texture2DArray.MipLevels = 1;


					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myDepthStencilBuffer.Get(), &srvDecs, myDepthShaderResource.GetAddressOf()));
				}
				else
				{
					D3D11_TEXTURE2D_DESC depthStencilDesc = {};
					depthStencilDesc.Width = myData.Width;
					depthStencilDesc.Height = myData.Height;
					depthStencilDesc.MipLevels = myData.DepthMips;
					depthStencilDesc.ArraySize = 1;
					depthStencilDesc.Format = FormatToDXFormat(myData.Formats[i]);
					depthStencilDesc.SampleDesc.Count = myData.Samples;
					depthStencilDesc.SampleDesc.Quality = 0;
					depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
					depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
					depthStencilDesc.MiscFlags = 0;
					FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&depthStencilDesc, nullptr, (ID3D11Texture2D**)myDepthStencilBuffer.GetAddressOf()));
					D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
					desc.Format = GetResourceDepthFormat(myData.Formats[i]);
					desc.Texture2D.MipSlice = 0;
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

					FF_DX_ASSERT(GraphicsContext::Device()->CreateDepthStencilView(myDepthStencilBuffer.Get(), &desc, myDepthStencilView.GetAddressOf()));
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDecs{};
					srvDecs.Format = GetResourceFormat(myData.Formats[i]);
					srvDecs.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDecs.Texture2D.MipLevels = 1;
					srvDecs.Texture2D.MostDetailedMip = 0;


					FF_DX_ASSERT(GraphicsContext::Device()->CreateShaderResourceView(myDepthStencilBuffer.Get(), &srvDecs, myDepthShaderResource.GetAddressOf()));
				}

			}
		}

		myViewport.Width = (float)myData.Width;
		myViewport.Height = (float)myData.Height;
		myViewport.MinDepth = 0.0f;
		myViewport.MaxDepth = 1.0f;
		myViewport.TopLeftX = 0;
		myViewport.TopLeftY = 0;
	}
}
