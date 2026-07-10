#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

void TextBox::UpdateScrollMetrics()
{
    const bool createTextures = m_textDirty;
    m_textLayout.Refresh(m_text, m_fontSize, m_contentHostWidth, m_contentHostHeight, m_textWrapping, createTextures);
    if (createTextures) {
        m_textDirty = false;
    }

    const int contentWidth = m_textLayout.GetContentWidth();
    const int contentHeight = m_textLayout.GetContentHeight();
    const int baseViewportWidth = std::max(0, m_contentHostWidth);
    const int baseViewportHeight = std::max(0, m_contentHostHeight);
    const int scrollbarThickness = 6;
    const bool supportsVerticalScroll = m_acceptsReturn || m_textWrapping != TextWrapping::NoWrap;
    const bool supportsHorizontalScroll = m_textWrapping == TextWrapping::NoWrap;

    int viewportWidth = baseViewportWidth;
    int viewportHeight = baseViewportHeight;
    bool showHorizontalScrollbar = false;
    bool showVerticalScrollbar = false;

    for (int pass = 0; pass < 2; ++pass) {
        showHorizontalScrollbar = supportsHorizontalScroll && contentWidth > viewportWidth;
        showVerticalScrollbar = supportsVerticalScroll && contentHeight > viewportHeight && m_textLayout.GetLineCount() > 1;
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateScrollMetrics: pass=%d content=(%d,%d) viewport=(%d,%d) showH=%d showV=%d", pass, contentWidth, contentHeight, viewportWidth, viewportHeight, showHorizontalScrollbar ? 1 : 0, showVerticalScrollbar ? 1 : 0);
        }

        const int nextViewportWidth = std::max(0, baseViewportWidth - (showVerticalScrollbar ? scrollbarThickness : 0));
        const int nextViewportHeight = std::max(0, baseViewportHeight - (showHorizontalScrollbar ? scrollbarThickness : 0));
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateScrollMetrics: pass=%d nextViewport=(%d,%d)", pass, nextViewportWidth, nextViewportHeight);
        }
        if (nextViewportWidth == viewportWidth && nextViewportHeight == viewportHeight) {
            break;
        }

        viewportWidth = nextViewportWidth;
        viewportHeight = nextViewportHeight;
    }

    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_showVerticalScrollbar = showVerticalScrollbar && supportsVerticalScroll && contentHeight > m_viewportHeight;
    m_showHorizontalScrollbar = showHorizontalScrollbar && contentWidth > m_viewportWidth;
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] UpdateScrollMetrics: final viewport=(%d,%d) showH=%d showV=%d content=(%d,%d)", m_viewportWidth, m_viewportHeight, m_showHorizontalScrollbar ? 1 : 0, m_showVerticalScrollbar ? 1 : 0, contentWidth, contentHeight);
    }

    m_verticalScrollBar->SetContentSize(contentHeight);
    m_verticalScrollBar->SetViewportSize(m_viewportHeight);
    m_horizontalScrollBar->SetContentSize(contentWidth);
    m_horizontalScrollBar->SetViewportSize(m_viewportWidth);

    ClampScrollOffsets();
}

void TextBox::ClampScrollOffsets()
{
    m_verticalScrollBar->SetValue(m_verticalScrollBar->GetValue());
    m_horizontalScrollBar->SetValue(m_horizontalScrollBar->GetValue());
}

void TextBox::EnsureCaretVisible()
{
    if (m_text.empty()) {
        return;
    }

    const auto [lineIndex, _] = GetCaretLineColumn(m_caretPosition);
    int caretY = 0;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    std::size_t lineStart = 0;
    std::size_t currentLine = 0;
    while (lineStart < m_text.size() && currentLine < lineIndex) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
        const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
        caretY += std::max(baseMetrics.height, std::max(lineMetrics.height, m_fontSize));
        lineStart = lineEnd + 1;
        ++currentLine;
    }

    const int viewportHeight = GetViewportHeight();
    const int caretHeight = std::max(1, GetCaretHeight());
    if (caretY < m_verticalScrollBar->GetValue()) {
        m_verticalScrollBar->SetValue(caretY);
    }
    else if (caretY + caretHeight > m_verticalScrollBar->GetValue() + viewportHeight) {
        m_verticalScrollBar->SetValue(std::max(0, caretY + caretHeight - viewportHeight));
    }

    const int caretX = std::max(0, GetCaretX());
    const int viewportWidth = GetViewportWidth();
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
    m_horizontalScrollBar->SetValue(x);
    m_verticalScrollBar->SetValue(y);
    InvalidateVisual();
}

int TextBox::GetViewportWidth() const
{
    return m_viewportWidth;
}

int TextBox::GetViewportHeight() const
{
    return m_viewportHeight;
}

int TextBox::GetMaxHorizontalScroll() const
{
    return std::max(0, m_textLayout.GetContentWidth() - m_viewportWidth);
}

int TextBox::GetMaxVerticalScroll() const
{
    return std::max(0, m_textLayout.GetContentHeight() - m_viewportHeight);
}

int TextBox::GetVerticalThumbLength() const
{
    if (!m_verticalScrollBar) {
        return 0;
    }
    const int contentHeight = m_textLayout.GetContentHeight();
    if (contentHeight <= 0) {
        return m_viewportHeight;
    }
    return m_verticalScrollBar->GetThumbLength(m_viewportHeight);
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
    const int thumbPosition = (m_verticalScrollBar->GetValue() * m_viewportHeight) / contentHeight;
    return std::max(0, std::min(thumbPosition, m_viewportHeight - GetVerticalThumbLength()));
}

int TextBox::GetHorizontalThumbLength() const
{
    if (!m_horizontalScrollBar) {
        return 0;
    }
    const int contentWidth = m_textLayout.GetContentWidth();
    if (contentWidth <= 0) {
        return m_viewportWidth;
    }
    return m_horizontalScrollBar->GetThumbLength(m_viewportWidth);
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
    const int thumbPosition = (m_horizontalScrollBar->GetValue() * m_viewportWidth) / contentWidth;
    return std::max(0, std::min(thumbPosition, m_viewportWidth - GetHorizontalThumbLength()));
}

void TextBox::UpdateVerticalScrollFromPointer(int pointerY)
{
    if (!m_verticalScrollBar) {
        return;
    }
    const int trackHeight = m_viewportHeight;
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
    const int trackWidth = m_viewportWidth;
    const int contentWidth = m_textLayout.GetContentWidth();
    if (contentWidth <= 0 || trackWidth <= 0) {
        return;
    }
    const int value = (pointerX * contentWidth) / trackWidth;
    m_horizontalScrollBar->SetValue(value);
}

}
