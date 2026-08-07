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

    if (!m_verticalScrollBar || !m_horizontalScrollBar) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] OnMouseDown: missing scrollbar instances");
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const int contentInset = std::max(0, m_padding);
    const int viewportX = absX + contentInset;
    const int viewportY = absY + contentInset;
    const int viewportWidth = GetViewportWidth();
    const int viewportHeight = GetViewportHeight();
    const int scrollbarThickness = 6;

    const int finalWidth = std::max(0, GetFinalWidth());
    const int finalHeight = std::max(0, GetFinalHeight());
    const int verticalBarX = absX + finalWidth - scrollbarThickness - 2;
    const int verticalBarY = absY + contentInset + 4;
    const int verticalBarHeight = std::min(72, std::max(24, finalHeight - contentInset * 2 - 8));
    const int verticalThumbLength = m_verticalScrollBar->GetThumbLength(verticalBarHeight);
    const int verticalThumbPosition = m_verticalScrollBar->GetThumbPosition(verticalBarHeight, verticalThumbLength);

    if (m_scroll.showVertical && x >= verticalBarX && x < verticalBarX + scrollbarThickness && y >= verticalBarY && y < verticalBarY + verticalBarHeight) {
        const int thumbTop = verticalBarY + verticalThumbPosition;
        const int thumbBottom = thumbTop + verticalThumbLength;
        if (y >= thumbTop && y < thumbBottom) {
            m_drag.draggingVertical = true;
            m_drag.lastPointer = y;
            m_drag.startPointerY = std::clamp(y - verticalBarY, 0, std::max(0, verticalBarHeight - verticalThumbLength));
            m_drag.startScrollY = m_verticalScrollBar->GetValue();
            return;
        }

        const int pointerY = std::clamp(y - verticalBarY - verticalThumbLength / 2, 0, std::max(0, verticalBarHeight - verticalThumbLength));
        const int range = m_verticalScrollBar->GetScrollRange();
        if (range <= 0) {
            m_verticalScrollBar->SetValue(0);
        } else {
            const int availableTrack = std::max(0, verticalBarHeight - verticalThumbLength);
            const int newValue = static_cast<int>((static_cast<long long>(pointerY) * range + availableTrack / 2) / std::max(1, availableTrack));
            m_verticalScrollBar->SetValue(std::clamp(newValue, 0, range));
        }
        InvalidateVisual();
        return;
    }

    const int horizontalBarX = absX + contentInset + 4;
    const int horizontalBarY = absY + finalHeight - scrollbarThickness - 2;
    const int horizontalBarWidth = std::min(96, std::max(24, finalWidth - contentInset * 2 - 8));
    const int horizontalThumbLength = m_horizontalScrollBar->GetThumbLength(horizontalBarWidth);
    const int horizontalThumbPosition = m_horizontalScrollBar->GetThumbPosition(horizontalBarWidth, horizontalThumbLength);

    if (m_scroll.showHorizontal && y >= horizontalBarY && y < horizontalBarY + scrollbarThickness && x >= horizontalBarX && x < horizontalBarX + horizontalBarWidth) {
        const int thumbLeft = horizontalBarX + horizontalThumbPosition;
        const int thumbRight = thumbLeft + horizontalThumbLength;
        if (x >= thumbLeft && x < thumbRight) {
            m_drag.draggingHorizontal = true;
            m_drag.lastPointer = x;
            m_drag.startPointerX = std::clamp(x - horizontalBarX, 0, std::max(0, horizontalBarWidth - horizontalThumbLength));
            m_drag.startScrollX = m_horizontalScrollBar->GetValue();
            return;
        }

        const int pointerX = std::clamp(x - horizontalBarX - horizontalThumbLength / 2, 0, std::max(0, horizontalBarWidth - horizontalThumbLength));
        const int range = m_horizontalScrollBar->GetScrollRange();
        if (range <= 0) {
            m_horizontalScrollBar->SetValue(0);
        } else {
            const int availableTrack = std::max(0, horizontalBarWidth - horizontalThumbLength);
            const int newValue = static_cast<int>((static_cast<long long>(pointerX) * range + availableTrack / 2) / std::max(1, availableTrack));
            m_horizontalScrollBar->SetValue(std::clamp(newValue, 0, range));
        }
        InvalidateVisual();
        return;
    }

    UpdateCaretFromMouse(x, y);
    m_isFocused = true;
    InvalidateVisual();
}

void TextBox::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (!IsEnabled()) {
        return;
    }

    const bool hadDrag = m_drag.draggingVertical || m_drag.draggingHorizontal;
    m_drag.draggingVertical = false;
    m_drag.draggingHorizontal = false;
    m_drag.lastPointer = 0;

    if (hadDrag) {
        InvalidateVisual();
    }
}

void TextBox::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!args || !IsEnabled()) {
        return;
    }

    if (!args->IsLeftButtonDown()) {
        m_drag.draggingVertical = false;
        m_drag.draggingHorizontal = false;
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    if (m_drag.draggingVertical) {
        const int contentInset = m_padding;
        const int verticalBarHeight = std::min(72, std::max(24, GetFinalHeight() - contentInset * 2 - 8));
        const int thumbLength = m_verticalScrollBar->GetThumbLength(verticalBarHeight);
        const int availableTrack = std::max(0, verticalBarHeight - thumbLength);
        const int range = m_verticalScrollBar->GetScrollRange();
        if (range > 0 && availableTrack > 0) {
            const int pointerPos = std::clamp(args->GetY() - (absY + contentInset + 4), 0, availableTrack);
            const int dragDelta = pointerPos - m_drag.startPointerY;
            const int valueDelta = static_cast<int>((static_cast<long long>(dragDelta) * range + availableTrack / 2) / availableTrack);
            m_verticalScrollBar->SetValue(std::clamp(m_drag.startScrollY + valueDelta, 0, range));
        }
        ScrollTo(m_horizontalScrollBar->GetValue(), m_verticalScrollBar->GetValue());
        return;
    }

    if (m_drag.draggingHorizontal) {
        const int contentInset = m_padding;
        const int horizontalBarWidth = std::min(96, std::max(24, GetFinalWidth() - contentInset * 2 - 8));
        const int thumbLength = m_horizontalScrollBar->GetThumbLength(horizontalBarWidth);
        const int availableTrack = std::max(0, horizontalBarWidth - thumbLength);
        const int range = m_horizontalScrollBar->GetScrollRange();
        if (range > 0 && availableTrack > 0) {
            const int pointerPos = std::clamp(args->GetX() - (absX + contentInset + 4), 0, availableTrack);
            const int dragDelta = pointerPos - m_drag.startPointerX;
            const int valueDelta = static_cast<int>((static_cast<long long>(dragDelta) * range + availableTrack / 2) / availableTrack);
            m_horizontalScrollBar->SetValue(std::clamp(m_drag.startScrollX + valueDelta, 0, range));
        }
        ScrollTo(m_horizontalScrollBar->GetValue(), m_verticalScrollBar->GetValue());
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
        case VK_TAB:
            args->SetHandled(true);
            return;
        default:
            break;
        }
    }

    if (changed) {
        if (true) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] OnKeyboardInput: keyCode=%d charCode=%u caret=%zu textLength=%zu", keyCode, static_cast<unsigned int>(charCode), m_caret.position, m_text.size());
        }
        args->SetHandled(true);
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
    SetText(newText);
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
    SetText(newText);
    EnsureCaretVisible();
}

}
