#pragma once
#include <filesystem>
#include <dxgiformat.h>

namespace DirectX
{
	class ScratchImage;
}

namespace LLL
{
	enum class DDSFormat
	{
		BC7UNorm,
		BC7UNormSRGB
	};

	class ImageConverter
	{
		public:
			static bool ConvertDDSToPNG(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, std::string& aErrorStringOut);
			static bool ConvertPNGToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, std::string& aErrorStringOut, bool aGenerateMips = true);
			static bool ConvertDDSToTGA(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, std::string& aErrorStringOut);
			static bool ConvertTGAToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, std::string& aErrorStringOut, bool aGenerateMips = true);

		private:
			static bool ConvertDDSToWIC(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, const uint32_t& aWICCodec, std::string& aErrorStringOut);
			static bool ConvertWICToDDS(const std::filesystem::path& aInputPath, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, bool aGenerateMips, std::string& aErrorStringOut);

			static void ConvertImageToDDS(DirectX::ScratchImage& aSourceImage, const std::filesystem::path& aOutputPath, DDSFormat aDDSFormat, bool aGenerateMips, std::string& aErrorStringOut);
			static void LoadAndDecompressDDS(DirectX::ScratchImage& aSourceImage, const std::filesystem::path& aPath, std::string& aErrorStringOut);

			static void GenerateMipMapsForImage(DirectX::ScratchImage& aSourceImage);
			static void CompressImage(DirectX::ScratchImage& aSourceImage, DDSFormat aDDSFormat);

			static DXGI_FORMAT DDSFormatToDXGIFormat(DDSFormat aDDSFormat);
			static bool IsCompressedDDSFormat(DDSFormat aDDSFormat);
			static bool IsGPUCodecFormat(DDSFormat aDDSFormat);
			static bool IsSRGBFormat(DDSFormat aDDSFormat);
			static float GetDDSFormatAlphaThreshold(DDSFormat aDDSFormat);
	};
}
