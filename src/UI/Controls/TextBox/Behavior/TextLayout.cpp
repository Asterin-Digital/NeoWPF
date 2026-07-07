#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
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

void TextLayout::Refresh(const std::wstring& text, int fontSize, bool createTextures) {
    if (!createTextures && text == m_text && fontSize == m_fontSize && !m_lines.empty()) {

        return;
    }

    m_text = text;
    m_fontSize = fontSize;
    Clear();

    const auto lines = SplitLines(text);
    for (const auto& lineText : lines) {
        LineRenderData line;
        line.text = lineText;

        const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
        const int lineHeight = MeasureLineHeight(lineText, m_fontSize);
        m_contentHeight += lineHeight;
        m_contentWidth = std::max(m_contentWidth, lineMetrics.width);

        if (createTextures && !lineText.empty()) {
            int width = 0;
            int height = 0;
            auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(lineText, m_fontSize, width, height);
            if (!bitmap.empty() && width > 0 && height > 0) {
                line.textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), bitmap.data());
                line.width = width;
                line.height = height;
            }
        }

        line.lineHeight = line.height > 0 ? line.height : lineHeight;
        m_lines.push_back(line);
    }
}

int TextLayout::MeasureLineHeight(const std::wstring& text, int fontSize) {
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", fontSize, 0, 0);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(text.empty() ? L"W" : text, fontSize, 0, 0);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, fontSize));
}

int TextLayout::GetLineHeight(std::size_t lineIndex) const {
    if (lineIndex < m_lines.size()) {
        return m_lines[lineIndex].lineHeight;
    }
    return MeasureLineHeight(L"W", m_fontSize);
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
