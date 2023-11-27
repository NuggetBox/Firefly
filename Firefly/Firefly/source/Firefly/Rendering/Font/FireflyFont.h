#pragma once
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include <Firefly/Asset/Texture/Texture2D.h>

namespace Firefly
{
	using namespace msdf_atlas;
#define THREADS 8

	struct FontInput
	{
		const char* FontFilename;
		GlyphIdentifierType GlyphIdentifier;
		const char* CharsetFilename;
		double FontScale;
		const char* FontName;
	};

	struct Configuration
	{
		ImageType ImageTyper;
		msdf_atlas::ImageFormat ImageFormat;
		YDirection YDir;
		int Width, Height;
		double EmSize;
		double PxRange;
		double AngleThreshold;
		double MiterLimit;
		void (*edgeColoring)(msdfgen::Shape&, double, unsigned long long);
		bool ExpensiveColoring;
		unsigned long long ColoringSeed;
		GeneratorAttributes GenAttributes;
	};

	struct MSDFData
	{
		FontGeometry FontGeometry;
		std::vector<GlyphGeometry> GlyphStorage;
	};

	class Font : public Asset
	{
	public:
		Font() = default;
		Font(const std::filesystem::path& afilePath);
		void Load(const std::filesystem::path& afilePath);
		Ref<Texture2D> GetTexture() { return myTexture; }
		MSDFData& GetFontData() { return myData; }
		static Ref<Font> Create(const std::filesystem::path& aFilePath);
		static AssetType GetStaticType() { return AssetType::Font; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }
	private:
		template <typename T, typename S, int N, GeneratorFunction<S, N> GEN_FN>
		Ref<Texture2D> CreateAtlas(const std::string& fontName, float fontSize, const std::vector<GlyphGeometry>& glyphs, const FontGeometry& fontGeometry, const Configuration& config);
		Ref<Texture2D> myTexture;
		MSDFData myData;
	};
	template<typename T, typename S, int N, GeneratorFunction<S, N> GEN_FN>
	inline Ref<Texture2D> Font::CreateAtlas(const std::string& fontName, float fontSize, const std::vector<GlyphGeometry>& glyphs, const FontGeometry& fontGeometry, const Configuration& config)
	{
		ImmediateAtlasGenerator<S, N, GEN_FN, BitmapAtlasStorage<T, N>> generator(config.Width, config.Height);
		generator.setAttributes(config.GenAttributes);
		generator.setThreadCount(THREADS);
		generator.generate(glyphs.data(), glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();
		return CreateRef<Texture2D>((uint32_t)bitmap.width, (uint32_t)bitmap.height, ImageFormat::RGBA32F, 1, (void*)bitmap.pixels);
	}
}