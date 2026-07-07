#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <algorithm>

namespace NeoWPF::UI {

void TextBoxScrollBar::Reset() noexcept {
    m_contentWidth = 0;
    m_contentHeight = 0;
    m_viewportWidth = 0;
    m_viewportHeight = 0;
    m_scrollOffsetX = 0;
    m_scrollOffsetY = 0;
    m_maxHorizontalScroll = 0;
    m_maxVerticalScroll = 0;
    m_verticalThumbLength = 0;
    m_verticalThumbPosition = 0;
    m_horizontalThumbLength = 0;
    m_horizontalThumbPosition = 0;
    m_showVerticalScrollbar = false;
    m_showHorizontalScrollbar = false;
    m_dragStartPointer = 0;
    m_dragStartScrollOffset = 0;
}

void TextBoxScrollBar::UpdateMetrics(int contentWidth, int contentHeight, int controlWidth, int controlHeight, int padding) noexcept {
    m_contentWidth = std::max(0, contentWidth);
    m_contentHeight = std::max(0, contentHeight);

    const int baseWidth = std::max(0, controlWidth - (padding * 2));
    const int baseHeight = std::max(0, controlHeight - (padding * 2));
    const bool showVertical = m_contentHeight > baseHeight;
    m_showVerticalScrollbar = showVertical;
    const int viewportWidth = std::max(0, baseWidth - (showVertical ? 12 : 0));
    const int viewportHeight = std::max(0, baseHeight - (m_showHorizontalScrollbar ? 12 : 0));
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;

    m_maxHorizontalScroll = std::max(0, m_contentWidth - m_viewportWidth);
    m_maxVerticalScroll = std::max(0, m_contentHeight - m_viewportHeight);

    if (m_viewportHeight <= 0 || m_contentHeight <= m_viewportHeight) {
        m_verticalThumbLength = std::max(8, m_viewportHeight);
    } else {
        m_verticalThumbLength = std::max(8, (m_viewportHeight * m_viewportHeight) / m_contentHeight);
    }
    if (m_viewportHeight <= 0 || m_maxVerticalScroll <= 0 || m_viewportHeight <= m_verticalThumbLength) {
        m_verticalThumbPosition = 0;
    } else {
        m_verticalThumbPosition = (m_scrollOffsetY * (m_viewportHeight - m_verticalThumbLength)) / m_maxVerticalScroll;
    }

    if (m_viewportWidth <= 0 || m_contentWidth <= m_viewportWidth) {
        m_horizontalThumbLength = std::max(8, m_viewportWidth);
    } else {
        m_horizontalThumbLength = std::max(8, (m_viewportWidth * m_viewportWidth) / m_contentWidth);
    }
    if (m_viewportWidth <= 0 || m_maxHorizontalScroll <= 0 || m_viewportWidth <= m_horizontalThumbLength) {
        m_horizontalThumbPosition = 0;
    } else {
        m_horizontalThumbPosition = (m_scrollOffsetX * (m_viewportWidth - m_horizontalThumbLength)) / m_maxHorizontalScroll;
    }
}

void TextBoxScrollBar::SetScrollOffset(int x, int y) noexcept {
    m_scrollOffsetX = x;
    m_scrollOffsetY = y;
}

void TextBoxScrollBar::ScrollTo(int x, int y) noexcept {
    m_scrollOffsetX = std::clamp(x, 0, m_maxHorizontalScroll);
    m_scrollOffsetY = std::clamp(y, 0, m_maxVerticalScroll);
}

void TextBoxScrollBar::ClampScrollOffsets() noexcept {
    m_scrollOffsetX = std::clamp(m_scrollOffsetX, 0, m_maxHorizontalScroll);
    m_scrollOffsetY = std::clamp(m_scrollOffsetY, 0, m_maxVerticalScroll);
}

void TextBoxScrollBar::UpdateVerticalScrollFromPointer(int pointerY) noexcept {
    const int availableTrack = std::max(0, m_viewportHeight - m_verticalThumbLength);
    if (m_maxVerticalScroll <= 0 || availableTrack <= 0) {
        m_scrollOffsetY = 0;
        return;
    }
    const int newValue = (pointerY * m_maxVerticalScroll) / availableTrack;
    m_scrollOffsetY = std::clamp(newValue, 0, m_maxVerticalScroll);
}

void TextBoxScrollBar::UpdateHorizontalScrollFromPointer(int pointerX) noexcept {
    const int availableTrack = std::max(0, m_viewportWidth - m_horizontalThumbLength);
    if (m_maxHorizontalScroll <= 0 || availableTrack <= 0) {
        m_scrollOffsetX = 0;
        return;
    }
    const int newValue = (pointerX * m_maxHorizontalScroll) / availableTrack;
    m_scrollOffsetX = std::clamp(newValue, 0, m_maxHorizontalScroll);
}

bool TextBoxScrollBar::IsPointOnVerticalThumb(int pointerX, int pointerY, int barX, int barY, int viewportHeight, int scrollbarThickness) const noexcept {
    (void)pointerX;
    const int thumbLength = GetVerticalThumbLength();
    const int thumbPosition = GetVerticalThumbPosition();
    const int thumbTop = barY + thumbPosition;
    const int thumbBottom = thumbTop + thumbLength;
    return pointerY >= thumbTop && pointerY <= thumbBottom && pointerX >= barX && pointerX <= barX + scrollbarThickness;
}

bool TextBoxScrollBar::IsPointOnHorizontalThumb(int pointerX, int pointerY, int barX, int barY, int viewportWidth, int scrollbarThickness) const noexcept {
    (void)viewportWidth;
    const int thumbLength = GetHorizontalThumbLength();
    const int thumbPosition = GetHorizontalThumbPosition();
    const int thumbLeft = barX + thumbPosition;
    const int thumbRight = thumbLeft + thumbLength;
    return pointerX >= thumbLeft && pointerX <= thumbRight && pointerY >= barY && pointerY <= barY + scrollbarThickness;
}

void TextBoxScrollBar::BeginVerticalDrag(int pointerY) noexcept {
    m_dragStartPointer = pointerY;
    m_dragStartScrollOffset = m_scrollOffsetY;
}

void TextBoxScrollBar::BeginHorizontalDrag(int pointerX) noexcept {
    m_dragStartPointer = pointerX;
    m_dragStartScrollOffset = m_scrollOffsetX;
}

void TextBoxScrollBar::DragVerticalBy(int delta) noexcept {
    const int availableTrack = std::max(0, m_viewportHeight - m_verticalThumbLength);
    if (m_maxVerticalScroll <= 0 || availableTrack <= 0) {
        m_scrollOffsetY = 0;
        return;
    }
    const int newValue = m_dragStartScrollOffset + (delta * m_maxVerticalScroll / availableTrack);
    m_scrollOffsetY = std::clamp(newValue, 0, m_maxVerticalScroll);
}

void TextBoxScrollBar::DragHorizontalBy(int delta) noexcept {
    const int availableTrack = std::max(0, m_viewportWidth - m_horizontalThumbLength);
    if (m_maxHorizontalScroll <= 0 || availableTrack <= 0) {
        m_scrollOffsetX = 0;
        return;
    }
    const int newValue = m_dragStartScrollOffset + (delta * m_maxHorizontalScroll / availableTrack);
    m_scrollOffsetX = std::clamp(newValue, 0, m_maxHorizontalScroll);
}

}