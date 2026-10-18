/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Text/TextTypes.h>

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Rendering::Text {

std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily);

bool RegisterFontFile(const std::wstring& filePath);
bool RegisterFontMemory(const unsigned char* data, size_t size, const std::wstring& familyName);
std::vector<std::wstring> GetRegisteredFontFiles();

}

namespace NeoWPF::Rendering::Text {

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

}
