#include "FFpch.h"
#include "PostProcessUtils.h"
#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"

namespace Firefly
{
	void PostProcessUtils::CopyFBtoFB(Ref<Framebuffer> aFrom, Ref<Framebuffer> aTo, ID3D11DeviceContext* aContext)
	{
		aFrom->BindSRV(0, 0, ShaderType::Pixel, aContext);
		ShaderLibrary::Bind("Copy", aContext);
		aTo->Bind(aContext);
		aContext->Draw(3, 0);
		aTo->UnBind(aContext);
		ShaderLibrary::Unbind("Copy", aContext);
		aFrom->UnBindShaderResource(0, ShaderType::Pixel, aContext);
	}
	void PostProcessUtils::FullScreenPass(const std::string& aShaderKey, Ref<Framebuffer> aIn, Ref<Framebuffer> aOut, ID3D11DeviceContext* aContext)
	{
		aIn->BindSRV(0, 0, ShaderType::Pixel, aContext);
		ShaderLibrary::Bind(aShaderKey, aContext);
		aOut->Bind(aContext);
		aContext->Draw(3, 0);
		aOut->UnBind(aContext);
		ShaderLibrary::Unbind(aShaderKey, aContext);
		aIn->UnBindShaderResource(0, ShaderType::Pixel, aContext);
	}
	void PostProcessUtils::FullQuadPass(const std::string& aShaderKey, Ref<Framebuffer> aOut, ID3D11DeviceContext* aContext)
	{
		ShaderLibrary::Bind(aShaderKey, aContext);
		aOut->Bind(aContext);
		aContext->Draw(3, 0);
		aOut->UnBind(aContext);
		ShaderLibrary::Unbind(aShaderKey, aContext);
	}
}