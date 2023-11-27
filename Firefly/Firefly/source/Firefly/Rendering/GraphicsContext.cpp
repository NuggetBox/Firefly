#include "FFpch.h"
#include "GraphicsContext.h"


#include "Firefly/Application/Application.h"
#include "Firefly/Rendering/Framebuffer.h"

#include "Postprocess/PostProcessUtils.h"

#include "Shader/ShaderLibrary.h"

namespace Firefly
{
	void GraphicsContext::Initialize(const GraphicsContextInfo& aInfo)
	{
		myGraphicsContextInfo = aInfo;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = Application::Get().GetWindow()->GetHandle();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = !aInfo.Fullscreen;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		FF_DX_ASSERT(D3D11CreateDeviceAndSwapChain(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			aInfo.EnableDebug ? D3D11_CREATE_DEVICE_DEBUG : NULL,
			nullptr,
			NULL,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			mySwapchain.GetAddressOf(),
			myDevice.GetAddressOf(),
			nullptr,
			myDeviceContext.GetAddressOf()));

		//myDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)myAnnotaion.GetAddressOf());

		LOGINFO("Swapchain creation success.");
		ID3D11Texture2D* backBuffer;
		FF_DX_ASSERT(mySwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
		FF_DX_ASSERT(myDevice->CreateRenderTargetView(backBuffer, NULL, &mySwapchainRenderTargetView));
		backBuffer->Release();
		backBuffer = nullptr;
		
		Validate();
		myDeviceContext->QueryInterface(myAnnotaion.GetAddressOf());
		myRenderStateManager.Initialize();
		myIsCreated = true;
	}

	void GraphicsContext::Clear(const Vector4f& color)
	{
		float col[4] = { color.x, color.y, color.z, color.w };
		myDeviceContext->ClearRenderTargetView(mySwapchainRenderTargetView.Get(), col);
	}

	void GraphicsContext::VSync(const bool& aShouldVSync)
	{
		myGraphicsContextInfo.Vsync = aShouldVSync;
	}

	void GraphicsContext::FullScreen(const bool& aActivate)
	{
		if (aActivate)
		{
			Resize(Application::Get().GetWindow()->GetWidth(), Application::Get().GetWindow()->GetHeight());
		}
		mySwapchain->SetFullscreenState(aActivate, NULL);
	}

	void GraphicsContext::BeginEvent(const std::string& aTag)
	{
		std::filesystem::path wstr(aTag);
		myAnnotaion->BeginEvent(wstr.c_str());
	}

	void GraphicsContext::EndEvent()
	{
		myAnnotaion->EndEvent();
	}

	void GraphicsContext::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		myGraphicsContextInfo.Width = aWidth;
		myGraphicsContextInfo.Height = aHeight;
		Validate();
	}

	void GraphicsContext::Present()
	{
		auto hr = mySwapchain->Present(myGraphicsContextInfo.Vsync, 0);
		if (hr != S_OK)
		{
			auto hung = myDevice->GetDeviceRemovedReason();

			switch (hung)
			{
			case DXGI_ERROR_DEVICE_HUNG:
				LOGERROR("The graphics driver stopped responding because of an invalid combination of graphics commands sent by the app. If you get this error repeatedly, it is a likely indication that your app caused the device to hang and needs to be debugged.");
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				LOGERROR("The graphics device has been physically removed, turned off, or a driver upgrade has occurred. This happens occasionally and is normal; your app or game should recreate device resources as described in this topic.");
				ReCreateDevice();
				break;
			case DXGI_ERROR_DEVICE_RESET:
				LOGERROR("The graphics device failed because of a badly formed command. If you get this error repeatedly, it may mean that your code is sending invalid drawing commands.");
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				LOGERROR("The graphics driver encountered an error and reset the device.");
				break;
			case DXGI_ERROR_INVALID_CALL:
				LOGERROR("The application provided invalid parameter data. If you get this error even once, it means that your code caused the device removed condition and must be debugged.");
				break;
			default:
				break;
			}

		}
	}

	void GraphicsContext::BindSwapchainRTV()
	{
		myDeviceContext->OMSetRenderTargets(1, mySwapchainRenderTargetView.GetAddressOf(), nullptr);
	}

	void GraphicsContext::Topology(const TopologyType& aTopology, ID3D11DeviceContext* aContext)
	{
		aContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)aTopology);
	}

	void GraphicsContext::TransferFBtoSwapchain(Framebuffer* fb)
	{
		/*Topology(TopologyType::TRIANGLELIST, myDeviceContext.Get());
		fb->BindSRV(0, 0, ShaderType::Pixel, myDeviceContext.Get());
		ShaderLibrary::Bind("Copy", myDeviceContext.Get());
		myDeviceContext->Draw(3, 0);
		ShaderLibrary::Unbind("Copy", myDeviceContext.Get());
		fb->UnBindShaderResource(0, ShaderType::Pixel, myDeviceContext.Get());*/
		ID3D11Texture2D* backBuffer;
		FF_DX_ASSERT(mySwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

		D3D11_TEXTURE2D_DESC desc = {};
		backBuffer->GetDesc(&desc);

		fb->Resize({ desc.Width, desc.Height }, true);
		myDeviceContext->CopyResource(backBuffer, fb->GetTextures2D(0).Get());
		backBuffer->Release();
		backBuffer = nullptr;
	}

	ComPtr<ID3D11DeviceContext> GraphicsContext::GetCommandList()
	{
		ComPtr<ID3D11DeviceContext> commandList;
		myDevice->CreateDeferredContext(0, commandList.GetAddressOf());
		return commandList;
	}

	std::string GraphicsContext::HResultToString(HRESULT aResult)
	{
		switch (aResult)
		{
		case S_OK: return "Successful"; // Operation successful	0x00000000
		case E_ABORT: return "Aborted"; // Operation aborted 0x80004004
		case E_ACCESSDENIED: return "Access Denied"; // General access denied error	0x80070005
		case E_FAIL: return "Unspecified Failure"; // Unspecified failure 0x80004005
		case E_HANDLE: return "Invalid Handle"; // Handle that is not valid 0x80070006
		case E_INVALIDARG: return "Invalid Args"; // One or more arguments are not valid 0x80070057
		case E_NOINTERFACE: return "Interface not Supported"; //  No such interface supported 0x80004002
		case E_NOTIMPL: return "Not Implemented"; // Not implemented 0x80004001
		case E_OUTOFMEMORY: return "Out of Memory"; //  Failed to allocate necessary memory	0x8007000E
		case E_POINTER:return "Invalid Pointer"; // Pointer that is not valid	0x80004003
		case E_UNEXPECTED: return "Unexpected Failure"; // Unexpected failure 0x8000FFFF
		default: return "Unknown Error Result";
		}
	}

	void GraphicsContext::ReCreateDevice()
	{
		mySwapchain = nullptr;
		myDeviceContext = nullptr;
		myDevice = nullptr;
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = Application::Get().GetWindow()->GetHandle();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = !myGraphicsContextInfo.Fullscreen;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		FF_DX_ASSERT(D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			myGraphicsContextInfo.EnableDebug ? D3D11_CREATE_DEVICE_DEBUG : NULL,
			NULL,
			NULL,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			mySwapchain.GetAddressOf(),
			myDevice.GetAddressOf(),
			NULL,
			myDeviceContext.GetAddressOf()));
	}

	void GraphicsContext::Validate()
	{
		if (myGraphicsContextInfo.Width == 0 || myGraphicsContextInfo.Height == 0)
		{
			return;
		}

		FF_ASSERT(myDeviceContext);
		FF_ASSERT(myDevice);
		FF_ASSERT(mySwapchain);

		mySwapchainRenderTargetView.Reset();

		ID3D11RenderTargetView* nullViews[] = { nullptr, nullptr };
		myDeviceContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
		myDeviceContext->Flush();
		//Resize swap chain
		FF_DX_ASSERT(mySwapchain->ResizeBuffers(1, myGraphicsContextInfo.Width, myGraphicsContextInfo.Height, DXGI_FORMAT_UNKNOWN, 0));

		////Create new views
		ID3D11Texture2D* backBuffer;
		FF_DX_ASSERT(mySwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

		FF_DX_ASSERT(myDevice->CreateRenderTargetView(backBuffer, 0, &mySwapchainRenderTargetView));

		backBuffer->Release();


		myDeviceContext->OMSetRenderTargets(1, mySwapchainRenderTargetView.GetAddressOf(), nullptr);
		//Bind to output merger

		//Set viewport
		D3D11_VIEWPORT viewportDesc = {};
		viewportDesc.TopLeftX = 0.f;
		viewportDesc.TopLeftY = 0.f;
		viewportDesc.Width = static_cast<float>(myGraphicsContextInfo.Width);
		viewportDesc.Height = static_cast<float>(myGraphicsContextInfo.Height);
		viewportDesc.MinDepth = 0.f;
		viewportDesc.MaxDepth = 1.f;

		myDeviceContext->RSSetViewports(1, &viewportDesc);
	}
}
