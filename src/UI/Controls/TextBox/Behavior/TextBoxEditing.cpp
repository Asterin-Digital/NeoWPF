#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Input/Keyboard.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

void TextBox::InsertCharacter(wchar_t character)
{
    if (m_isReadOnly) return;

    std::wstring newText = m_text;
    if (m_caret.position > newText.size()) {
        m_caret.position = newText.size();
    }

    newText.insert(m_caret.position, 1, character);
    ++m_caret.position;

    SetText(newText, false);
    EnsureCaretVisible();
}

void TextBox::RemoveCharacter(bool backspace)
{
    if (m_isReadOnly) return;

    if (m_caret.position > m_text.size()) {
        m_caret.position = std::min(m_caret.position, m_text.size());
    }
    if (m_caret.position == 0 && backspace) {
        return;
    }

    std::wstring newText = m_text;
    if (backspace) {
        newText.erase(m_caret.position - 1, 1);
        --m_caret.position;
    }
    else if (m_caret.position < newText.size()) {
        newText.erase(m_caret.position, 1);
    }

    m_caret.position = std::min(m_caret.position, newText.size());
    SetText(newText, false);
    EnsureCaretVisible();
}

void TextBox::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    Content::ContentControl::OnMouseDown(x, y, buttonState);
    if ((buttonState & 0x01) != 0) {
        UpdateCaretFromMouse(x, y);
        m_isFocused = true;
        InvalidateVisual();
    }
}

void TextBox::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    Content::ContentControl::OnMouseUp(x, y, buttonState);
}

void TextBox::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    Content::ContentControl::OnMouseInput(args);
    if (!args) return;

    if (args->GetWheelDelta() != 0) {
        const int scrollDelta = args->GetWheelDelta() > 0 ? -20 : 20;
        SetScrollOffset(m_scrollOffsetX, m_scrollOffsetY + scrollDelta);
        args->SetHandled(true);
    }
}

void TextBox::OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args)
{
    Content::ContentControl::OnKeyboardInput(args);
    if (!args || !args->IsKeyDown()) return;

    const int keyCode = args->GetKeyCode();

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] OnKeyboardInput: keyCode=%d charCode=%c", keyCode, args->GetCharCode());

    switch (keyCode) {
        case static_cast<int>(NeoWPF::Input::Key::Back):
            RemoveCharacter(true);
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::Delete):
            RemoveCharacter(false);
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::Left):
            if (m_caret.position > 0) {
                --m_caret.position;
                InvalidateVisual();
                EnsureCaretVisible();
            }
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::Right):
            if (m_caret.position < m_text.size()) {
                ++m_caret.position;
                InvalidateVisual();
                EnsureCaretVisible();
            }
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::Home):
            MoveCaretToLineColumn(0, 0);
            InvalidateVisual();
            EnsureCaretVisible();
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::End):
            m_caret.position = m_text.size();
            InvalidateVisual();
            EnsureCaretVisible();
            args->SetHandled(true);
            break;

        case static_cast<int>(NeoWPF::Input::Key::Up): {
            const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
            if (lineIndex > 0) {
                MoveCaretToLineColumn(lineIndex - 1, column);
                InvalidateVisual();
                EnsureCaretVisible();
            }
            args->SetHandled(true);
            break;
        }

        case static_cast<int>(NeoWPF::Input::Key::Down): {
            const auto& lines = m_renderCache.GetLines();
            const auto [lineIndex, column] = GetCaretLineColumn(m_caret.position);
            if (lineIndex + 1 < lines.size()) {
                MoveCaretToLineColumn(lineIndex + 1, column);
                InvalidateVisual();
                EnsureCaretVisible();
            }
            args->SetHandled(true);
            break;
        }

        case static_cast<int>(NeoWPF::Input::Key::Enter):
            if (m_acceptsReturn) {
                InsertCharacter(L'\n');
            }
            args->SetHandled(true);
            break;

        default:
            if (args->GetCharCode() != 0 && args->GetCharCode() >= 32) {
                InsertCharacter(args->GetCharCode());
                args->SetHandled(true);
            }
            break;
    }
}

}
