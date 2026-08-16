#include <NeoWPF/UI/Text/TextRasterCache.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>

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
    m_hasCache = false;
}

bool TextRasterCache::Refresh(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                             int availableWidth, TextWrapping wrapping) {
    const int effectiveFontSize = fontSize > 0 ? fontSize : 16;

    if (m_hasCache && text == m_cachedText && effectiveFontSize == m_cachedFontSize &&
        fontFamily == m_cachedFontFamily && availableWidth == m_cachedAvailableWidth &&
        wrapping == m_cachedWrapping) {
        return false;
    }

    Clear();

    m_cachedText = text;
    m_cachedFontSize = effectiveFontSize;
    m_cachedFontFamily = fontFamily;
    m_cachedAvailableWidth = availableWidth;
    m_cachedWrapping = wrapping;
    m_hasCache = true;

    if (text.empty()) {
        return true;
    }

    m_flow = BreakText(text, effectiveFontSize, fontFamily, availableWidth, wrapping);
    m_lineTextureIds.reserve(m_flow.lines.size());

    for (auto& line : m_flow.lines) {
        if (line.text.empty()) {
            m_lineTextureIds.push_back(0);
            continue;
        }

        int w = 0;
        int h = 0;
        int inkTop = 0;
        int inkHeight = 0;
        auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(
            line.text, effectiveFontSize, 0, 0, fontFamily, w, h, inkTop, inkHeight);
        line.width = w;
        line.height = h;
        line.inkTop = inkTop;

        const std::uint32_t textureId = (!bitmap.empty() && w > 0 && h > 0)
            ? NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(w), static_cast<unsigned int>(h), bitmap.data())
            : 0;
        m_lineTextureIds.push_back(textureId);
    }

    return true;
}

std::uint32_t TextRasterCache::GetLineTextureId(std::size_t lineIndex) const noexcept {
    return lineIndex < m_lineTextureIds.size() ? m_lineTextureIds[lineIndex] : 0;
}

}
