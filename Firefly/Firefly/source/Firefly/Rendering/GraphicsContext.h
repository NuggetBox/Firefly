#pragma once
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector.h"
#include <Firefly/Rendering/RenderStateManager.h>
#include <d3d11_1.h>
//#include "RenderStateManager.h"

using namespace Microsoft::WRL;
namespace Firefly
{
	// DO NOT OPEN!
	enum class TopologyType
	{
		UNDEFINED = 0,
		POINTLIST = 1,
		LINELIST = 2,
		LINESTRIP = 3,
		TRIANGLELIST = 4,
		TRIANGLESTRIP = 5,
		LINELIST_ADJ = 10,
		LINESTRIP_ADJ = 11,
		TRIANGLELIST_ADJ = 12,
		TRIANGLESTRIP_ADJ = 13,
		TOPOLOGY_1_CONTROL_POINT_PATCHLIST = 33,
		TOPOLOGY_2_CONTROL_POINT_PATCHLIST = 34,
		TOPOLOGY_3_CONTROL_POINT_PATCHLIST = 35,
		TOPOLOGY_4_CONTROL_POINT_PATCHLIST = 36,
		TOPOLOGY_5_CONTROL_POINT_PATCHLIST = 37,
		TOPOLOGY_6_CONTROL_POINT_PATCHLIST = 38,
		TOPOLOGY_7_CONTROL_POINT_PATCHLIST = 39,
		TOPOLOGY_8_CONTROL_POINT_PATCHLIST = 40,
		TOPOLOGY_9_CONTROL_POINT_PATCHLIST = 41,
		TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42,
		TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43,
		TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44,
		TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45,
		TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46,
		TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47,
		TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48,
		TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49,
		TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50,
		TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51,
		TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52,
		TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53,
		TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54,
		TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55,
		TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56,
		TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57,
		TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58,
		TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59,
		TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60,
		TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61,
		TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62,
		TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63,
		TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64,
	};

	// some info about swapchain creation
	struct GraphicsContextInfo
	{
		uint32_t Width;
		uint32_t Height;
		bool Fullscreen;
		bool EnableDebug;
		bool Vsync = false;
	};
	class Framebuffer;
	// DX11
	class GraphicsContext
	{
	public:
		// Creates The swapchain and calls the Validate function.
		static void Initialize(const GraphicsContextInfo& aInfo);
		// Clears the swapchain views to a desired color.
		static void Clear(const Vector4f& color = { 0.62f,0.f,0.77f, 1.f });
		// resizes the swapchain views to desired size.
		static void VSync(const bool& aShouldVSync);
		static void FullScreen(const bool& aActivate);
		static void BeginEvent(const std::string& aTag);
		static void EndEvent();
		static void Resize(uint32_t aWidth, uint32_t aHeight);
		// presents the the rendertarget to the window.
		static void Present();
		// Sets rendertargetview
		static void BindSwapchainRTV();

		static void Topology(const TopologyType& aTopology, ID3D11DeviceContext* aContext);

		static void TransferFBtoSwapchain(Framebuffer* fb);
		static ComPtr<IDXGISwapChain>& SwapChain() { return mySwapchain; }
		static ComPtr<ID3D11Device>& Device() { return myDevice; }
		static ComPtr<ID3D11DeviceContext>& Context() { return myDeviceContext; }
		static ComPtr<ID3D11DeviceContext> GetCommandList();
		static std::string HResultToString(HRESULT aResult);
		static RenderStateManager& GetRenderStateManager() { return myRenderStateManager; }
		static bool IsCreated() { return myIsCreated; }
	private:
		static void ReCreateDevice();
		// Recreates the views to change size.
		static void Validate();

		inline static RenderStateManager myRenderStateManager;
		inline static GraphicsContextInfo myGraphicsContextInfo;

		inline static ComPtr<IDXGISwapChain> mySwapchain;
		inline static ComPtr<ID3D11Device> myDevice;
		inline static ComPtr<ID3DUserDefinedAnnotation> myAnnotaion;
		inline static ComPtr<ID3D11DeviceContext> myDeviceContext;
		inline static ComPtr<ID3D11RenderTargetView> mySwapchainRenderTargetView;

		inline static bool myIsCreated = false;
	};
}
