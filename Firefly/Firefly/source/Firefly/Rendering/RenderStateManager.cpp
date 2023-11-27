#include "FFpch.h"
#include "RenderStateManager.h"

#include "GraphicsContext.h"

void Firefly::RenderStateManager::Initialize()
{
	InitializeBlendStates();
	InitializeDepthStates();
	InitializeSamlerStates();
	InitializeCullState();
	PushBlendState(BlendState::Opaque, GraphicsContext::Context().Get());
	PushDepthStencilState(DepthStencilState::ReadWrite, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Wrap, 0, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Border, 1, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Mirror, 2, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Point, 3, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Clamp, 4, GraphicsContext::Context().Get());
	SetSamplerState(SamplerState::Shadow, 5, GraphicsContext::Context().Get());
	PushCullState(CullState::None, GraphicsContext::Context().Get());
}

void Firefly::RenderStateManager::PushBlendState(BlendState aBlendMode, ID3D11DeviceContext* aContext)
{
	myBlendStateStack.push(myBlendStates[aBlendMode]);
	UpdateBlendState(aContext);
}

void Firefly::RenderStateManager::PopBlendState(ID3D11DeviceContext* aContext)
{
	myBlendStateStack.pop();
	UpdateBlendState(aContext);
}


void Firefly::RenderStateManager::PushCullState(CullState state, ID3D11DeviceContext* aContext)
{
	myRasterizerStateStack.push(myRasterizerStates[state]);
	UpdateCullState(aContext);
}

void Firefly::RenderStateManager::PopCullState(ID3D11DeviceContext* aContext)
{
	myRasterizerStateStack.pop();
	UpdateCullState(aContext);
}

void Firefly::RenderStateManager::PushDepthStencilState(DepthStencilState aDepthStencilMode, ID3D11DeviceContext* aContext)
{
	myDepthStencilStateStack.push(myDepthStencilStates[aDepthStencilMode]);
	UpdateDepthStencilState(aContext);
}

void Firefly::RenderStateManager::PopDepthStencilState(ID3D11DeviceContext* aContext)
{
	myDepthStencilStateStack.pop();
	UpdateDepthStencilState(aContext);
}

void Firefly::RenderStateManager::SetSamplerState(SamplerState state, uint32_t index, ID3D11DeviceContext* aContext)
{
	aContext->PSSetSamplers(index, 1, mySamplerStates[state].GetAddressOf());
	aContext->VSSetSamplers(index, 1, mySamplerStates[state].GetAddressOf());
	aContext->CSSetSamplers(index, 1, mySamplerStates[state].GetAddressOf());
}

void Firefly::RenderStateManager::InitializeCullState()
{
	D3D11_RASTERIZER_DESC cmDesc = {};
	cmDesc.FillMode = D3D11_FILL_SOLID;
	cmDesc.CullMode = D3D11_CULL_BACK;
	cmDesc.FrontCounterClockwise = false;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateRasterizerState(&cmDesc, &myRasterizerStates[CullState::Back]));
	cmDesc.CullMode = D3D11_CULL_FRONT;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateRasterizerState(&cmDesc, &myRasterizerStates[CullState::Front]));
	cmDesc.CullMode = D3D11_CULL_NONE;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateRasterizerState(&cmDesc, &myRasterizerStates[CullState::None]));
	cmDesc.FillMode = D3D11_FILL_WIREFRAME;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateRasterizerState(&cmDesc, &myRasterizerStates[CullState::Wireframe]));
}

void Firefly::RenderStateManager::InitializeBlendStates()
{
	//Setup BlendState None
	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::Opaque].GetAddressOf()));
	//

	blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::Masked].GetAddressOf()));

	blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::Translucent].GetAddressOf()));

	blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::Additive].GetAddressOf()));

	blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::AlphaComposite].GetAddressOf()));

	blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	FF_DX_ASSERT(GraphicsContext::Device()->CreateBlendState(&blendDesc, myBlendStates[BlendState::AlphaHoldout].GetAddressOf()));

	LOGINFO("Blend States setup successfully");
}

void Firefly::RenderStateManager::InitializeDepthStates()
{
	//Setup depth readonly
	D3D11_DEPTH_STENCIL_DESC readOnlyDepthDesc{};
	readOnlyDepthDesc.DepthEnable = true;
	readOnlyDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	readOnlyDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	readOnlyDepthDesc.StencilEnable = false;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateDepthStencilState(&readOnlyDepthDesc, &myDepthStencilStates[DepthStencilState::ReadOnly]));
	//

	//Setup depth readwrite
	myDepthStencilStates[DepthStencilState::ReadWrite] = nullptr;
	//

	//Setup depth off
	D3D11_DEPTH_STENCIL_DESC depthOffDesc{};
	depthOffDesc.DepthEnable = false;
	depthOffDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthOffDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthOffDesc.StencilEnable = false;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateDepthStencilState(&depthOffDesc, &myDepthStencilStates[DepthStencilState::Off]));
	//

	LOGINFO("Depth Stencil States setup successfully");
}

void Firefly::RenderStateManager::InitializeSamlerStates()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateSamplerState(&samplerDesc, &mySamplerStates[SamplerState::Wrap]));
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateSamplerState(&samplerDesc, &mySamplerStates[SamplerState::Point]));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateSamplerState(&samplerDesc, &mySamplerStates[SamplerState::Clamp]));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateSamplerState(&samplerDesc, &mySamplerStates[SamplerState::Border]));
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	FF_DX_ASSERT(GraphicsContext::Device()->CreateSamplerState(&samplerDesc, &mySamplerStates[SamplerState::Shadow]));
}

void Firefly::RenderStateManager::UpdateBlendState(ID3D11DeviceContext* aContext)
{
	aContext->OMSetBlendState(myBlendStateStack.top().Get(), nullptr, 0xFFFFFFFF);
}

void Firefly::RenderStateManager::UpdateDepthStencilState(ID3D11DeviceContext* aContext)
{
	aContext->OMSetDepthStencilState(myDepthStencilStateStack.top().Get(), 0);
}

void Firefly::RenderStateManager::UpdateCullState(ID3D11DeviceContext* aContext)
{
	aContext->RSSetState(myRasterizerStateStack.top().Get());
}