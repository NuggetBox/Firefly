#include "FFpch.h"
#include "Texture3D.h"
#include <Firefly/Rendering/RenderCommands.h>
#include <Utils/StringUtils.hpp>
namespace Firefly
{
	Texture3D::Texture3D(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth, ImageFormat aFormat, uint32_t levels)
	{
		myHeight = aHeight;
		myWidth = aWidth;
		myDepth = aDepth;
		myLevels = levels;
		myImageFormat = aFormat;

		Validate();
	}

	void Texture3D::Resize(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth)
	{
		if (aHeight == 0 || aWidth == 0 || aDepth == 0)
		{
			return;
		}
		myHeight = aHeight;
		myWidth = aWidth;
		myDepth = aDepth;

		Validate();
	}

	void Texture3D::Bind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext)
	{
		switch (shaderType)
		{
		case ShaderType::Vertex: aContext->VSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		case ShaderType::Pixel: aContext->PSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		case ShaderType::Geometry: aContext->GSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		case ShaderType::Hull: aContext->HSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		case ShaderType::Domain: aContext->DSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		case ShaderType::Compute: aContext->CSSetShaderResources(slot, 1, mySRV.GetAddressOf()); break;
		}
	}

	void Texture3D::UnBind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext)
	{
		ID3D11ShaderResourceView* dummySRV = nullptr;
		switch (shaderType)
		{
		case ShaderType::Vertex: aContext->VSSetShaderResources(slot, 1, &dummySRV); break;
		case ShaderType::Pixel: aContext->PSSetShaderResources(slot, 1, &dummySRV); break;
		case ShaderType::Geometry: aContext->GSSetShaderResources(slot, 1, &dummySRV); break;
		case ShaderType::Hull: aContext->HSSetShaderResources(slot, 1, &dummySRV); break;
		case ShaderType::Domain: aContext->DSSetShaderResources(slot, 1, &dummySRV); break;
		case ShaderType::Compute: aContext->CSSetShaderResources(slot, 1, &dummySRV); break;
		}
	}

	void Texture3D::BindUAV(uint32_t slot, ID3D11DeviceContext* aContext)
	{
		aContext->CSSetUnorderedAccessViews(slot, 1, myUAV.GetAddressOf(), nullptr);
	}

	void Texture3D::UnbindUAV(uint32_t slot, ID3D11DeviceContext* aContext)
	{
		ID3D11UnorderedAccessView* view = nullptr;
		aContext->CSSetUnorderedAccessViews(slot, 1, &view, nullptr);
	}

	Ref<Texture3D> Texture3D::Create(uint32_t aWidth, uint32_t aHeight, uint32_t aDepth, ImageFormat aFormat, uint32_t levels)
	{
		return CreateRef<Texture3D>(aWidth, aHeight, aDepth, aFormat, levels);
	}
	void Texture3D::Validate()
	{
		if (myTexture)
		{
			myTexture->Release();
		}
		if (mySRV)
		{
			mySRV->Release();
		}
		if (myUAV)
		{
			myUAV->Release();
		}

		D3D11_TEXTURE3D_DESC texElementDesc = {};
		texElementDesc.Width = myWidth;
		texElementDesc.Height = myHeight;
		texElementDesc.Depth = myDepth;
		texElementDesc.MipLevels = myLevels;
		texElementDesc.Format = FormatToDXFormat(myImageFormat);
		texElementDesc.Usage = D3D11_USAGE_DEFAULT;
		texElementDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;


		FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture3D(&texElementDesc, nullptr, myTexture.GetAddressOf()));
		// Create a resource view to the texture array.
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = texElementDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		viewDesc.Texture3D.MostDetailedMip = 0;
		viewDesc.Texture3D.MipLevels = myLevels;

		if (FAILED(GraphicsContext::Device()->CreateShaderResourceView(myTexture.Get(), &viewDesc, mySRV.GetAddressOf())))
		{
			LOGERROR("Validate Texture3D: Could not create SRV");
			return;
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};

		desc.Format = texElementDesc.Format;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.WSize = myDepth;
		desc.Texture3D.FirstWSlice = 0;
		desc.Texture3D.MipSlice = 0;

		if (FAILED(GraphicsContext::Device()->CreateUnorderedAccessView(myTexture.Get(), &desc, myUAV.GetAddressOf())))
		{
			LOGERROR("Validate Texture3D:  Could not create UAV");
			return;
		}
	}
}
