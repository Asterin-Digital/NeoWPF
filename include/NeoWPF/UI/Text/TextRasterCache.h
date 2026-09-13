/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Text/TextFlow.h>
#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::UI::Text {

class TextRasterCache {
public:
    ~TextRasterCache();

    bool Refresh(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                 int availableWidth, TextWrapping wrapping,
                 int fontWeight = 400,
                 NeoWPF::UI::FontStyle fontStyle = NeoWPF::UI::FontStyle::Normal);
    void Clear();

    const std::vector<FlowLine>& GetLines() const noexcept { return m_flow.lines; }
    std::uint32_t GetLineTextureId(std::size_t lineIndex) const noexcept;
    int GetContentWidth() const noexcept { return m_flow.contentWidth; }
    int GetContentHeight() const noexcept { return m_flow.contentHeight; }

private:
    FlowResult m_flow;
    std::vector<std::uint32_t> m_lineTextureIds;

    std::wstring m_cachedText;
    int m_cachedFontSize = 0;
    std::wstring m_cachedFontFamily;
    int m_cachedAvailableWidth = 0;
    TextWrapping m_cachedWrapping = TextWrapping::NoWrap;
    int m_cachedFontWeight = 400;
    NeoWPF::UI::FontStyle m_cachedFontStyle = NeoWPF::UI::FontStyle::Normal;
    bool m_hasCache = false;
};

}
