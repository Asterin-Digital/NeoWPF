#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Input/Keyboard.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
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

void TextBox::OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args) {
        return;
    }

    if (auto* pointerArgs = dynamic_cast<NeoWPF::Input::Core::RoutedPointerEventArgs*>(args)) {
        if (args->GetRoutedEvent() == &NeoWPF::Input::Core::PointerPressedEvent() &&
            pointerArgs->GetChangedButton() == 0x01u) {
            UpdateCaretFromMouse(static_cast<int>(pointerArgs->GetX()),
                                 static_cast<int>(pointerArgs->GetY()));
            m_isFocused = true;
            InvalidateVisual();
        } else if (args->GetRoutedEvent() == &NeoWPF::Input::Core::PointerWheelEvent() &&
                   pointerArgs->GetWheelDelta() != 0.0) {
            const int scrollDelta = pointerArgs->GetWheelDelta() > 0.0 ? -20 : 20;
            SetScrollOffset(m_scrollOffsetX, m_scrollOffsetY + scrollDelta);
            args->SetHandled(true);
        }
        return;
    }

    if (args->GetRoutedEvent() != &NeoWPF::Input::Core::KeyDownEvent()) {
        return;
    }
    auto* keyboardArgs = dynamic_cast<NeoWPF::Input::Core::RoutedKeyboardEventArgs*>(args);
    if (!keyboardArgs || !keyboardArgs->IsKeyDown()) {
        return;
    }

    const int keyCode = keyboardArgs->GetKeyCode();

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] KeyDown: keyCode=%d charCode=%c", keyCode, keyboardArgs->GetCharCode());

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
            if (keyboardArgs->GetCharCode() != 0 && keyboardArgs->GetCharCode() >= 32) {
                InsertCharacter(keyboardArgs->GetCharCode());
                args->SetHandled(true);
            }
            break;
    }
}

}
