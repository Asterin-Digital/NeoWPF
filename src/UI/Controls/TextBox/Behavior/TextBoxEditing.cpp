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
    if (!IsEnabled()) {
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
    if (!args || !IsEnabled() || !args->IsKeyDown()) {
        return;
    }

    const int keyCode = args->GetKeyCode();
    const wchar_t charCode = args->GetCharCode();
    bool changed = false;

    if (m_isReadOnly && (charCode != 0 || keyCode == VK_BACK || keyCode == VK_DELETE || keyCode == VK_RETURN)) {
        args->SetHandled(true);
        return;
    }

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
    TextChanged.Invoke(m_text);
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
    TextChanged.Invoke(m_text);
}

}
