#include "FFpch.h"
#include "DeferredContext.h"

#include "Firefly/Rendering/GraphicsContext.h"

namespace Firefly
{
	DeferredContext::DeferredContext()
	{
		myContext = GraphicsContext::GetCommandList();
		myContext->QueryInterface(myAnnotaion.GetAddressOf());
	}
	ID3D11DeviceContext* DeferredContext::Get()
	{
		return myContext.Get();
	}
	void DeferredContext::BeginEvent(std::string_view aName)
	{
		std::filesystem::path wstr(aName);
		myAnnotaion->BeginEvent(wstr.wstring().c_str());
	}
	void DeferredContext::EndEvent()
	{
		myAnnotaion->EndEvent();
	}
	DeferredContext::~DeferredContext()
	{
	}
}