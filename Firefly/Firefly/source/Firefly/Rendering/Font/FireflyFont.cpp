#include "FFpch.h"
#include "FireflyFont.h"
#include "Firefly/Utilities/BinaryFileUtils.h"
namespace Firefly
{
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define DEFAULT_MITER_LIMIT 1.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull


	static bool FindCompiledFont(const std::filesystem::path& aFilename, std::filesystem::path& outPath)
	{
		static std::string compiledAssetPath = "Editor/CompiledAssets/";
		outPath = compiledAssetPath + aFilename.stem().string() + ".txt";
		if (std::filesystem::exists(outPath))
		{
			return true;
		}
		return false;
	}


    Font::Font(const std::filesystem::path& afilePath)
    {
		Load(afilePath);
	}

	void Font::Load(const std::filesystem::path& afilePath)
	{
		int result = 0;
		FontInput fontInput = { };
		Configuration config = { };
		fontInput.GlyphIdentifier = GlyphIdentifierType::UNICODE_CODEPOINT;
		fontInput.FontScale = -1;
		config.ImageFormat = msdf_atlas::ImageFormat::BINARY_FLOAT;
		config.YDir = YDirection::BOTTOM_UP;
		config.edgeColoring = msdfgen::edgeColoringInkTrap;
		const char* imageFormatName = nullptr;
		int fixedWidth = -1, fixedHeight = -1;
		config.GenAttributes.config.overlapSupport = true;
		config.GenAttributes.scanlinePass = true;
		double minEmSize = 0;
		double rangeValue = 2.0;
		TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
		config.AngleThreshold = DEFAULT_ANGLE_THRESHOLD;
		config.MiterLimit = DEFAULT_MITER_LIMIT;
		config.ImageTyper = ImageType::MTSDF;

		std::string fontFilepath = afilePath.string();
		fontInput.FontFilename = fontFilepath.c_str();

		config.EmSize = 40;

		bool anyCodepointsAvailable = false;
		// Load Font
		class FontHolder {
			msdfgen::FreetypeHandle* ft;
			msdfgen::FontHandle* font;
			const char* fontFilename;
		public:
			FontHolder() : ft(msdfgen::initializeFreetype()), font(nullptr), fontFilename(nullptr) { }
			~FontHolder() {
				if (ft) {
					if (font)
						msdfgen::destroyFont(font);
					msdfgen::deinitializeFreetype(ft);
				}
			}
			bool load(const char* fontFilename) {
				if (ft && fontFilename) {
					if (this->fontFilename && strcmp(this->fontFilename, fontFilename) == 0)
						return true;
					if (font)
						msdfgen::destroyFont(font);
					if ((font = msdfgen::loadFont(ft, fontFilename))) {
						this->fontFilename = fontFilename;
						return true;
					}
					this->fontFilename = nullptr;
				}
				return false;
			}
			operator msdfgen::FontHandle* () const {
				return font;
			}
		} font;
		if (!font.load(fontInput.FontFilename))
			FF_ASSERT(false);
		if (fontInput.FontScale <= 0)
			fontInput.FontScale = 1;

		fontInput.GlyphIdentifier = GlyphIdentifierType::UNICODE_CODEPOINT;
		Charset charset;

		static const uint32_t charsetRanges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0,
		};

		for (int range = 0; range < 8; range += 2)
		{
			for (int c = charsetRanges[range]; c <= charsetRanges[range + 1]; c++)
				charset.add(c);
		}

		myData.FontGeometry = FontGeometry(&myData.GlyphStorage);
		int glyphsLoaded = -1;
		switch (fontInput.GlyphIdentifier)
		{
		case GlyphIdentifierType::GLYPH_INDEX:
			glyphsLoaded = myData.FontGeometry.loadGlyphset(font, fontInput.FontScale, charset);
			break;
		case GlyphIdentifierType::UNICODE_CODEPOINT:
			glyphsLoaded = myData.FontGeometry.loadCharset(font, fontInput.FontScale, charset);
			anyCodepointsAvailable |= glyphsLoaded > 0;
			break;
		}

		if (glyphsLoaded < 0)
		{
			FF_ASSERT(false);
		}

		if (fontInput.FontName)
			myData.FontGeometry.setName(fontInput.FontName);

		double pxRange = rangeValue;
		bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
		bool fixedScale = config.EmSize > 0;
		TightAtlasPacker atlasPacker;
		if (fixedDimensions)
			atlasPacker.setDimensions(fixedWidth, fixedHeight);
		else
			atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
		atlasPacker.setPadding(config.ImageTyper == ImageType::MSDF || config.ImageTyper == ImageType::MTSDF ? 0 : -1);
		// TODO: In this case (if padding is -1), the border pixels of each glyph are black, but still computed. For floating-point output, this may play a role.
		if (fixedScale)
			atlasPacker.setScale(config.EmSize);
		else
			atlasPacker.setMinimumScale(minEmSize);
		atlasPacker.setPixelRange(pxRange);
		atlasPacker.setMiterLimit(config.MiterLimit);


		if (int remaining = atlasPacker.pack(myData.GlyphStorage.data(), myData.GlyphStorage.size()))
		{
			if (remaining < 0)
			{
				FF_ASSERT(false);
			}
			else
			{
				FF_ASSERT(false);
			}
		}

		atlasPacker.getDimensions(config.Width, config.Height);
		config.EmSize = atlasPacker.getScale();
		config.PxRange = atlasPacker.getPixelRange();

		if (config.ImageTyper == ImageType::MSDF || config.ImageTyper == ImageType::MTSDF)
		{
			if (config.ExpensiveColoring) {
				Workload([&glyphs = myData.GlyphStorage, &config](int i, int threadNo) -> bool
					{
						unsigned long long glyphSeed = (LCG_MULTIPLIER * (config.ColoringSeed ^ i) + LCG_INCREMENT) * !!config.ColoringSeed;
						glyphs[i].edgeColoring(config.edgeColoring, config.AngleThreshold, glyphSeed);
						return true;
					}, myData.GlyphStorage.size()).finish(THREADS);
			}
			else
			{
				unsigned long long glyphSeed = config.ColoringSeed;
				for (GlyphGeometry& glyph : myData.GlyphStorage)
				{
					glyphSeed *= LCG_MULTIPLIER;
					glyph.edgeColoring(config.edgeColoring, config.AngleThreshold, glyphSeed);
				}
			}
		}

		std::string fontName = afilePath.filename().string();
		std::filesystem::path compiledPath;
		if (FindCompiledFont(fontName, compiledPath))
		{
			auto bytes = ReadBinary(compiledPath);
			myTexture = CreateRef<Texture2D>((uint32_t)config.Width, (uint32_t)config.Height, ImageFormat::RGBA32F, 1, (void*)bytes.data());
			return;
		}


		LOGINFO("stated to generate font {0}", afilePath.string().c_str());
		bool floatingPointFormat = true;
		switch (config.ImageTyper)
		{
		case ImageType::MSDF:
			if (floatingPointFormat)
				myTexture = CreateAtlas<float, float, 3, msdfGenerator>(fontName, (float)config.EmSize, myData.GlyphStorage, myData.FontGeometry, config);
			else
				myTexture = CreateAtlas<byte, float, 3, msdfGenerator>(fontName, (float)config.EmSize, myData.GlyphStorage, myData.FontGeometry, config);
			break;
		case ImageType::MTSDF:
			if (floatingPointFormat)
				myTexture = CreateAtlas<float, float, 4, mtsdfGenerator>(fontName, (float)config.EmSize, myData.GlyphStorage, myData.FontGeometry, config);
			else
				myTexture = CreateAtlas<byte, float, 4, mtsdfGenerator>(fontName, (float)config.EmSize, myData.GlyphStorage, myData.FontGeometry, config);
			break;
		}
		LOGINFO("Finished to generate font {0}", afilePath.string().c_str());

		size_t sizeOfData = myTexture->GetHeight() * myTexture->GetWidth() * sizeof(float) * 4;

		std::vector<uint8_t> byteVector(sizeOfData);
		memcpy(byteVector.data(), myTexture->Pixels(), sizeOfData);
		WriteBinary(compiledPath, byteVector);
	}

	Ref<Font> Font::Create(const std::filesystem::path& aFilePath)
    {
		return CreateRef<Font>(aFilePath);
    }
}