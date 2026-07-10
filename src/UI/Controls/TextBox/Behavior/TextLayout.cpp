#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <cwctype>
#include <limits>

namespace NeoWPF::UI {

namespace {

int MeasureLineHeight(const std::wstring& text, int fontSize) {
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", fontSize, 0, 0);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(text.empty() ? L"W" : text, fontSize, 0, 0);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, fontSize));
}

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

bool FitsWithin(const std::wstring& text, int maxWidth, int fontSize) {
    if (maxWidth <= 0 || text.empty()) {
        return true;
    }
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(text, fontSize, 0, 0);
    return metrics.width <= maxWidth;
}

std::vector<WrappedSegment> WrapLine(const std::wstring& lineText, std::size_t lineStartOffset, int availableWidth, int fontSize, TextWrapping textWrapping) {
    std::vector<WrappedSegment> result;
    if (lineText.empty() || availableWidth <= 0 || textWrapping == TextWrapping::NoWrap) {
        result.push_back({lineText, lineStartOffset, lineStartOffset + lineText.size()});
        return result;
    }

    std::wstring currentLine;
    std::size_t currentLineStartOffset = lineStartOffset;
    for (std::size_t i = 0; i < lineText.size(); ++i) {
        const wchar_t ch = lineText[i];
        const std::wstring character(1, ch);
        const std::wstring candidate = currentLine + character;
        const bool fits = FitsWithin(candidate, availableWidth, fontSize);

        if (fits) {
            currentLine = candidate;
            continue;
        }

        if (!currentLine.empty()) {
            result.push_back({currentLine, currentLineStartOffset, lineStartOffset + i});
            currentLine.clear();
            currentLineStartOffset = lineStartOffset + i;
        }

        if (FitsWithin(character, availableWidth, fontSize)) {
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

void TextLayout::Refresh(const std::wstring& text, int fontSize, int availableWidth, int availableHeight, TextWrapping textWrapping, bool createTextures) {
    if (!createTextures && text == m_text && fontSize == m_fontSize && availableWidth == m_availableWidth && availableHeight == m_availableHeight && textWrapping == m_textWrapping && !m_lines.empty()) {
        return;
    }

    m_text = text;
    m_fontSize = fontSize;
    m_availableWidth = availableWidth;
    m_availableHeight = availableHeight;
    m_textWrapping = textWrapping;
    Clear();

    const auto lines = SplitLines(text);
    if (TextBox::s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::Refresh: textLength=%zu fontSize=%d available=(%d,%d) wrapping=%d logicalLines=%zu createTextures=%d",
            text.size(), fontSize, availableWidth, availableHeight, static_cast<int>(textWrapping), lines.size(), createTextures ? 1 : 0);
    }
    for (std::size_t logicalLineIndex = 0; logicalLineIndex < lines.size(); ++logicalLineIndex) {
        const auto& logicalLine = lines[logicalLineIndex];
        const bool shouldWrap = textWrapping != TextWrapping::NoWrap;
        const int lineWidthConstraint = shouldWrap ? std::max(1, m_availableWidth) : 0;
        const int lineHeightConstraint = 0;
        const auto wrappedLines = shouldWrap ? WrapLine(logicalLine.text, logicalLine.startOffset, lineWidthConstraint, m_fontSize, textWrapping) : std::vector<WrappedSegment>{{logicalLine.text, logicalLine.startOffset, logicalLine.startOffset + logicalLine.text.size()}};
        for (const auto& wrappedLine : wrappedLines) {
            if (TextBox::s_textBoxDebug) {
                NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::Refresh: logicalLine=%zu startOffset=%zu wrappedSegment=(%zu,%zu) text='%ls'",
                    logicalLineIndex, logicalLine.startOffset, wrappedLine.startOffset, wrappedLine.endOffset, wrappedLine.text.c_str());
            }
            LineRenderData line;
            line.text = wrappedLine.text;
            line.charStart = wrappedLine.startOffset;
            line.charEnd = wrappedLine.endOffset;

            const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(wrappedLine.text.empty() ? L"W" : wrappedLine.text, m_fontSize, lineWidthConstraint, lineHeightConstraint);
            const int lineHeight = MeasureLineHeight(wrappedLine.text, m_fontSize);

            if (createTextures && !wrappedLine.text.empty()) {
                int width = 0;
                int height = 0;
                auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(wrappedLine.text, m_fontSize, lineWidthConstraint, lineHeightConstraint, width, height);
                if (!bitmap.empty() && width > 0 && height > 0) {
                    line.textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), bitmap.data());
                    line.width = width;
                    line.height = height;
                }
            }

            const int renderedHeight = std::max(1, line.height > 0 ? line.height : lineHeight);
            line.lineHeight = renderedHeight;
            m_contentHeight += renderedHeight;
            if (textWrapping == TextWrapping::NoWrap) {
                m_contentWidth = std::max(m_contentWidth, std::max(line.width > 0 ? line.width : lineMetrics.width, 1));
            }
            else {
                m_contentWidth = std::max(m_contentWidth, line.width > 0 ? line.width : lineMetrics.width);
            }
            m_lines.push_back(line);
        }
    }
}

int TextLayout::MeasureLineHeight(const std::wstring& text, int fontSize) {
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", fontSize, 0, 0);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(text.empty() ? L"W" : text, fontSize, 0, 0);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, fontSize));
}

int TextLayout::GetLineHeight(std::size_t lineIndex) const {
    if (lineIndex < m_lines.size()) {
        return std::max(1, m_lines[lineIndex].lineHeight);
    }
    return std::max(1, MeasureLineHeight(L"W", m_fontSize));
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
    if (TextBox::s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: caretPosition=%zu lineCount=%zu", caretPosition, m_lines.size());
    }
    for (std::size_t lineIndex = 0; lineIndex < m_lines.size(); ++lineIndex) {
        const auto& line = m_lines[lineIndex];
        if (TextBox::s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: scanning line=%zu charStart=%zu charEnd=%zu text='%ls'", lineIndex, line.charStart, line.charEnd, line.text.c_str());
        }
        if (caretPosition < line.charStart) {
            continue;
        }

        if (caretPosition < line.charEnd) {
            const std::size_t column = caretPosition - line.charStart;
            if (TextBox::s_textBoxDebug) {
                NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: matched inside line=%zu column=%zu", lineIndex, column);
            }
            return { lineIndex, column };
        }

        if (caretPosition == line.charEnd) {
            if (lineIndex + 1 < m_lines.size()) {
                const auto& nextLine = m_lines[lineIndex + 1];
                if (TextBox::s_textBoxDebug) {
                    NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: caret at line end line=%zu nextLineStart=%zu", lineIndex, nextLine.charStart);
                }
                if (nextLine.charStart == line.charEnd + 1) {
                    const std::size_t column = std::min(static_cast<std::size_t>(line.text.size()), caretPosition - line.charStart);
                    if (TextBox::s_textBoxDebug) {
                        NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: adjusting caret on wrapped boundary line=%zu column=%zu", lineIndex, column);
                    }
                    return { lineIndex, column };
                }
            }
            const std::size_t column = std::min(static_cast<std::size_t>(line.text.size()), caretPosition - line.charStart);
            if (TextBox::s_textBoxDebug) {
                NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: caret at line end line=%zu column=%zu", lineIndex, column);
            }
            return { lineIndex, column };
        }
    }

    if (!m_lines.empty()) {
        const auto& lastLine = m_lines.back();
        const std::size_t column = caretPosition > lastLine.charStart ? caretPosition - lastLine.charStart : 0;
        if (TextBox::s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: fallback lastLine=%zu column=%zu", m_lines.size() - 1, std::min(column, static_cast<std::size_t>(lastLine.text.size())));
        }
        return { m_lines.size() - 1, std::min(column, static_cast<std::size_t>(lastLine.text.size())) };
    }

    if (TextBox::s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] TextLayout::GetCaretLineColumn: fallback empty layout");
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
    return std::max(1, MeasureLineHeight(L"W", m_fontSize));
}

}
