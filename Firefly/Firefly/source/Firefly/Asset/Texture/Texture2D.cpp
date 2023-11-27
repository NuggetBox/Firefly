#include "FFpch.h"
#include "Texture2D.h"
#include "DDSTextureLoader/DDSTextureLoader11.h"
#include <Firefly/Rendering/RenderCommands.h>
#include <Utils/StringUtils.hpp>

namespace Firefly
{
	Texture2D::~Texture2D()
	{

	}

	bool Texture2D::operator==(Texture2D& other)
	{
		if (myTexture == other.myTexture)
		{
			return true;
		}
		return false;
	}
	Texture2D::Texture2D(uint32_t width, uint32_t height, ImageFormat format, uint32_t levels, void* data, bool aIsFloat, bool aCreateUAV)
	{
		m_Height = height;
		m_Width = width;
		D3D11_TEXTURE2D_DESC texElementDesc = {};
		texElementDesc.Width = width;
		texElementDesc.Height = height;
		texElementDesc.MipLevels = levels;
		texElementDesc.Format = FormatToDXFormat(format);
		texElementDesc.ArraySize = 1;
		texElementDesc.SampleDesc.Count = 1;
		texElementDesc.SampleDesc.Quality = 0;
		texElementDesc.Usage = D3D11_USAGE_DEFAULT;
		texElementDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		if (levels == 0)
		{
			texElementDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texElementDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = data;
		size_t sizeBytes;
		if (aIsFloat)
		{
			subData.SysMemPitch = width * sizeof(float) * 4;
			sizeBytes = width * height * sizeof(float) * 4;
		}
		else
		{
			subData.SysMemPitch = width * 4;
			sizeBytes = width * height * 4;
		}
		myTexturePixels.resize(sizeBytes);
		if (data)
		{
			memcpy_s(myTexturePixels.data(), sizeBytes, data, sizeBytes);
			FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&texElementDesc, &subData, myTexture.GetAddressOf()));
		}
		else
		{
			FF_DX_ASSERT(GraphicsContext::Device()->CreateTexture2D(&texElementDesc, 0, myTexture.GetAddressOf()));
		}

		// Create a resource view to the texture array.
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = texElementDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.TextureCube.MostDetailedMip = 0;
		viewDesc.TextureCube.MipLevels = -1;

		if (FAILED(GraphicsContext::Device()->CreateShaderResourceView(myTexture.Get(), &viewDesc, myShaderResourceView.GetAddressOf())))
		{
			return;
		}
		//gen mip maps
		if (levels == 0)
		{
			//GraphicsContext::Context()->GenerateMips(myShaderResourceView.Get());
		}

		if (aCreateUAV)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};

			desc.Format = texElementDesc.Format;
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 0;

			if (FAILED(GraphicsContext::Device()->CreateUnorderedAccessView(myTexture.Get(), &desc, myUAV.GetAddressOf())))
			{
				LOGERROR("Could not create UAV");
				return;
			}
		}
	}

	Texture2D::Texture2D(const unsigned char* data, uint32_t size)
	{
		auto device = GraphicsContext::Device();
		ID3D11Resource* resource = nullptr;
		DirectX::CreateDDSTextureFromMemory(device.Get(), data, size, &resource, myShaderResourceView.GetAddressOf());
		FF_DX_ASSERT(resource->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(myTexture.GetAddressOf())));
		D3D11_TEXTURE2D_DESC desc;
		myTexture->GetDesc(&desc);
		m_Width = desc.Width;
		m_Height = desc.Height;
		myTexturePixels.resize(size);
		memcpy(myTexturePixels.data(), data, size);
	}

	bool Firefly::Texture2D::Init(const std::filesystem::path& aPath, const bool& isSRGB)
	{
		if (!std::filesystem::exists(aPath))
		{
			LOGERROR("Texture {} does not exist!", aPath.string());
			return false;
		}
		SetPath(aPath);

		std::string fileExtension = myPath.extension().string();

		if (Utils::ToLower(fileExtension) == ".dds")
		{
			auto device = GraphicsContext::Device();
			ID3D11Resource* resource = nullptr;
			DirectX::CreateDDSTextureFromFile(device.Get(), myPath.wstring().c_str(), &resource, myShaderResourceView.GetAddressOf());

			if (!resource)
			{
				LOGERROR("When loading texture with path: \"{}\", resource was null", aPath.string());
				return false;
			}

			FF_DX_ASSERT(resource->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(myTexture.GetAddressOf())));
			D3D11_TEXTURE2D_DESC desc;
			myTexture->GetDesc(&desc);

			m_Width = desc.Width;
			m_Height = desc.Height;
			if (!IsPowerOfTwo(m_Width) || !IsPowerOfTwo(m_Height))
			{
				LOGWARNING("Texture: {0} is not POW of 2! A texture should be POW of 2 due to it being the standard for grading.", aPath.string());
			}
		}
		else
		{
			LOGWARNING("Invalid Texture format");
			return false;
		}

		return true;
	}

	void Firefly::Texture2D::Bind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext)
	{
		globalRendererStats.TextureBinds++;
		switch (shaderType)
		{
		case ShaderType::Vertex: aContext->VSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		case ShaderType::Pixel: aContext->PSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		case ShaderType::Geometry: aContext->GSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		case ShaderType::Hull: aContext->HSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		case ShaderType::Domain: aContext->DSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		case ShaderType::Compute: aContext->CSSetShaderResources(slot, 1, myShaderResourceView.GetAddressOf()); break;
		}
	}


	void Texture2D::UnBind(uint32_t slot, ShaderType shaderType, ID3D11DeviceContext* aContext)
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
	void Texture2D::BindUAV(uint32_t slot, ID3D11DeviceContext* aContext)
	{
		aContext->CSSetUnorderedAccessViews(slot, 1, myUAV.GetAddressOf(), nullptr);
	}
	void Texture2D::UnbindUAV(uint32_t slot, ID3D11DeviceContext* aContext)
	{
		ID3D11UnorderedAccessView* dummyUAV = nullptr;
		aContext->CSSetUnorderedAccessViews(slot, 1, &dummyUAV, nullptr);
	}
	bool Texture2D::IsPOW2(const std::filesystem::path& aPath)
	{
		auto device = GraphicsContext::Device();
		ComPtr<ID3D11Resource> resource;
		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11Texture2D> txr;
		DirectX::CreateDDSTextureFromFile(device.Get(), aPath.wstring().c_str(), resource.GetAddressOf(), srv.GetAddressOf());

		FF_DX_ASSERT(resource->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(txr.GetAddressOf())));
		D3D11_TEXTURE2D_DESC desc;
		txr->GetDesc(&desc);
		if (!IsPowerOfTwo(desc.Width) || !IsPowerOfTwo(desc.Height))
		{
			return false;
		}
		return true;
	}
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, ImageFormat format, uint32_t levels, void* data, bool aIsFloat, bool aCreateUAV)
	{
		return CreateRef<Texture2D>(width, height, format, levels, data, aIsFloat, aCreateUAV);
	}
	Ref<Texture2D> Texture2D::Create(const unsigned char* data, uint32_t size)
	{
		return CreateRef<Texture2D>(data, size);
	}
}
