#include <NeoWPF/UI/Text/TextRasterCache.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI::Text {

TextRasterCache::~TextRasterCache() {
    Clear();
}

void TextRasterCache::Clear() {
    if (NeoWPF::Rendering::Renderer_IsInitialized()) {
        for (auto textureId : m_lineTextureIds) {
            if (textureId != 0) {
                NeoWPF::Rendering::Renderer_DestroyTexture(textureId);
            }
        }
    }
    m_lineTextureIds.clear();
    m_flow = {};
}

bool TextRasterCache::Refresh(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                              double availableWidth, TextWrapping wrapping,
                              int fontWeight,
                              NeoWPF::UI::FontStyle fontStyle) {
    Clear();

    if (text.empty()) {
        return true;
    }

    m_flow = BreakText(text, fontSize, fontFamily, availableWidth, wrapping, fontWeight, fontStyle);
    m_lineTextureIds.reserve(m_flow.lines.size());

    for (auto& line : m_flow.lines) {
        if (line.text.empty()) {
            m_lineTextureIds.push_back(0);
            continue;
        }

        int bitmapWidth = 0;
        int bitmapHeight = 0;
        int inkTop = 0;
        int inkHeight = 0;
        const auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(
            line.text,
            fontSize,
            0,
            0,
            fontFamily,
            fontWeight,
            fontStyle,
            bitmapWidth,
            bitmapHeight,
            inkTop,
            inkHeight);

        line.width = bitmapWidth;
        line.height = bitmapHeight;
        line.inkTop = inkTop;

        const std::uint32_t textureId = !bitmap.empty() && bitmapWidth > 0 && bitmapHeight > 0
            ? NeoWPF::Rendering::Renderer_CreateTexture(bitmapWidth, bitmapHeight, bitmap.data())
            : 0;
        m_lineTextureIds.push_back(textureId);
    }

    return true;
}

std::uint32_t TextRasterCache::GetLineTextureId(std::size_t lineIndex) const noexcept {
    return lineIndex < m_lineTextureIds.size() ? m_lineTextureIds[lineIndex] : 0;
}

}
