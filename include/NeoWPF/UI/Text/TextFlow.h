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
    double width = 0.0;
    double height = 0.0;
    double lineHeight = 0.0;
    double inkTop = 0.0;
};

struct FlowResult {
    std::vector<FlowLine> lines;
    double contentWidth = 0.0;
    double contentHeight = 0.0;
};

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     double availableWidth, TextWrapping wrapping, int fontWeight,
                     NeoWPF::UI::FontStyle fontStyle);

int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily);
int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int fontWeight, NeoWPF::UI::FontStyle fontStyle);

}
