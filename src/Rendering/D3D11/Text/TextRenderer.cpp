#include <NeoWPF/Rendering/TextRenderer.h>

namespace NeoWPF::Rendering::Text {

bool s_textRendererDebug = false;

TextMetrics TextMeasureTextInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight);
std::vector<uint8_t> TextRenderTextBitmapInternal(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight);

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
{
    return TextMeasureTextInternal(text, fontSize, maxWidth, maxHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, 0, 0, outWidth, outHeight);
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int maxWidth, int maxHeight, int& outWidth, int& outHeight)
{
    return TextRenderTextBitmapInternal(text, fontSize, maxWidth, maxHeight, outWidth, outHeight);
}

}