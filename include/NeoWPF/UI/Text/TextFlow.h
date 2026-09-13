/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
#include <NeoWPF/UI/Text/FontStyle.h>
#include <string>
#include <vector>

namespace NeoWPF::UI::Text {

struct FlowLine {
    std::wstring text;
    std::size_t charStart = 0;
    std::size_t charEnd = 0;
    int width = 0;
    int height = 0;
    int lineHeight = 0;
    int inkTop = 0;
};

struct FlowResult {
    std::vector<FlowLine> lines;
    int contentWidth = 0;
    int contentHeight = 0;
};

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int availableWidth, TextWrapping wrapping);
FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int availableWidth, TextWrapping wrapping, int fontWeight,
                     NeoWPF::UI::FontStyle fontStyle);

int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily);
int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int fontWeight, NeoWPF::UI::FontStyle fontStyle);

}
