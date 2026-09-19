#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <algorithm>

namespace NeoWPF::UI {

int TextBox::ClampHorizontalScrollOffset(int offset) const noexcept
{
    const double contentWidth = m_renderCache.GetContentWidth();
    const double innerWidth = GetFinalWidth() - GetPadding().Horizontal();
    const double usableInnerWidth = innerWidth > 0.0 ? innerWidth : 0.0;

    if (contentWidth <= usableInnerWidth) {
        return 0;
    }

    const double maxOffset = contentWidth - usableInnerWidth;
    const int maxScroll = maxOffset > 0.0 ? std::lround(maxOffset) : 0;
    if (offset < 0) {
        return 0;
    }
    if (offset > maxScroll) {
        return maxScroll;
    }
    return offset;
}

int TextBox::ClampVerticalScrollOffset(int offset) const noexcept
{
    const double contentHeight = m_renderCache.GetContentHeight();
    const double innerHeight = GetFinalHeight() - GetPadding().Vertical();
    const double usableInnerHeight = innerHeight > 0.0 ? innerHeight : 0.0;

    if (contentHeight <= usableInnerHeight) {
        return 0;
    }

    const double maxOffset = contentHeight - usableInnerHeight;
    const int maxScroll = maxOffset > 0.0 ? std::lround(maxOffset) : 0;
    if (offset < 0) {
        return 0;
    }
    if (offset > maxScroll) {
        return maxScroll;
    }
    return offset;
}

void TextBox::SetScrollOffset(int x, int y) noexcept
{
    const int clampedX = ClampHorizontalScrollOffset(x);
    const int clampedY = ClampVerticalScrollOffset(y);
    
    if (clampedX == m_scrollOffsetX && clampedY == m_scrollOffsetY) {
        return;
    }
    
    m_scrollOffsetX = clampedX;
    m_scrollOffsetY = clampedY;
    InvalidateVisual();
}

void TextBox::EnsureCaretVisible()
{
    if (m_textDirty) {
        EnsureTextTexture();
    }

    const auto& lines = m_renderCache.GetLines();
    if (lines.empty()) {
        return;
    }

    const auto [lineIndex, _] = GetCaretLineColumn(m_caret.position);
    if (lineIndex >= lines.size()) {
        return;
    }

    const int caretX = GetCaretX();
    const int caretY = GetCaretY();
    const int caretHeight = GetCaretHeight();
    const double padding = GetPadding().Left > 0.0 ? GetPadding().Left : 0.0;
    const double innerWidth = GetFinalWidth() - GetPadding().Horizontal();
    const double innerHeight = GetFinalHeight() - GetPadding().Vertical();
    const double usableInnerWidth = innerWidth > 0.0 ? innerWidth : 0.0;
    const double usableInnerHeight = innerHeight > 0.0 ? innerHeight : 0.0;

    int newScrollX = m_scrollOffsetX;
    int newScrollY = m_scrollOffsetY;

    // Horizontal scroll: keep caret visible
    if (caretX - m_scrollOffsetX < 0) {
        const int targetX = caretX - std::lround(padding);
        newScrollX = targetX > 0 ? targetX : 0;
    } else if (caretX - m_scrollOffsetX + 2 >= std::lround(usableInnerWidth)) {
        const int targetX = std::lround(caretX + 2.0 - usableInnerWidth);
        newScrollX = targetX > 0 ? targetX : 0;
    }

    // Vertical scroll: keep caret visible
    if (caretY - m_scrollOffsetY < 0) {
        newScrollY = caretY > 0 ? caretY : 0;
    } else if (caretY - m_scrollOffsetY + caretHeight >= std::lround(usableInnerHeight)) {
        const int targetY = std::lround(caretY + caretHeight - usableInnerHeight);
        newScrollY = targetY > 0 ? targetY : 0;
    }

    SetScrollOffset(newScrollX, newScrollY);
}

}
