#pragma once
#include "Firefly/Rendering/Framebuffer.h"
#include "Firefly/Rendering/Buffer/ConstantBuffers.h"

namespace Firefly
{
	class PostProcessUtils
	{
	public:
		static void CopyFBtoFB(Ref<Framebuffer> aFrom, Ref<Framebuffer> aTo, ID3D11DeviceContext* aContext);
		static void FullScreenPass(const std::string& aShaderKey, Ref<Framebuffer> aIn, Ref<Framebuffer> aOut, ID3D11DeviceContext* aContext);
		static void FullQuadPass(const std::string& aShaderKey, Ref<Framebuffer> aOut, ID3D11DeviceContext* aContext);
	};
}