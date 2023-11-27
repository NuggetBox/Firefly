#include "ImageConverter.h"
#pragma warning(push, 0)
#include "../DirectXTex/DirectXTex.h"
#pragma warning(pop)
#include "Firefly/Rendering/GraphicsContext.h"
#include <filesystem>

bool LLL::ImageConverter::ConvertDDSToPNG(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, std::string& aErrorStringOut)
{
	return ConvertDDSToWIC(aInputPath, aOutputPath, DirectX::WIC_CODEC_PNG, aErrorStringOut);
}

bool LLL::ImageConverter::ConvertPNGToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, std::string& aErrorStringOut, bool aGenerateMips)
{
	return ConvertWICToDDS(aInputPath, aOutputPath, aDDSFormat, aGenerateMips, aErrorStringOut);
}

bool LLL::ImageConverter::ConvertDDSToTGA(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, std::string& aErrorStringOut)
{
	if (aInputPath.extension() != ".dds")
	{
		aErrorStringOut = ("Input file doesn't have the '.dds' extension.");
		return false;
	}

	try
	{
		DirectX::ScratchImage sourceImage;
		LoadAndDecompressDDS(sourceImage, aInputPath, aErrorStringOut);

		HRESULT result = DirectX::SaveToTGAFile(*sourceImage.GetImage(0, 0, 0), DirectX::TGA_FLAGS_NONE, aOutputPath.c_str(), &sourceImage.GetMetadata());

		if (FAILED(result))
		{
			throw std::exception("Failed to save dds as tga.");
		}

		return true;
	}
	catch (std::exception& e)
	{
		aErrorStringOut = e.what();
	}

	return false;
}

bool LLL::ImageConverter::ConvertDDSToWIC(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, const uint32_t& aWICCodec, std::string& aErrorStringOut)
{
	if (aInputPath.extension() != ".dds")
	{
		aErrorStringOut = ("Input file doesn't have the '.dds' extension.");
		return false;
	}

	try
	{
		DirectX::ScratchImage sourceImage;
		LoadAndDecompressDDS(sourceImage, aInputPath, aErrorStringOut);

		HRESULT result = DirectX::SaveToWICFile(*sourceImage.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, static_cast<const GUID>(aWICCodec), aOutputPath.c_str());

		if (FAILED(result))
		{
			throw std::exception("Failed to save dds as wic.");
		}

		return true;
	}
	catch (std::exception& e)
	{
		aErrorStringOut = e.what();
	}

	return false;
}

bool LLL::ImageConverter::ConvertWICToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, bool aGenerateMips, std::string& aErrorStringOut)
{
	try
	{
		DirectX::TexMetadata sourceMetadata{};
		DirectX::ScratchImage sourceImage;
		HRESULT result = DirectX::LoadFromWICFile(aInputPath.c_str(), IsSRGBFormat(aDDSFormat) ? DirectX::WIC_FLAGS_DEFAULT_SRGB : DirectX::WIC_FLAGS_NONE, &sourceMetadata, sourceImage);

		if (FAILED(result))
		{
			throw std::exception("Failed to load source image when converting wic to dds.");
		}

		ConvertImageToDDS(sourceImage, aOutputPath, aDDSFormat, aGenerateMips, aErrorStringOut);

		return true;
	}
	catch (std::exception& e)
	{
		aErrorStringOut = e.what();
	}

	return false;
}

bool LLL::ImageConverter::ConvertTGAToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, std::string& aErrorStringOut, bool aGenerateMips)
{
	try
	{
		DirectX::TexMetadata sourceMetadata{};
		DirectX::ScratchImage sourceImage;
		HRESULT result = DirectX::LoadFromTGAFile(aInputPath.c_str(), IsSRGBFormat(aDDSFormat) ? DirectX::TGA_FLAGS_DEFAULT_SRGB : DirectX::TGA_FLAGS_NONE, &sourceMetadata, sourceImage);

		if (FAILED(result))
		{
			throw std::exception("Failed to load source image when converting tga to dds.");
		}

		ConvertImageToDDS(sourceImage, aOutputPath, aDDSFormat, aGenerateMips, aErrorStringOut);

		return true;
	}
	catch (std::exception& e)
	{
		aErrorStringOut = e.what();
	}

	return false;
}

void LLL::ImageConverter::ConvertImageToDDS(DirectX::ScratchImage& aSourceImage, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, bool aGenerateMips, std::string& aErrorStringOut)
{
	//GENERATE MIPS!
	if (aGenerateMips)
	{
		GenerateMipMapsForImage(aSourceImage);
	}

	if (IsCompressedDDSFormat(aDDSFormat))
	{
		CompressImage(aSourceImage, aDDSFormat);
	}
	else
	{
		//non compressed images probably need to be converted or something
	}

	HRESULT result = DirectX::SaveToDDSFile(aSourceImage.GetImages(), aSourceImage.GetImageCount(), aSourceImage.GetMetadata(), DirectX::DDS_FLAGS_NONE, aOutputPath.c_str());

	if (FAILED(result))
	{
		throw std::exception("Failed to save dds.");
	}
}

void LLL::ImageConverter::LoadAndDecompressDDS(DirectX::ScratchImage& aSourceImage, const std::filesystem::path& aInputPath, std::string& aErrorStringOut)
{
	DirectX::TexMetadata sourceMetadata{};
	HRESULT result = DirectX::LoadFromDDSFile(aInputPath.c_str(), DirectX::DDS_FLAGS_NONE, &sourceMetadata, aSourceImage);
	if (FAILED(result))
	{
		throw std::exception(__FUNCTION__"Failed to load source image.");
	}

	DirectX::ScratchImage decompressedSource;
	result = DirectX::Decompress(aSourceImage.GetImages(), aSourceImage.GetImageCount(), aSourceImage.GetMetadata(), DXGI_FORMAT_UNKNOWN, decompressedSource);
	if (FAILED(result))
	{
		throw std::exception(__FUNCTION__"Failed to decompress source image.");
	}

	aSourceImage = std::move(decompressedSource);
}

void LLL::ImageConverter::GenerateMipMapsForImage(DirectX::ScratchImage& aSourceImage)
{
	DirectX::ScratchImage mipChainImage;

	HRESULT result = DirectX::GenerateMipMaps(aSourceImage.GetImages(), aSourceImage.GetImageCount(), aSourceImage.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChainImage);
	if (FAILED(result))
	{
		throw std::exception(__FUNCTION__"Failed to generate mipmaps.");
	}

	aSourceImage = std::move(mipChainImage);
}

void LLL::ImageConverter::CompressImage(DirectX::ScratchImage& aSourceImage, DDSFormat aDDSFormat)
{
	DirectX::ScratchImage outputImage;

	HRESULT result;

	if (IsGPUCodecFormat(aDDSFormat))
	{
		result = DirectX::Compress(Firefly::GraphicsContext::Device().Get(), aSourceImage.GetImages(), aSourceImage.GetImageCount(), aSourceImage.GetMetadata(), DDSFormatToDXGIFormat(aDDSFormat), DirectX::TEX_COMPRESS_DEFAULT, GetDDSFormatAlphaThreshold(aDDSFormat), outputImage);
	}
	else
	{
		result = DirectX::Compress(aSourceImage.GetImages(), aSourceImage.GetImageCount(), aSourceImage.GetMetadata(), DDSFormatToDXGIFormat(aDDSFormat), DirectX::TEX_COMPRESS_DEFAULT, GetDDSFormatAlphaThreshold(aDDSFormat), outputImage);
	}

	if (FAILED(result))
	{
		throw std::exception("Failed to compress source image");
	}

	aSourceImage = std::move(outputImage);
}

DXGI_FORMAT LLL::ImageConverter::DDSFormatToDXGIFormat(DDSFormat aDDSFormat)
{
	switch (aDDSFormat)
	{
		case DDSFormat::BC7UNorm:
			return DXGI_FORMAT_BC7_UNORM;
		case DDSFormat::BC7UNormSRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
	}

	assert(false && "Unknown dds format provided");

	return DXGI_FORMAT_UNKNOWN;
}

bool LLL::ImageConverter::IsCompressedDDSFormat(DDSFormat aDDSFormat)
{
	switch (aDDSFormat)
	{
		case DDSFormat::BC7UNorm:
		case DDSFormat::BC7UNormSRGB:
			return true;
		default:
			return false;
	}
}

bool LLL::ImageConverter::IsGPUCodecFormat(DDSFormat aDDSFormat)
{
	switch (aDDSFormat)
	{
		case DDSFormat::BC7UNorm:
		case DDSFormat::BC7UNormSRGB:
			return true;
		default:
			return false;
	}
}

bool LLL::ImageConverter::IsSRGBFormat(DDSFormat aDDSFormat)
{
	switch (aDDSFormat)
	{
		case DDSFormat::BC7UNorm:
			return false;
		case DDSFormat::BC7UNormSRGB:
			return true;
		default:
			return false;
	}
}

float LLL::ImageConverter::GetDDSFormatAlphaThreshold(DDSFormat aDDSFormat)
{
	switch (aDDSFormat)
	{
		case DDSFormat::BC7UNorm:
		case DDSFormat::BC7UNormSRGB:
			return 1.0f;
		default:
			return DirectX::TEX_THRESHOLD_DEFAULT;
	}
}
