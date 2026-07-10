#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
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

std::vector<std::wstring> SplitLines(const std::wstring& text) {
    std::vector<std::wstring> lines;
    std::size_t lineStart = 0;
    while (lineStart <= text.size()) {
        const std::size_t lineBreak = text.find(L'\n', lineStart);
        const std::size_t lineEnd = lineBreak == std::wstring::npos ? text.size() : lineBreak;
        lines.emplace_back(text.substr(lineStart, lineEnd - lineStart));
        if (lineBreak == std::wstring::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }
    return lines;
}

bool FitsWithin(const std::wstring& text, int maxWidth, int fontSize) {
    if (maxWidth <= 0 || text.empty()) {
        return true;
    }
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(text, fontSize, 0, 0);
    return metrics.width <= maxWidth;
}

std::vector<std::wstring> WrapLine(const std::wstring& lineText, int availableWidth, int fontSize, TextWrapping textWrapping) {
    std::vector<std::wstring> result;
    if (lineText.empty() || availableWidth <= 0 || textWrapping == TextWrapping::NoWrap) {
        result.push_back(lineText);
        return result;
    }

    std::wstring currentLine;
    std::size_t index = 0;
    while (index < lineText.size()) {
        const std::size_t tokenStart = index;
        while (index < lineText.size() && std::iswspace(static_cast<wint_t>(lineText[index]))) {
            ++index;
        }
        const std::size_t tokenEnd = index;
        if (tokenStart != tokenEnd) {
            if (!currentLine.empty()) {
                result.push_back(currentLine);
                currentLine.clear();
            }
            continue;
        }

        std::size_t wordEnd = index;
        while (wordEnd < lineText.size() && !std::iswspace(static_cast<wint_t>(lineText[wordEnd]))) {
            ++wordEnd;
        }

        std::wstring word = lineText.substr(index, wordEnd - index);
        const std::wstring candidate = currentLine.empty() ? word : currentLine + L" " + word;
        if (!currentLine.empty() && !FitsWithin(candidate, availableWidth, fontSize)) {
            result.push_back(currentLine);
            currentLine.clear();
        }

        if (currentLine.empty()) {
            if (FitsWithin(word, availableWidth, fontSize)) {
                currentLine = word;
            }
            else if (textWrapping == TextWrapping::Wrap) {
                for (std::size_t charIndex = 0; charIndex < word.size(); ++charIndex) {
                    const std::wstring character(1, word[charIndex]);
                    if (!currentLine.empty() && !FitsWithin(currentLine + character, availableWidth, fontSize)) {
                        result.push_back(currentLine);
                        currentLine.clear();
                    }
                    currentLine += character;
                }
            }
            else {
                result.push_back(word);
            }
        }
        else if (FitsWithin(candidate, availableWidth, fontSize)) {
            currentLine = candidate;
        }
        else {
            result.push_back(currentLine);
            currentLine = word;
        }

        index = wordEnd;
    }

    if (!currentLine.empty()) {
        result.push_back(currentLine);
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
    for (const auto& lineText : lines) {
        const bool shouldWrap = textWrapping != TextWrapping::NoWrap;
        const int lineWidthConstraint = shouldWrap ? std::max(1, m_availableWidth) : 0;
        const int lineHeightConstraint = 0;

        const auto wrappedLines = shouldWrap ? WrapLine(lineText, lineWidthConstraint, m_fontSize, textWrapping) : std::vector<std::wstring>{lineText};
        for (const auto& wrappedLineText : wrappedLines) {
            LineRenderData line;
            line.text = wrappedLineText;

            const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(wrappedLineText.empty() ? L"W" : wrappedLineText, m_fontSize, lineWidthConstraint, lineHeightConstraint);
            const int lineHeight = MeasureLineHeight(wrappedLineText, m_fontSize);

            if (createTextures && !wrappedLineText.empty()) {
                int width = 0;
                int height = 0;
                auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(wrappedLineText, m_fontSize, lineWidthConstraint, lineHeightConstraint, width, height);
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
    std::size_t lineIndex = 0;
    std::size_t column = 0;
    for (std::size_t i = 0; i < m_text.size() && i < caretPosition; ++i) {
        if (m_text[i] == L'\n') {
            ++lineIndex;
            column = 0;
        }
        else {
            ++column;
        }
    }
    return { lineIndex, column };
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
