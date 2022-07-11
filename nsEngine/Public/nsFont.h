#pragma once

#include "nsTextureTypes.h"


NS_ENGINE_DECLARE_HANDLE(nsFontID, nsFontManager)



class nsFontManager
{
public:
	static void Initialize(const nsString& defaultFontTTFFile, float defaultFontSize) noexcept;
	NS_NODISCARD static NS_ENGINE_API nsFontID CreateFontTTF(const nsString& ttfFile, float fontSize);
	NS_NODISCARD static NS_ENGINE_API bool IsFontValid(nsFontID font) noexcept;
	NS_NODISCARD static NS_ENGINE_API nsRectFloat CalculateRect(nsFontID font, const nsPointFloat& position, const char* text, int length);
	NS_NODISCARD static NS_ENGINE_API nsRectFloat CalculateSelectedRect(nsFontID font, const nsPointFloat& position, const char* text, int length, int selectedIndex, int selectedCount) noexcept;
	NS_NODISCARD static NS_ENGINE_API float CalculateCaretPositionX(nsFontID font, const nsString& text, int caretCharIndex);
	static NS_ENGINE_API int GenerateVertices(nsFontID font, nsPointFloat& position, const char* text, int length, const nsColor& color, nsTArray<nsVertexGUI>& outVertices, nsTArray<uint32>& outIndices);
	NS_NODISCARD static NS_ENGINE_API float GetFontSize(nsFontID font);
	NS_NODISCARD static NS_ENGINE_API nsTextureID GetFontTexture(nsFontID font);
	NS_NODISCARD static NS_ENGINE_API nsFontID GetDefaultFont() noexcept;

};
