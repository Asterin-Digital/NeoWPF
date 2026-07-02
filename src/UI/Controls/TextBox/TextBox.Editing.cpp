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
        NeoWPF::Diagnostics::LogInfo("[TextBox] OnMouseDown: x=%d, buttonState=%u", x, buttonState);
    }

    UpdateCaretFromMouse(x);
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
        case VK_HOME:
            m_caretPosition = 0;
            changed = true;
            break;
        case VK_END:
            m_caretPosition = m_text.size();
            changed = true;
            break;
        case VK_RETURN:
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
        m_text.erase(m_caretPosition - 1, 1);
        --m_caretPosition;
    }
    else if (m_caretPosition < m_text.size()) {
        m_text.erase(m_caretPosition, 1);
    }

    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::UpdateCaretFromMouse(int x)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const int localX = x - absX - m_padding;
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateCaretFromMouse: x=%d absX=%d localX=%d textLength=%zu", x, absX, localX, m_text.size());
    }

    if (localX <= 0) {
        m_caretPosition = 0;
        return;
    }

    std::size_t bestPosition = 0;
    int bestDistance = std::numeric_limits<int>::max();
    for (std::size_t i = 0; i <= m_text.size(); ++i) {
        const std::wstring prefix = m_text.substr(0, i);
        const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
        const int distance = std::abs(localX - metrics.width);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestPosition = i;
        }
    }

    m_caretPosition = bestPosition;
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateCaretFromMouse: chosen caret=%zu bestDistance=%d", m_caretPosition, bestDistance);
    }
}

int TextBox::GetCaretX() const
{
    if (m_caretPosition == 0) {
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] GetCaretX: caretPosition=0 -> x=0");
        }
        return 0;
    }

    const std::wstring prefix = m_text.substr(0, m_caretPosition);
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] GetCaretX: caretPosition=%zu x=%d", m_caretPosition, metrics.width);
    }
    return metrics.width;
}

int TextBox::GetCaretHeight() const
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int baselineHeight = std::max(baseMetrics.height, m_fontSize);

    if (m_textHeight > 0) {
        return std::max(baselineHeight, m_textHeight);
    }

    return baselineHeight;
}

}
