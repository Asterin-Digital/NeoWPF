#pragma once

#include <string>
#include <vector>

namespace NeoWPF::Rendering::Text {

struct TextMetrics {
    int width = 0;
    int height = 0;
};

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight);
std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight);

}
