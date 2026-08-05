#pragma once

#include <dwrite.h>
#include <string>
#include <vector>

namespace NeoWPF::Rendering::Text {

std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily);

bool RegisterFontFile(const std::wstring& filePath);
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
 
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, const std::wstring& fontFamily, int& outWidth, int& outHeight);

bool RegisterFontFile(const std::wstring& filePath);

}
