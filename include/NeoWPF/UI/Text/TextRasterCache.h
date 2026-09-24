/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
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
                 double availableWidth, TextWrapping wrapping,
                 int fontWeight = 400,
                 NeoWPF::UI::FontStyle fontStyle = NeoWPF::UI::FontStyle::Normal);
    void Clear();

    const std::vector<FlowLine>& GetLines() const noexcept { return m_flow.lines; }
    std::uint32_t GetLineTextureId(std::size_t lineIndex) const noexcept;
    double GetContentWidth() const noexcept { return m_flow.contentWidth; }
    double GetContentHeight() const noexcept { return m_flow.contentHeight; }

private:
    FlowResult m_flow;
    std::vector<std::uint32_t> m_lineTextureIds;
};

}
