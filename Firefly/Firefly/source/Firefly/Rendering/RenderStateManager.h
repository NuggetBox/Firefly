#pragma once
#include <stack>
#include <unordered_map>
#include "Firefly/Core/DXHelpers.h"

namespace Firefly
{
	namespace wrl = Microsoft::WRL;

	enum class BlendState
	{
		Opaque,
		Translucent,
		Additive,
		Masked,
		AlphaComposite,
		AlphaHoldout,
		COUNT
	};

	enum class DepthStencilState
	{
		ReadOnly,
		ReadWrite,
		Off,
		COUNT
	};

	enum class SamplerState
	{
		Wrap,
		Mirror,
		Border,
		Point,
		Clamp,
		Shadow,
	};

	enum class CullState
	{
		None,
		Front,
		Back,
		Wireframe,
	};
	inline std::vector<std::string> BlendStateNamesToStringVector()
	{
		static std::vector<std::string> names(6);
		names[0] = "Opaque";
		names[1] = "Translucent";
		names[2] = "Additive";
		names[3] = "Masked";
		names[4] = "AlphaComposite";
		names[5] = "AlphaHoldout";
		return names;
	}

	inline std::vector<std::string> CullStateNamesToStringVector()
	{
		static std::vector<std::string> names(4);
		names[0] = "DoubleSided";
		names[1] = "Front";
		names[2] = "Back";
		names[3] = "Wireframe";
		return names;
	}

	inline std::vector<std::string> DepthStencilStateNamesToStringVector()
	{
		static std::vector<std::string> names(3);
		names[0] = "ReadOnly";
		names[1] = "ReadWrite";
		names[2] = "Off";
		return names;
	}
	class RenderStateManager
	{
	public:
		void Initialize();
		void PushBlendState(BlendState aBlendMode, ID3D11DeviceContext* aContext);
		void PopBlendState(ID3D11DeviceContext* aContext);

		void PushCullState(CullState aState, ID3D11DeviceContext* aContext);
		void PopCullState(ID3D11DeviceContext* aContext);

		void PushDepthStencilState(DepthStencilState aDepthStencilMode, ID3D11DeviceContext* aContext);
		void PopDepthStencilState(ID3D11DeviceContext* aContext);
		void SetSamplerState(SamplerState state, uint32_t index, ID3D11DeviceContext* aContext);
	private:
		void InitializeBlendStates();
		void InitializeDepthStates();
		void InitializeSamlerStates();
		void InitializeCullState();
		void UpdateBlendState(ID3D11DeviceContext* aContext);
		void UpdateDepthStencilState(ID3D11DeviceContext* aContext);
		void UpdateCullState(ID3D11DeviceContext* aContext);

		std::unordered_map<CullState, wrl::ComPtr<ID3D11RasterizerState>> myRasterizerStates;
		std::stack<wrl::ComPtr<ID3D11RasterizerState>> myRasterizerStateStack;
		std::unordered_map<SamplerState, wrl::ComPtr<ID3D11SamplerState>> mySamplerStates;
		std::unordered_map<BlendState, wrl::ComPtr<ID3D11BlendState>> myBlendStates;
		std::stack<wrl::ComPtr<ID3D11BlendState>> myBlendStateStack;
		std::unordered_map<DepthStencilState, wrl::ComPtr<ID3D11DepthStencilState>> myDepthStencilStates;
		std::stack<wrl::ComPtr<ID3D11DepthStencilState>> myDepthStencilStateStack;
	};
}