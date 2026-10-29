/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
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

    /*
        @summary
        Returns the cached text flow for the supplied layout inputs, computing it when the key changes.

        @internal
    */
    const FlowResult& GetFlow(const std::wstring& text, int fontSize,
                              const std::wstring& fontFamily, double availableWidth,
                              TextWrapping wrapping, int fontWeight = 400,
                              NeoWPF::UI::FontStyle fontStyle = NeoWPF::UI::FontStyle::Normal);

    /*
        @summary
        Refreshes rasterized line textures, reusing flow when its cache key matches.

        @internal
    */
    bool Refresh(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                 double availableWidth, TextWrapping wrapping,
                 int fontWeight = 400,
                 NeoWPF::UI::FontStyle fontStyle = NeoWPF::UI::FontStyle::Normal);

    /*
        @summary
        Releases cached flow and line textures.

        @internal
    */
    void Clear();

    /*
        @summary
        Returns the flow lines in the current text layout.

        @internal
    */
    const std::vector<FlowLine>& GetLines() const noexcept { return m_flow.lines; }
    /*
        @summary
        Returns the texture identifier for a flow line.

        @param lineIndex
        Index of the line.

        @internal
    */
    std::uint32_t GetLineTextureId(std::size_t lineIndex) const noexcept;
    /*
        @summary
        Returns the width of the current text layout.

        @internal
    */
    double GetContentWidth() const noexcept { return m_flow.contentWidth; }
    /*
        @summary
        Returns the height of the current text layout.

        @internal
    */
    double GetContentHeight() const noexcept { return m_flow.contentHeight; }

private:
    /*
        @summary
        Releases textures created for the current flow without discarding the flow cache.

        @internal
    */
    void ClearLineTextures();

    FlowResult m_flow;
    std::vector<std::uint32_t> m_lineTextureIds;
    std::wstring m_cachedText;
    std::wstring m_cachedFontFamily;
    int m_cachedFontSize = 0;
    int m_cachedFontWeight = 0;
    double m_cachedAvailableWidth = -1.0;
    TextWrapping m_cachedWrapping = TextWrapping::NoWrap;
    NeoWPF::UI::FontStyle m_cachedFontStyle = NeoWPF::UI::FontStyle::Normal;
    std::uint64_t m_cachedFontGeneration = 0;
    bool m_flowValid = false;
};

}
