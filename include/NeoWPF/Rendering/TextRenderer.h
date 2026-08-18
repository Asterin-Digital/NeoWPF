#pragma once

#include <dwrite.h>
#include <string>
#include <vector>
#include <NeoWPF/UI/Text/FontStyle.h>

namespace NeoWPF::Rendering::Text {

std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily);

bool RegisterFontFile(const std::wstring& filePath);
bool RegisterFontMemory(const unsigned char* data, size_t size, const std::wstring& familyName);
IDWriteFontFace* CreateRegisteredFontFace(const std::wstring& familyName);
std::vector<std::wstring> GetRegisteredFontFiles();
IDWriteFontCollection* GetCustomFontCollection();

}

namespace NeoWPF::Rendering::Text {

struct TextMetrics {
    int width = 0;
    int height = 0;
};

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight);
TextMetrics MeasureText(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight);
TextMetrics MeasureText(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int maxWidth, int maxHeight, int fontWeight, NeoWPF::UI::FontStyle fontStyle);

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight, int& outInkTop, int& outInkHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int fontWeight, NeoWPF::UI::FontStyle fontStyle, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int fontWeight, NeoWPF::UI::FontStyle fontStyle, int& outWidth, int& outHeight, int& outInkTop, int& outInkHeight);

bool RegisterFontFile(const std::wstring& filePath);

}
