#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <algorithm>

namespace NeoWPF::UI {

int TextBox::ClampHorizontalScrollOffset(int offset) const noexcept
{
    const int contentWidth = m_renderCache.GetContentWidth();
    const int innerWidth = std::max(0, GetFinalWidth() - GetPadding().Horizontal());
    
    if (contentWidth <= innerWidth) {
        return 0;
    }
    
    return std::max(0, std::min(offset, contentWidth - innerWidth));
}

int TextBox::ClampVerticalScrollOffset(int offset) const noexcept
{
    const int contentHeight = m_renderCache.GetContentHeight();
    const int innerHeight = std::max(0, GetFinalHeight() - GetPadding().Vertical());
    
    if (contentHeight <= innerHeight) {
        return 0;
    }
    
    return std::max(0, std::min(offset, contentHeight - innerHeight));
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
    const int padding = std::max(0, GetPadding().Left);
    const int innerWidth = std::max(0, GetFinalWidth() - GetPadding().Horizontal());
    const int innerHeight = std::max(0, GetFinalHeight() - GetPadding().Vertical());

    int newScrollX = m_scrollOffsetX;
    int newScrollY = m_scrollOffsetY;

    // Horizontal scroll: keep caret visible
    if (caretX - m_scrollOffsetX < 0) {
        newScrollX = std::max(0, caretX - padding);
    } else if (caretX - m_scrollOffsetX + 2 >= innerWidth) {
        newScrollX = std::max(0, caretX + 2 - innerWidth);
    }

    // Vertical scroll: keep caret visible
    if (caretY - m_scrollOffsetY < 0) {
        newScrollY = std::max(0, caretY);
    } else if (caretY - m_scrollOffsetY + caretHeight >= innerHeight) {
        newScrollY = std::max(0, caretY + caretHeight - innerHeight);
    }

    SetScrollOffset(newScrollX, newScrollY);
}

}
