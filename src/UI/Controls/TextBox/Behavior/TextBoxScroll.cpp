#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

void TextBox::UpdateScrollMetrics()
{
    if (m_scroll.contentHostWidth < 0 || m_scroll.contentHostHeight < 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] UpdateScrollMetrics: negative content host size (%d,%d); clamping", m_scroll.contentHostWidth, m_scroll.contentHostHeight);
        m_scroll.contentHostWidth = std::max(0, m_scroll.contentHostWidth);
        m_scroll.contentHostHeight = std::max(0, m_scroll.contentHostHeight);
    }

    const bool createTextures = m_textDirty;
    m_textLayout.Refresh(m_text, m_fontSize, GetFontFamily(), m_scroll.contentHostWidth, m_scroll.contentHostHeight, m_textWrapping, createTextures);
    if (createTextures) {
        m_textDirty = false;
    }

    const int contentWidth = m_textLayout.GetContentWidth();
    const int contentHeight = m_textLayout.GetContentHeight();
    const int baseViewportWidth = std::max(0, m_scroll.contentHostWidth);
    const int baseViewportHeight = std::max(0, m_scroll.contentHostHeight);
    const int scrollbarThickness = 6;
    const bool supportsVerticalScroll = m_acceptsReturn || m_textWrapping != TextWrapping::NoWrap;
    const bool supportsHorizontalScroll = m_textWrapping == TextWrapping::NoWrap;
    const int viewportInset = std::max(0, m_padding * 2);

    int viewportWidth = baseViewportWidth;
    int viewportHeight = baseViewportHeight;
    bool showHorizontalScrollbar = false;
    bool showVerticalScrollbar = false;

    const int overflowTolerance = 4;
    for (int pass = 0; pass < 2; ++pass) {
        showHorizontalScrollbar = supportsHorizontalScroll && contentWidth > viewportWidth - overflowTolerance;
        showVerticalScrollbar = supportsVerticalScroll && contentHeight > viewportHeight - overflowTolerance && m_textLayout.GetLineCount() > 1;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateScrollMetrics: pass=%d content=(%d,%d) viewport=(%d,%d) showH=%d showV=%d", pass, contentWidth, contentHeight, viewportWidth, viewportHeight, showHorizontalScrollbar ? 1 : 0, showVerticalScrollbar ? 1 : 0);

        const int nextViewportWidth = std::max(0, baseViewportWidth - viewportInset - (showVerticalScrollbar ? scrollbarThickness : 0));
        const int nextViewportHeight = std::max(0, baseViewportHeight - viewportInset - (showHorizontalScrollbar ? scrollbarThickness : 0));
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateScrollMetrics: pass=%d nextViewport=(%d,%d)", pass, nextViewportWidth, nextViewportHeight);
        if (nextViewportWidth == viewportWidth && nextViewportHeight == viewportHeight) {
            break;
        }

        viewportWidth = nextViewportWidth;
        viewportHeight = nextViewportHeight;
    }

    m_scroll.viewportWidth = viewportWidth;
    m_scroll.viewportHeight = viewportHeight;
    m_scroll.showVertical = showVerticalScrollbar && supportsVerticalScroll && contentHeight > m_scroll.viewportHeight;
    m_scroll.showHorizontal = showHorizontalScrollbar && contentWidth > m_scroll.viewportWidth;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] UpdateScrollMetrics: final viewport=(%d,%d) showH=%d showV=%d content=(%d,%d)", m_scroll.viewportWidth, m_scroll.viewportHeight, m_scroll.showHorizontal ? 1 : 0, m_scroll.showVertical ? 1 : 0, contentWidth, contentHeight);

    m_verticalScrollBar->SetContentSize(contentHeight);
    m_verticalScrollBar->SetViewportSize(m_scroll.viewportHeight);
    m_horizontalScrollBar->SetContentSize(contentWidth);
    m_horizontalScrollBar->SetViewportSize(m_scroll.viewportWidth);

    ClampScrollOffsets();
}

void TextBox::ClampScrollOffsets()
{
    if (!m_verticalScrollBar || !m_horizontalScrollBar) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] ClampScrollOffsets: missing scrollbar instances");
        return;
    }

    m_verticalScrollBar->SetValue(m_verticalScrollBar->GetValue());
    m_horizontalScrollBar->SetValue(m_horizontalScrollBar->GetValue());
}

void TextBox::EnsureCaretVisible()
{
    if (m_text.empty()) {
        return;
    }

    if (!m_verticalScrollBar || !m_horizontalScrollBar) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] EnsureCaretVisible: missing scrollbar instances");
        return;
    }

    const int caretY = std::max(0, m_textLayout.GetCaretY(m_caret.position));
    const int viewportHeight = std::max(0, GetViewportHeight());
    const int caretHeight = std::max(1, GetCaretHeight());
    if (caretY < m_verticalScrollBar->GetValue()) {
        m_verticalScrollBar->SetValue(caretY);
    }
    else if (caretY + caretHeight > m_verticalScrollBar->GetValue() + viewportHeight) {
        m_verticalScrollBar->SetValue(std::max(0, caretY + caretHeight - viewportHeight));
    }

    const int caretX = std::max(0, GetCaretX());
    const int viewportWidth = std::max(0, GetViewportWidth());
    if (caretX < m_horizontalScrollBar->GetValue()) {
        m_horizontalScrollBar->SetValue(caretX);
    }
    else if (caretX > m_horizontalScrollBar->GetValue() + viewportWidth) {
        m_horizontalScrollBar->SetValue(std::max(0, caretX - viewportWidth + 8));
    }

    ClampScrollOffsets();
}

void TextBox::ScrollTo(int x, int y)
{
    if (!m_horizontalScrollBar || !m_verticalScrollBar) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] ScrollTo: missing scrollbar instances");
        return;
    }

    m_horizontalScrollBar->SetValue(x);
    m_verticalScrollBar->SetValue(y);
    InvalidateVisual();
}

int TextBox::GetViewportWidth() const
{
    return m_scroll.viewportWidth;
}

int TextBox::GetViewportHeight() const
{
    return m_scroll.viewportHeight;
}

int TextBox::GetMaxHorizontalScroll() const
{
    return std::max(0, m_textLayout.GetContentWidth() - m_scroll.viewportWidth);
}

int TextBox::GetMaxVerticalScroll() const
{
    return std::max(0, m_textLayout.GetContentHeight() - m_scroll.viewportHeight);
}

int TextBox::GetVerticalThumbLength() const
{
    if (!m_verticalScrollBar) {
        return 0;
    }
    const int contentHeight = m_textLayout.GetContentHeight();
    if (contentHeight <= 0) {
        return m_scroll.viewportHeight;
    }
    return m_verticalScrollBar->GetThumbLength(m_scroll.viewportHeight);
}

int TextBox::GetVerticalThumbPosition() const
{
    if (!m_verticalScrollBar) {
        return 0;
    }
    const int contentHeight = m_textLayout.GetContentHeight();
    if (contentHeight <= 0) {
        return 0;
    }
    const int thumbPosition = (m_verticalScrollBar->GetValue() * m_scroll.viewportHeight) / contentHeight;
    return std::max(0, std::min(thumbPosition, m_scroll.viewportHeight - GetVerticalThumbLength()));
}

int TextBox::GetHorizontalThumbLength() const
{
    if (!m_horizontalScrollBar) {
        return 0;
    }
    const int contentWidth = m_textLayout.GetContentWidth();
    if (contentWidth <= 0) {
        return m_scroll.viewportWidth;
    }
    return m_horizontalScrollBar->GetThumbLength(m_scroll.viewportWidth);
}

int TextBox::GetHorizontalThumbPosition() const
{
    if (!m_horizontalScrollBar) {
        return 0;
    }
    const int contentWidth = m_textLayout.GetContentWidth();
    if (contentWidth <= 0) {
        return 0;
    }
    const int thumbPosition = (m_horizontalScrollBar->GetValue() * m_scroll.viewportWidth) / contentWidth;
    return std::max(0, std::min(thumbPosition, m_scroll.viewportWidth - GetHorizontalThumbLength()));
}

void TextBox::UpdateVerticalScrollFromPointer(int pointerY)
{
    if (!m_verticalScrollBar) {
        return;
    }
    const int trackHeight = m_scroll.viewportHeight;
    const int contentHeight = m_textLayout.GetContentHeight();
    if (contentHeight <= 0 || trackHeight <= 0) {
        return;
    }
    const int value = (pointerY * contentHeight) / trackHeight;
    m_verticalScrollBar->SetValue(value);
}

void TextBox::UpdateHorizontalScrollFromPointer(int pointerX)
{
    if (!m_horizontalScrollBar) {
        return;
    }
    const int trackWidth = m_scroll.viewportWidth;
    const int contentWidth = m_textLayout.GetContentWidth();
    if (contentWidth <= 0 || trackWidth <= 0) {
        return;
    }
    const int value = (pointerX * contentWidth) / trackWidth;
    m_horizontalScrollBar->SetValue(value);
}

}
