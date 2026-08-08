#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <algorithm>

namespace NeoWPF::UI {

int TextBox::ClampHorizontalScrollOffset(int offset) const noexcept
{
    if (GetFinalWidth() <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
            "[TextBox] ClampHorizontalScrollOffset: invalid final width=%d", GetFinalWidth());
        return 0;
    }

    const int contentHostWidth = std::max(0, GetFinalWidth() - (GetPadding() * 2));
    if (contentHostWidth <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
            "[TextBox] ClampHorizontalScrollOffset: content host width=%d too small", contentHostWidth);
        return 0;
    }

    const int maxOffset = std::max(0, m_textLayout.GetContentWidth() - contentHostWidth);
    const int clamped = std::clamp(offset, 0, maxOffset);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
        "[TextBox] ClampHorizontalScrollOffset: requested=%d clamped=%d maxOffset=%d contentHostWidth=%d", offset, clamped, maxOffset, contentHostWidth);
    return clamped;
}

int TextBox::ClampVerticalScrollOffset(int offset) const noexcept
{
    if (GetFinalHeight() <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
            "[TextBox] ClampVerticalScrollOffset: invalid final height=%d", GetFinalHeight());
        return 0;
    }

    const int contentHostHeight = std::max(0, GetFinalHeight() - (GetPadding() * 2));
    if (contentHostHeight <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
            "[TextBox] ClampVerticalScrollOffset: content host height=%d too small", contentHostHeight);
        return 0;
    }

    const int maxOffset = std::max(0, m_textLayout.GetContentHeight() - contentHostHeight);
    const int clamped = std::clamp(offset, 0, maxOffset);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
        "[TextBox] ClampVerticalScrollOffset: requested=%d clamped=%d maxOffset=%d contentHostHeight=%d", offset, clamped, maxOffset, contentHostHeight);
    return clamped;
}

void TextBox::SetScrollOffset(int x, int y) noexcept
{
    const int clampedX = ClampHorizontalScrollOffset(x);
    const int clampedY = ClampVerticalScrollOffset(y);
    if (clampedX == m_scrollOffsetX && clampedY == m_scrollOffsetY) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug,
            "[TextBox] SetScrollOffset: no change x=%d y=%d", clampedX, clampedY);
        return;
    }

    if (clampedX < 0 || clampedY < 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
            "[TextBox] SetScrollOffset: invalid clamped values x=%d y=%d", clampedX, clampedY);
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] SetScrollOffset: from=(%d,%d) to=(%d,%d)", m_scrollOffsetX, m_scrollOffsetY, clampedX, clampedY);
    m_scrollOffsetX = clampedX;
    m_scrollOffsetY = clampedY;
    InvalidateVisual();
}

void TextBox::EnsureCaretVisible()
{
    if (m_caretScrollBehavior == CaretScrollBehavior::Disabled) {
        return;
    }

    const int contentHostWidth = std::max(0, GetFinalWidth() - (GetPadding() * 2));
    const int contentHostHeight = std::max(0, GetFinalHeight() - (GetPadding() * 2));
    if (contentHostWidth <= 0 || contentHostHeight <= 0) {
        return;
    }

    const int contentWidth = std::max(0, m_textLayout.GetContentWidth());
    const int contentHeight = std::max(0, m_textLayout.GetContentHeight());
    if (contentWidth <= 0 || contentHeight <= 0) {
        return;
    }

    int textOffsetX = 0;
    int textOffsetY = 0;
    TextAlignment::ApplyTextAlignment(
        contentWidth,
        contentHeight,
        contentHostWidth,
        contentHostHeight,
        m_textHorizontalAlignment,
        m_textVerticalAlignment,
        textOffsetX,
        textOffsetY);

    int caretX = textOffsetX + GetCaretX();
    int caretY = textOffsetY + GetCaretY();
    const int caretHeight = GetCaretHeight();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info,
        "[TextBox] EnsureCaretVisible: caret=(%d,%d) height=%d scroll=(%d,%d) host=(%d,%d)",
        caretX, caretY, caretHeight, m_scrollOffsetX, m_scrollOffsetY, contentHostWidth, contentHostHeight);

    int desiredX = m_scrollOffsetX;
    if (caretX < m_scrollOffsetX) {
        desiredX = caretX;
    }
    else if (caretX > m_scrollOffsetX + contentHostWidth) {
        desiredX = caretX - contentHostWidth;
    }

    int desiredY = m_scrollOffsetY;
    if (caretY < m_scrollOffsetY) {
        desiredY = caretY;
    }
    else if (caretY + caretHeight > m_scrollOffsetY + contentHostHeight) {
        desiredY = caretY + caretHeight - contentHostHeight;
    }

    SetScrollOffset(desiredX, desiredY);
}

}
