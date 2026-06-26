#include <NeoWPF/Rendering/TextRenderer.h>

namespace NeoWPF::Rendering::Text {

TextMetrics MeasureText(const std::wstring& text, int fontSize, int maxWidth, int maxHeight)
{
    return {};
}

std::vector<uint8_t> RenderTextBitmap(const std::wstring& text, int fontSize, int& outWidth, int& outHeight)
{
    outWidth = 0;
    outHeight = 0;
    return {};
}

}
