#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Text/TextFlow.h>
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

namespace {

std::pair<std::size_t, std::size_t> ResolveCaretLineColumn(
    const std::vector<NeoWPF::UI::Text::FlowLine>& lines, std::size_t caretPosition)
{
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        if (caretPosition < line.charStart) continue;
        if (caretPosition <= line.charEnd) {
            const std::size_t column = caretPosition - line.charStart;
            return { i, std::min(column, line.text.size()) };
        }
    }
    if (!lines.empty()) {
        const auto& last = lines.back();
        const std::size_t column = caretPosition > last.charStart ? caretPosition - last.charStart : 0;
        return { lines.size() - 1, std::min(column, last.text.size()) };
    }
    return { 0, 0 };
}

int ResolveCaretY(const std::vector<NeoWPF::UI::Text::FlowLine>& lines, std::size_t lineIndex)
{
    int y = 0;
    for (std::size_t i = 0; i < lineIndex && i < lines.size(); ++i) {
        y += std::max(1, lines[i].lineHeight);
    }
    return y;
}

int ResolveLineVisualOffset(const NeoWPF::UI::Text::FlowLine& line)
{
    if (line.height > 0 && line.lineHeight > line.height) {
        return (line.lineHeight - line.height) / 2;
    }
    return 0;
}

} // namespace

void TextBox::UpdateCaretFromMouse(int x, int y)
{
    int absX = 0, absY = 0;
    GetAbsolutePosition(absX, absY);
    const int localX = x - absX - std::max(0, GetPadding().Left);
    const int localY = y - absY - std::max(0, GetPadding().Top);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] UpdateCaretFromMouse: x=%d y=%d localX=%d localY=%d", x, y, localX, localY);

    const auto& lines = m_renderCache.GetLines();
    if (lines.empty()) {
        m_caret.position = 0;
        return;
    }

    std::size_t targetLine = 0;
    int accumulatedY = 0;
    for (; targetLine + 1 < lines.size(); ++targetLine) {
        const int lineHeight = std::max(1, lines[targetLine].lineHeight);
        if (localY < accumulatedY + lineHeight) break;
        accumulatedY += lineHeight;
    }

    const auto& line = lines[targetLine];
    if (localX <= 0) {
        m_caret.position = line.charStart;
        return;
    }

    std::size_t bestPosition = line.charStart;
    int bestDistance = std::numeric_limits<int>::max();
    for (std::size_t i = 0; i <= line.text.size(); ++i) {
        const auto metrics = NeoWPF::Rendering::Text::MeasureText(
            line.text.substr(0, i), m_fontSize, GetFontFamily(), 0, 0);
        const int distance = std::abs(localX - metrics.width);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestPosition = line.charStart + i;
        }
    }

    m_caret.position = bestPosition;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] UpdateCaretFromMouse: chosen caret=%zu line=%zu", m_caret.position, targetLine);
}

int TextBox::GetCaretX() const
{
    const auto& lines = m_renderCache.GetLines();
    const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
    if (lineIndex >= lines.size()) return 0;

    const std::wstring& lineText = lines[lineIndex].text;
    const std::size_t safeColumn = std::min<std::size_t>(column, lineText.size());
    if (safeColumn == 0) return 0;

    const auto metrics = NeoWPF::Rendering::Text::MeasureText(
        lineText.substr(0, safeColumn), m_fontSize, GetFontFamily(), 0, 0);
    return metrics.width;
}

int TextBox::GetCaretY() const
{
    const auto [lineIndex, _] = GetCaretLineColumn(m_caret.position);
    return ResolveCaretY(m_renderCache.GetLines(), lineIndex);
}

int TextBox::GetCaretHeight() const
{
    const auto& lines = m_renderCache.GetLines();
    const auto [lineIndex, _] = GetCaretLineColumn(m_caret.position);
    if (lineIndex < lines.size()) {
        return std::max(1, lines[lineIndex].lineHeight);
    }
    return std::max(1, NeoWPF::UI::Text::MeasureLineHeight(L"W", m_fontSize, GetFontFamily()));
}

std::pair<std::size_t, std::size_t> TextBox::GetCaretLineColumn(std::size_t caretPosition) const
{
    return ResolveCaretLineColumn(m_renderCache.GetLines(), caretPosition);
}

int TextBox::GetLineVisualOffset(std::size_t lineIndex) const
{
    const auto& lines = m_renderCache.GetLines();
    if (lineIndex < lines.size()) {
        return ResolveLineVisualOffset(lines[lineIndex]);
    }
    return 0;
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
    return NeoWPF::UI::Text::MeasureLineHeight(line, m_fontSize, GetFontFamily());
}

}
