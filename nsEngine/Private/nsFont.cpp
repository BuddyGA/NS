#include "nsFont.h"
#include "nsLogger.h"
#include "nsFileSystem.h"
#include "nsTextureManager.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "ThirdParty/stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "ThirdParty/stb_truetype.h"


NS_ENGINE_DEFINE_HANDLE(nsFontID);


static nsLogCategory FontLog = { "nsFontLog", nsELogVerbosity::LV_DEBUG };


struct nsFontData
{
	stbtt_packedchar PackedCharacters[96];
	float FontSize;
	float Ascent;
	float Descent;
	float LineSpace;
};

static nsTArrayFreeList<stbtt_fontinfo> FontInfos;
static nsTArrayFreeList<nsFontData> FontDatas;
static nsTArrayFreeList<nsTextureID> FontTextures;
static bool bFontManagerInitialized;



void nsFontManager::Initialize(const nsString& defaultFontTTFFile, float defaultFontSize) noexcept
{
	if (bFontManagerInitialized)
	{
		return;
	}

	FontInfos.Reserve(16);
	FontDatas.Reserve(16);
	FontTextures.Reserve(16);

	nsFontID defaultFont = CreateFontTTF(defaultFontTTFFile, defaultFontSize);

	bFontManagerInitialized = true;
}


nsFontID nsFontManager::CreateFontTTF(const nsString& ttfFile, float fontSize)
{
	const int infoId = FontInfos.Add();
	const int dataId = FontDatas.Add();
	const int texId = FontTextures.Add();
	NS_Assert(infoId == dataId && dataId == texId);

	const nsName ext = nsFileSystem::FileGetExtension(ttfFile);
	NS_ValidateV(ext == ".ttf", "File must be a valid TTF File!");

	const nsString name = nsFileSystem::FileGetName(ttfFile);


	// Read TTF file
	nsTArray<uint8> fileData;
	nsFileSystem::FileReadBinary(ttfFile, fileData);

	if (fileData.IsEmpty())
	{
		NS_LogError(FontLog, "Fail to create font. Cannot read data from file [%s]!", *ttfFile);
		return nsFontID::INVALID;
	}

	// Init font
	stbtt_fontinfo& fontInfo = FontInfos[infoId];
	{
		const int error = stbtt_InitFont(&fontInfo, fileData.GetData(), 0);
		NS_Assert(error != 0);
		NS_LogInfo(FontLog, "Create new font [%s, %ipx]", *name, static_cast<int>(fontSize));
	}


	// Init font datas
	constexpr uint32 WIDTH = 256;
	constexpr uint32 HEIGHT = 256;
	constexpr uint32 PIXEL_SIZE = WIDTH * HEIGHT;
	nsTArray<uint8> pixels(PIXEL_SIZE);
	nsFontData& fontData = FontDatas[dataId];
	{
		stbtt_pack_context context;
		stbtt_PackBegin(&context, pixels.GetData(), WIDTH, HEIGHT, 0, 1, nullptr);
		stbtt_PackSetOversampling(&context, 2, 2);
		stbtt_PackFontRange(&context, fileData.GetData(), 0, fontSize, 32, 96, fontData.PackedCharacters);
		stbtt_PackEnd(&context);

		fontData.FontSize = fontSize;
		const float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
		fontData.Ascent = (float)ascent * scale;
		fontData.Descent = (float)descent * scale;
		fontData.LineSpace = (fontData.Ascent - fontData.Descent) + (float)lineGap * scale;
	}


	// Create and update texture
	{
		FontTextures[texId] = nsTextureManager::Get().CreateTexture2D(nsName::Format("tex_font_%s", *name), nsETextureFormat::UNCOMPRESSED_R, WIDTH, HEIGHT);
		nsTextureManager::Get().UpdateTextureMipData(FontTextures[texId], 0, pixels.GetData(), PIXEL_SIZE);
	}

	return nsFontID(infoId);
}


bool nsFontManager::IsFontValid(nsFontID font) noexcept
{
	return font.IsValid() && FontDatas.IsValid(font.Id);
}


nsRectFloat nsFontManager::CalculateRect(nsFontID font, const nsPointFloat& position, const char* text, int length)
{
	nsRectFloat rect;
	rect.X = position.X;
	rect.Y = position.Y;

	if (text == nullptr || length <= 0)
	{
		return rect;
	}

	NS_Assert(IsFontValid(font));

	const nsFontData& data = FontDatas[font.Id];
	rect.Height = data.Ascent - data.Descent;

	for (int i = 0; i < length; ++i)
	{
		char c = text[i];
		NS_Assert(c != '\0');

		if (c == '\n')
		{
			rect.Height += data.LineSpace;
			continue;
		}

		const float width = data.PackedCharacters[c - 32].xadvance;
		NS_Assert(width >= 0.0f);
		rect.Width += width;
	}
	
	return rect;
}


nsRectFloat nsFontManager::CalculateSelectedRect(nsFontID font, const nsPointFloat& position, const char* text, int length, int selectedIndex, int selectedCount) noexcept
{
	nsRectFloat rect;
	rect.X = position.X;
	rect.Y = position.Y;

	if (text == nullptr || length <= 0 || selectedCount <= 0)
	{
		return rect;
	}

	NS_Assert(IsFontValid(font));
	NS_Assert(selectedIndex >= 0 && selectedIndex < length);
	NS_Assert(selectedIndex + selectedCount <= length);

	const nsFontData& data = FontDatas[font.Id];

	for (int i = 0; i < selectedIndex; ++i)
	{
		char c = text[i];

		if (c == '\n' || c == '\0')
		{
			continue;
		}

		const float width = data.PackedCharacters[c - 32].xadvance;
		NS_Assert(width >= 0.0f);
		rect.X += width;
	}

	for (int i = selectedIndex; i < (selectedIndex + selectedCount); ++i)
	{
		char c = text[i];

		if (c == '\n' || c == '\0')
		{
			continue;
		}

		const float width = data.PackedCharacters[c - 32].xadvance;
		NS_Assert(width >= 0.0f);
		rect.Width += width;
	}

	rect.Height = data.Ascent - data.Descent;

	return rect;
}


float nsFontManager::CalculateCaretPositionX(nsFontID font, const nsString& text, int caretCharIndex)
{
	if (text.IsEmpty() || caretCharIndex <= 0)
	{
		return 0.0f;
	}

	NS_Assert(IsFontValid(font));

	const int charLength = text.GetLength();
	const nsFontData& data = FontDatas[font.Id];
	float advance = 0.0f;

	for (int i = 0; i < charLength; ++i)
	{
		if (i == caretCharIndex)
		{
			break;
		}

		char c = text[i];
		advance += data.PackedCharacters[c - 32].xadvance;
	}

	return advance;
}


int nsFontManager::GenerateVertices(nsFontID font, nsPointFloat& position, const char* text, int length, const nsColor& color, nsTArray<nsVertexGUI>& outVertices, nsTArray<uint32>& outIndices)
{
	if (text == nullptr || length <= 0)
	{
		return 0;
	}

	NS_Assert(IsFontValid(font));

	outVertices.Reserve(outVertices.GetCount() + (length * 4));
	outIndices.Reserve(outIndices.GetCount() + (length * 6));

	const nsFontData& data = FontDatas[font.Id];
	const nsPointInt textureDimension = nsTextureManager::Get().GetTextureDimension(FontTextures[font.Id]);
	position.Y += data.Ascent;
	uint32 vertexCount = 0;
	const nsPointFloat initialPosition = position;
	int charCount = 0;

	for (int i = 0; i < length; ++i)
	{
		char c = text[i];
		NS_Assert(c != '\0');

		if (c == '\n')
		{
			position.X = initialPosition.X;
			position.Y += data.LineSpace;
			continue;
		}

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(data.PackedCharacters, textureDimension.X, textureDimension.Y, c - 32, &position.X, &position.Y, &quad, 0);

		const nsVertexGUI vertices[4] =
		{
			{ nsVector2(quad.x0, quad.y0), nsVector2(quad.s0, quad.t0), color },
			{ nsVector2(quad.x1, quad.y0), nsVector2(quad.s1, quad.t0), color },
			{ nsVector2(quad.x1, quad.y1), nsVector2(quad.s1, quad.t1), color },
			{ nsVector2(quad.x0, quad.y1), nsVector2(quad.s0, quad.t1), color },
		};

		outVertices.InsertAt(vertices, 4);

		const uint32 indices[6] =
		{
			vertexCount,
			vertexCount + 1,
			vertexCount + 2,
			vertexCount + 2,
			vertexCount + 3,
			vertexCount
		};

		outIndices.InsertAt(indices, 6);

		vertexCount += 4;

		charCount++;
	}

	return charCount;
}


float nsFontManager::GetFontSize(nsFontID font)
{
	NS_Assert(IsFontValid(font));

	return FontDatas[font.Id].FontSize;
}


nsTextureID nsFontManager::GetFontTexture(nsFontID font)
{
	NS_Assert(IsFontValid(font));

	return FontTextures[font.Id];
}


nsFontID nsFontManager::GetDefaultFont() noexcept
{
	return nsFontID(0);
}
