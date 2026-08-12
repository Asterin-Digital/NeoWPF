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

    UpdateCaretFromMouse(x, y);
    m_isFocused = true;
    InvalidateVisual();
}

void TextBox::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    (void)x;
    (void)y;
    (void)buttonState;
}

void TextBox::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!args || !IsEnabled()) {
        return;
    }

    if (!args->IsLeftButtonDown()) {
        return;
    }
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
            if (m_caret.position > 0) {
                --m_caret.position;
                changed = true;
            }
            break;
        case VK_RIGHT:
            if (m_caret.position < m_text.size()) {
                ++m_caret.position;
                changed = true;
            }
            break;
        case VK_UP: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
            if (lineIndex > 0) {
                MoveCaretToLineColumn(lineIndex - 1, column);
                changed = true;
            }
            break;
        }
        case VK_DOWN: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
            MoveCaretToLineColumn(lineIndex + 1, column);
            changed = true;
            break;
        }
        case VK_HOME:
            if (m_text.empty()) {
                m_caret.position = 0;
            }
            else {
                const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
                MoveCaretToLineColumn(lineIndex, 0);
            }
            changed = true;
            break;
        case VK_END: {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
            MoveCaretToLineColumn(lineIndex, std::numeric_limits<std::size_t>::max());
            changed = true;
            break;
        }
        case VK_RETURN:
            if (m_acceptsReturn) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Newline inserted: caret=%zu textLength=%zu", m_caret.position, m_text.size());
                InsertCharacter(L'\n');
                changed = true;
            }
            else {
                args->SetHandled(true);
                return;
            }
            break;
        default:
            break;
        }
    }

    if (changed) {
        if (true) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] OnKeyboardInput: keyCode=%d charCode=%u caret=%zu textLength=%zu", keyCode, static_cast<unsigned int>(charCode), m_caret.position, m_text.size());
        }
        args->SetHandled(true);
        EnsureCaretVisible();
        m_textDirty = true;
        InvalidateVisual();
    }
}

void TextBox::InsertCharacter(wchar_t character)
{
    if (m_caret.position > m_text.size()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] InsertCharacter: caret %zu exceeded text size %zu; clamping", m_caret.position, m_text.size());
        m_caret.position = std::min(m_caret.position, m_text.size());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] InsertCharacter: char=%u caret=%zu", static_cast<unsigned int>(character), m_caret.position);
    std::wstring newText = m_text;
    newText.insert(m_caret.position, 1, character);
    SetText(newText, false);
    ++m_caret.position;
    EnsureCaretVisible();
}

void TextBox::RemoveCharacter(bool backspace)
{
    if (m_caret.position > m_text.size()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] RemoveCharacter: caret %zu exceeded text size %zu; clamping", m_caret.position, m_text.size());
        m_caret.position = std::min(m_caret.position, m_text.size());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] RemoveCharacter: backspace=%d caret=%zu textLength=%zu", backspace ? 1 : 0, m_caret.position, m_text.size());
    if (m_caret.position == 0 && backspace) {
        return;
    }

    std::wstring newText = m_text;
    if (backspace) {
        if (m_caret.position > 0 && newText[m_caret.position - 1] == L'\n') {
            newText.erase(m_caret.position - 1, 1);
            --m_caret.position;
        }
        else {
            newText.erase(m_caret.position - 1, 1);
            --m_caret.position;
        }
    }
    else if (m_caret.position < newText.size()) {
        newText.erase(m_caret.position, 1);
    }

    m_caret.position = std::min(m_caret.position, newText.size());
    SetText(newText, false);
    EnsureCaretVisible();
}

}
