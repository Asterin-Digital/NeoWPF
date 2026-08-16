#pragma once

#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
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
};

struct FlowResult {
    std::vector<FlowLine> lines;
    int contentWidth = 0;
    int contentHeight = 0;
};

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int availableWidth, TextWrapping wrapping);

int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily);

}
