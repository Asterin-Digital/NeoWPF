#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <cmath>
#include <limits>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace NeoWPF::UI {

void TextBox::UpdateCaretFromMouse(int x, int y)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const int localX = x - absX - std::max(0, GetPadding().Left);
    const int localY = y - absY - std::max(0, GetPadding().Top);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateCaretFromMouse: x=%d y=%d absX=%d absY=%d localX=%d localY=%d textLength=%zu", x, y, absX, absY, localX, localY, m_text.size());

    if (localX <= 0) {
        m_caret.position = 0;
        return;
    }

    std::size_t lineStart = 0;
    std::size_t lineEnd = 0;
    int accumulatedY = 0;
    std::size_t targetLine = 0;
    while (lineStart < m_text.size()) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
        const int lineHeight = GetLineHeight(lineText);
        if (localY < accumulatedY + lineHeight) {
            break;
        }
        accumulatedY += lineHeight;
        lineStart = lineEnd + 1;
        ++targetLine;
    }

    if (lineStart > m_text.size()) {
        lineStart = m_text.size();
    }
    if (lineEnd > m_text.size()) {
        lineEnd = m_text.size();
    }

    if (lineStart == m_text.size()) {
        m_caret.position = m_text.size();
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateCaretFromMouse: chosen caret=%zu bestDistance=%d targetLine=%zu", m_caret.position, 0, targetLine);
        return;
    }

    if (lineEnd < lineStart) {
        m_caret.position = std::min(m_text.size(), lineStart);
        return;
    }

    std::size_t bestPosition = lineStart;
    int bestDistance = std::numeric_limits<int>::max();
    for (std::size_t i = lineStart; i <= lineEnd; ++i) {
        const std::wstring prefix = m_text.substr(lineStart, i - lineStart);
        int width = 0, height = 0;
        NeoWPF::Rendering::Text::RenderTextBitmap(prefix, m_fontSize, GetFontFamily(), width, height);
        const int distance = std::abs(localX - width);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestPosition = i;
        }
    }

    m_caret.position = bestPosition;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateCaretFromMouse: chosen caret=%zu bestDistance=%d targetLine=%zu", m_caret.position, bestDistance, targetLine);
}

int TextBox::GetCaretX() const
{
    const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
    const auto& lines = m_textLayout.GetLines();
    if (lineIndex >= lines.size()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] GetCaretX: invalid lineIndex=%zu lineCount=%zu caret=%zu", lineIndex, lines.size(), m_caret.position);
        return 0;
    }

    if (m_caret.position > m_text.size()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] GetCaretX: caret=%zu exceeds textLength=%zu", m_caret.position, m_text.size());
        return 0;
    }

    const std::wstring lineText = lines[lineIndex].text;
    const std::size_t safeColumn = std::min<std::size_t>(column, lineText.size());
    const std::wstring prefix = lineText.substr(0, safeColumn);
    if (prefix.empty()) return 0;

    int width = 0, height = 0;
    try {
        NeoWPF::Rendering::Text::RenderTextBitmap(prefix, m_fontSize, GetFontFamily(), width, height);
    } catch (const std::exception& ex) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] GetCaretX: render failed for prefix '%ls' (%s)", prefix.c_str(), ex.what());
        return 0;
    }
    return width;
}

int TextBox::GetCaretY() const
{
    return m_textLayout.GetCaretY(m_caret.position);
}

int TextBox::GetCaretHeight() const
{
    return m_textLayout.GetCaretHeight(m_caret.position);
}

std::pair<std::size_t, std::size_t> TextBox::GetCaretLineColumn(std::size_t caretPosition) const
{
    return m_textLayout.GetCaretLineColumn(caretPosition);
}

void TextBox::MoveCaretToLineColumn(std::size_t lineIndex, std::size_t column)
{
    if (lineIndex == 0 && column == 0) {
        m_caret.position = 0;
        return;
    }

    std::size_t currentLine = 0;
    std::size_t lineStart = 0;
    while (lineStart <= m_text.size()) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        if (currentLine == lineIndex) {
            const std::size_t maxColumn = lineEnd - lineStart;
            m_caret.position = std::min(lineStart + std::min(column, maxColumn), m_text.size());
            return;
        }
        lineStart = lineEnd + 1;
        ++currentLine;
    }

    m_caret.position = m_text.size();
}

int TextBox::GetLineHeight(const std::wstring& line) const
{
    return TextLayout::MeasureLineHeight(line, m_fontSize, GetFontFamily());
}

}
