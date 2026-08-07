#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <cwctype>
#include <limits>

namespace NeoWPF::UI {

TextLayout::TextLayout() noexcept {}

namespace {

struct LogicalLine {
    std::wstring text;
    std::size_t startOffset = 0;
};

std::vector<LogicalLine> SplitLines(const std::wstring& text) {
    std::vector<LogicalLine> lines;
    std::size_t lineStart = 0;
    while (lineStart <= text.size()) {
        const std::size_t lineBreak = text.find(L'\n', lineStart);
        const std::size_t lineEnd = lineBreak == std::wstring::npos ? text.size() : lineBreak;
        lines.push_back({text.substr(lineStart, lineEnd - lineStart), lineStart});
        if (lineBreak == std::wstring::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }
    return lines;
}

struct WrappedSegment {
    std::wstring text;
    std::size_t startOffset = 0;
    std::size_t endOffset = 0;
};

bool FitsWithin(const std::wstring& text, int maxWidth, int fontSize, const std::wstring& fontFamily) {
    if (maxWidth <= 0 || text.empty()) {
        return true;
    }
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(text, fontSize, fontFamily, 0, 0);
    return metrics.width <= maxWidth;
}

std::vector<WrappedSegment> WrapLine(const std::wstring& lineText, std::size_t lineStartOffset, int availableWidth, int fontSize, const std::wstring& fontFamily, TextWrapping textWrapping) {
    std::vector<WrappedSegment> result;
    if (lineText.empty() || availableWidth <= 0 || textWrapping == TextWrapping::NoWrap) {
        result.push_back({lineText, lineStartOffset, lineStartOffset + lineText.size()});
        return result;
    }

    std::wstring currentLine;
    std::size_t currentLineStartOffset = lineStartOffset;
    // keep the original offsets around so wrapped lines still map back to the right caret positions
    for (std::size_t i = 0; i < lineText.size(); ++i) {
        const wchar_t ch = lineText[i];
        const std::wstring character(1, ch);
        const std::wstring candidate = currentLine + character;
        const bool fits = FitsWithin(candidate, availableWidth, fontSize, fontFamily);

        if (fits) {
            currentLine = candidate;
            continue;
        }

        if (!currentLine.empty()) {
            result.push_back({currentLine, currentLineStartOffset, lineStartOffset + i});
            currentLine.clear();
            currentLineStartOffset = lineStartOffset + i;
        }

        if (FitsWithin(character, availableWidth, fontSize, fontFamily)) {
            currentLine = character;
        }
        else if (textWrapping == TextWrapping::WrapWithOverflow) {
            result.push_back({character, lineStartOffset + i, lineStartOffset + i + 1});
            currentLineStartOffset = lineStartOffset + i + 1;
        }
        else {
            result.push_back({character, lineStartOffset + i, lineStartOffset + i + 1});
            currentLineStartOffset = lineStartOffset + i + 1;
        }
    }

    if (!currentLine.empty()) {
        result.push_back({currentLine, currentLineStartOffset, lineStartOffset + lineText.size()});
    }

    return result;
}

}

TextLayout::~TextLayout() {
    Clear();
}

void TextLayout::Rebuild(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int availableWidth, int availableHeight, TextWrapping textWrapping) {
    std::wstring effectiveFontFamily = fontFamily;

    if (fontSize <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] TextLayout::Rebuild: invalid fontSize=%d; using fallback", fontSize);
        fontSize = 16;
    }

    if (text == m_text && fontSize == m_fontSize && effectiveFontFamily == m_fontFamily && availableWidth == m_availableWidth && availableHeight == m_availableHeight && textWrapping == m_textWrapping && !m_lines.empty()) {
        // this is the cheap out, otherwise we rebuild the same layout over and over
        return;
    }

    m_text = text;
    m_fontSize = fontSize;
    m_fontFamily = effectiveFontFamily;
    m_availableWidth = availableWidth;
    m_availableHeight = availableHeight;
    m_textWrapping = textWrapping;
    Clear();

    const auto lines = SplitLines(text);
    m_lines.reserve(std::max(lines.size() * 2, static_cast<std::size_t>(1)));
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] TextLayout::Rebuild: textLength=%zu fontSize=%d available=(%d,%d) wrapping=%d logicalLines=%zu",
        text.size(), fontSize, availableWidth, availableHeight, static_cast<int>(textWrapping), lines.size());
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] TextLayout::Rebuild: availableWidth=%d availableHeight=%d textLength=%zu fontSize=%d wrapping=%d",
        availableWidth, availableHeight, text.size(), fontSize, static_cast<int>(textWrapping));

    for (std::size_t logicalLineIndex = 0; logicalLineIndex < lines.size(); ++logicalLineIndex) {
        const auto& logicalLine = lines[logicalLineIndex];
        if (logicalLine.text.empty() && logicalLine.startOffset >= text.size()) {
            m_lines.push_back({std::wstring(), 0, 0, 0, std::max(1, MeasureLineHeight(L"W", m_fontSize, m_fontFamily)), logicalLine.startOffset, logicalLine.startOffset});
            continue;
        }
        const bool shouldWrap = textWrapping != TextWrapping::NoWrap;
        const int lineWidthConstraint = shouldWrap ? std::max(1, m_availableWidth) : 0;
        const auto wrappedLines = shouldWrap ? WrapLine(logicalLine.text, logicalLine.startOffset, lineWidthConstraint, m_fontSize, m_fontFamily, textWrapping) : std::vector<WrappedSegment>{{logicalLine.text, logicalLine.startOffset, logicalLine.startOffset + logicalLine.text.size()}};
        for (const auto& wrappedLine : wrappedLines) {
            const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(wrappedLine.text.empty() ? L"W" : wrappedLine.text, m_fontSize, m_fontFamily, lineWidthConstraint, 0);
            const int lineHeight = MeasureLineHeight(wrappedLine.text, m_fontSize, m_fontFamily);
            const int renderedHeight = std::max(1, lineHeight);

            m_contentHeight += renderedHeight;
            if (textWrapping == TextWrapping::NoWrap) {
                m_contentWidth = std::max(m_contentWidth, std::max(lineMetrics.width, 1));
            }
            else {
                m_contentWidth = std::max(m_contentWidth, lineMetrics.width);
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
                "[TextBox] TextLayout::Rebuild: line textLength=%zu measuredWidth=%d measuredHeight=%d renderedHeight=%d availableWidth=%d availableHeight=%d",
                wrappedLine.text.size(), lineMetrics.width, lineMetrics.height, renderedHeight, availableWidth, availableHeight);

            m_lines.push_back({wrappedLine.text, 0, lineMetrics.width, lineMetrics.height, renderedHeight, wrappedLine.startOffset, wrappedLine.endOffset});
        }
    }
}

void TextLayout::UploadTextures() {
    if (m_text.empty()) {
        return;
    }

    if (m_lines.empty()) {
        return;
    }

    for (auto& line : m_lines) {
        if (line.textureId != 0 || line.text.empty()) {
            continue;
        }

        const int lineWidthConstraint = m_textWrapping != TextWrapping::NoWrap ? std::max(1, m_availableWidth) : 0;
        auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(line.text, m_fontSize, lineWidthConstraint, 0, m_fontFamily, line.width, line.height);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
            "[TextBox] TextLayout::UploadTextures: availableWidth=%d availableHeight=%d textureWidth=%d textureHeight=%d textLength=%zu textureBitmapBytes=%zu",
            m_availableWidth, m_availableHeight, line.width, line.height, line.text.size(), bitmap.size());
        if (!bitmap.empty() && line.width > 0 && line.height > 0) {
            line.textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(line.width), static_cast<unsigned int>(line.height), bitmap.data());
        }
    }
}

void TextLayout::Clear() {
    for (auto& line : m_lines) {
        if (line.textureId != 0) {
            NeoWPF::Rendering::Renderer_DestroyTexture(line.textureId);
            line.textureId = 0;
        }
    }
    m_lines.clear();
    m_contentWidth = 0;
    m_contentHeight = 0;
}

void TextLayout::Refresh(const std::wstring& text, int fontSize, const std::wstring& fontFamily, int availableWidth, int availableHeight, TextWrapping textWrapping, bool createTextures) {
    Rebuild(text, fontSize, fontFamily, availableWidth, availableHeight, textWrapping);
    if (createTextures) {
        UploadTextures();
    }
}

int TextLayout::MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily) {
    if (fontSize <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] TextLayout::MeasureLineHeight: invalid fontSize=%d", fontSize);
        fontSize = 16;
    }
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", fontSize, fontFamily, 0, 0);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(text.empty() ? L"W" : text, fontSize, fontFamily, 0, 0);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, fontSize));
}

int TextLayout::GetLineHeight(std::size_t lineIndex) const {
    if (lineIndex < m_lines.size()) {
        return std::max(1, m_lines[lineIndex].lineHeight);
    }
    return std::max(1, MeasureLineHeight(L"W", m_fontSize, m_fontFamily));
}

int TextLayout::GetLineVisualOffset(std::size_t lineIndex) const {
    if (lineIndex < m_lines.size()) {
        const auto& line = m_lines[lineIndex];
        if (line.height > 0 && line.lineHeight > line.height) {
            return (line.lineHeight - line.height) / 2;
        }
    }
    return 0;
}

std::pair<std::size_t, std::size_t> TextLayout::GetCaretLineColumn(std::size_t caretPosition) const {
    for (std::size_t lineIndex = 0; lineIndex < m_lines.size(); ++lineIndex) {
        const auto& line = m_lines[lineIndex];
        if (caretPosition < line.charStart) {
            continue;
        }

        if (caretPosition < line.charEnd) {
            const std::size_t column = caretPosition - line.charStart;
            return { lineIndex, column };
        }

        if (caretPosition == line.charEnd) {
            if (lineIndex + 1 < m_lines.size()) {
                const auto& nextLine = m_lines[lineIndex + 1];
                if (nextLine.charStart == line.charEnd + 1) {
                    const std::size_t column = std::min(static_cast<std::size_t>(line.text.size()), caretPosition - line.charStart);
                    return { lineIndex, column };
                }
            }
            const std::size_t column = std::min(static_cast<std::size_t>(line.text.size()), caretPosition - line.charStart);
            return { lineIndex, column };
        }
    }

    if (!m_lines.empty()) {
        const auto& lastLine = m_lines.back();
        const std::size_t column = caretPosition > lastLine.charStart ? caretPosition - lastLine.charStart : 0;
        return { m_lines.size() - 1, std::min(column, static_cast<std::size_t>(lastLine.text.size())) };
    }

    return { 0, 0 };
}

int TextLayout::GetCaretY(std::size_t caretPosition) const {
    const auto [lineIndex, _] = GetCaretLineColumn(caretPosition);
    int y = 0;
    for (std::size_t i = 0; i < lineIndex && i < m_lines.size(); ++i) {
        y += std::max(1, m_lines[i].lineHeight);
    }
    return y;
}

int TextLayout::GetCaretHeight(std::size_t caretPosition) const {
    const auto [lineIndex, _] = GetCaretLineColumn(caretPosition);
    if (lineIndex < m_lines.size()) {
        return std::max(1, m_lines[lineIndex].lineHeight);
    }
    return std::max(1, MeasureLineHeight(L"W", std::max(1, m_fontSize), m_fontFamily));
}

}
