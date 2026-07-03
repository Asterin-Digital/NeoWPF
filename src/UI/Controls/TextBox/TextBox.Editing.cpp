#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Rendering/TextRenderer.h>
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

void TextBox::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (!m_isEnabled) {
        return;
    }

    if (true) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] OnMouseDown: x=%d y=%d buttonState=%u", x, y, buttonState);
    }

    UpdateCaretFromMouse(x, y);
    m_isFocused = true;
    InvalidateVisual();
}

void TextBox::OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args)
{
    if (!args || !m_isEnabled || !args->IsKeyDown()) {
        return;
    }

    const int keyCode = args->GetKeyCode();
    const wchar_t charCode = args->GetCharCode();
    bool changed = false;

    if (charCode != 0 && charCode != L'\r' && charCode != L'\n' && charCode != L'\t' && charCode != 8) {
        InsertCharacter(charCode);
        changed = true;
    }
    else {
        switch (keyCode) {
        case VK_BACK:
            RemoveCharacter(true);
            changed = true;
            break;
        case VK_DELETE:
            RemoveCharacter(false);
            changed = true;
            break;
        case VK_LEFT:
            if (m_caretPosition > 0) {
                --m_caretPosition;
                changed = true;
            }
            break;
        case VK_RIGHT:
            if (m_caretPosition < m_text.size()) {
                ++m_caretPosition;
                changed = true;
            }
            break;
        case VK_UP: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caretPosition);
            if (lineIndex > 0) {
                MoveCaretToLineColumn(lineIndex - 1, column);
                changed = true;
            }
            break;
        }
        case VK_DOWN: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caretPosition);
            MoveCaretToLineColumn(lineIndex + 1, column);
            changed = true;
            break;
        }
        case VK_HOME:
            if (m_text.empty()) {
                m_caretPosition = 0;
            }
            else {
                const auto [lineIndex, column] = GetCaretLineColumn(m_caretPosition);
                MoveCaretToLineColumn(lineIndex, 0);
            }
            changed = true;
            break;
        case VK_END: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caretPosition);
            MoveCaretToLineColumn(lineIndex, std::numeric_limits<std::size_t>::max());
            changed = true;
            break;
        }
        case VK_RETURN:
            if (m_acceptsReturn) {
                if (s_textBoxDebug) {
                    NeoWPF::Diagnostics::LogInfo("[TextBox] Newline inserted: caret=%zu textLength=%zu", m_caretPosition, m_text.size());
                }
                InsertCharacter(L'\n');
                changed = true;
            }
            else {
                args->SetHandled(true);
                return;
            }
            break;
        case VK_TAB:
            args->SetHandled(true);
            return;
        default:
            break;
        }
    }

    if (changed) {
        if (true) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] OnKeyboardInput: keyCode=%d charCode=%u caret=%zu textLength=%zu", keyCode, static_cast<unsigned int>(charCode), m_caretPosition, m_text.size());
        }
        args->SetHandled(true);
        InvalidateVisual();
    }
}

void TextBox::InsertCharacter(wchar_t character)
{
    NeoWPF::Diagnostics::LogInfo("[TextBox] InsertCharacter: char=%u caret=%zu", static_cast<unsigned int>(character), m_caretPosition);
    m_text.insert(m_caretPosition, 1, character);
    ++m_caretPosition;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::RemoveCharacter(bool backspace)
{
    NeoWPF::Diagnostics::LogInfo("[TextBox] RemoveCharacter: backspace=%d caret=%zu textLength=%zu", backspace ? 1 : 0, m_caretPosition, m_text.size());
    if (backspace) {
        if (m_caretPosition == 0) {
            return;
        }

        if (m_caretPosition > 0 && m_text[m_caretPosition - 1] == L'\n') {
            m_text.erase(m_caretPosition - 1, 1);
            --m_caretPosition;
        }
        else {
            m_text.erase(m_caretPosition - 1, 1);
            --m_caretPosition;
        }
    }
    else if (m_caretPosition < m_text.size()) {
        m_text.erase(m_caretPosition, 1);
    }

    m_caretPosition = std::min(m_caretPosition, m_text.size());
    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::UpdateCaretFromMouse(int x, int y)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const int localX = x - absX - m_padding;
    const int localY = y - absY - m_padding;
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateCaretFromMouse: x=%d y=%d absX=%d absY=%d localX=%d localY=%d textLength=%zu", x, y, absX, absY, localX, localY, m_text.size());
    }

    if (localX <= 0) {
        m_caretPosition = 0;
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

    std::size_t bestPosition = lineStart;
    int bestDistance = std::numeric_limits<int>::max();
    const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
    for (std::size_t i = lineStart; i <= lineEnd; ++i) {
        const std::wstring prefix = m_text.substr(lineStart, i - lineStart);
        const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
        const int distance = std::abs(localX - metrics.width);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestPosition = i;
        }
    }

    m_caretPosition = bestPosition;
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateCaretFromMouse: chosen caret=%zu bestDistance=%d targetLine=%zu", m_caretPosition, bestDistance, targetLine);
    }
}

int TextBox::GetCaretX() const
{
    const auto [lineIndex, column] = GetCaretLineColumn(m_caretPosition);
    (void)lineIndex;
    const std::size_t lineStart = [&]() -> std::size_t {
        std::size_t start = 0;
        std::size_t currentLine = 0;
        while (start < m_text.size()) {
            const std::size_t nextNewline = m_text.find(L'\n', start);
            const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
            if (currentLine == lineIndex) {
                return start;
            }
            start = lineEnd + 1;
            ++currentLine;
        }
        return m_text.size();
    }();
    const std::wstring prefix = m_text.substr(lineStart, column);
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] GetCaretX: caretPosition=%zu x=%d", m_caretPosition, metrics.width);
    }
    return metrics.width;
}

int TextBox::GetCaretY() const
{
    const auto [lineIndex, _] = GetCaretLineColumn(m_caretPosition);
    if (!m_lineRenderData.empty()) {
        int y = 0;
        const std::size_t lineCount = std::min<std::size_t>(lineIndex, m_lineRenderData.size());
        for (std::size_t i = 0; i < lineCount; ++i) {
            y += std::max(1, m_lineRenderData[i].lineHeight);
        }
        return y;
    }

    std::size_t lineStart = 0;
    int y = 0;
    std::size_t currentLine = 0;
    while (lineStart < m_text.size() && currentLine < lineIndex) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
        y += GetLineHeight(lineText);
        lineStart = lineEnd + 1;
        ++currentLine;
    }
    return y;
}

int TextBox::GetCaretHeight() const
{
    const auto [lineIndex, _] = GetCaretLineColumn(m_caretPosition);
    if (!m_lineRenderData.empty() && lineIndex < m_lineRenderData.size()) {
        return std::max(1, m_lineRenderData[lineIndex].lineHeight);
    }

    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int baselineHeight = std::max(baseMetrics.height, m_fontSize);

    std::size_t lineStart = 0;
    std::size_t currentLine = 0;
    while (lineStart < m_text.size()) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        if (currentLine == lineIndex) {
            const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
            const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
            return std::max(baselineHeight, std::max(lineMetrics.height, m_fontSize));
        }
        lineStart = lineEnd + 1;
        ++currentLine;
    }

    return baselineHeight;
}

std::pair<std::size_t, std::size_t> TextBox::GetCaretLineColumn(std::size_t caretPosition) const
{
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

void TextBox::MoveCaretToLineColumn(std::size_t lineIndex, std::size_t column)
{
    std::size_t currentLine = 0;
    std::size_t lineStart = 0;
    while (lineStart <= m_text.size()) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        if (currentLine == lineIndex) {
            const std::size_t maxColumn = lineEnd - lineStart;
            m_caretPosition = std::min(lineStart + std::min(column, maxColumn), m_text.size());
            return;
        }
        lineStart = lineEnd + 1;
        ++currentLine;
    }

    m_caretPosition = m_text.size();
}

int TextBox::GetLineHeight(const std::wstring& line) const
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(line.empty() ? L"W" : line, m_fontSize, 0, 0);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, m_fontSize));
}

}
