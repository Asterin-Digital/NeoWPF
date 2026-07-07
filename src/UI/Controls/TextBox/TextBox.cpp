#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Input/Core/Events.h>
#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace {
    uint32_t GetCaretTextureId() {
        static uint32_t s_caretTextureId = 0;
        if (s_caretTextureId == 0) {
            const uint32_t whitePixel = 0xFFFFFFFFu;
            s_caretTextureId = NeoWPF::Rendering::Renderer_CreateTexture(1, 1, &whitePixel);
        }
        return s_caretTextureId;
    }
}

bool NeoWPF::UI::TextBox::s_textBoxDebug = true;

namespace NeoWPF::UI {

TextBox::TextBox()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);

    m_verticalScrollBar = std::make_shared<ScrollBar>();
    m_verticalScrollBar->SetOrientation(ScrollBarOrientation::Vertical);
    m_verticalScrollBar->SetThumbSize(24);

    m_horizontalScrollBar = std::make_shared<ScrollBar>();
    m_horizontalScrollBar->SetOrientation(ScrollBarOrientation::Horizontal);
    m_horizontalScrollBar->SetThumbSize(24);

    if (auto style = Styling::StyleManager::GetDefault(typeid(TextBox))) {
        style->Apply(this);
    }

    InitializeVisualStates();
    ApplyVisualState("Normal", false);
}

TextBox::~TextBox()
{
}

void TextBox::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] SetText: oldLength=%zu, newLength=%zu", m_text.size(), text.size());
    }

    m_text = text;
    m_caretPosition = std::min(m_caretPosition, m_text.size());
    m_verticalScrollBar->SetValue(0);
    m_horizontalScrollBar->SetValue(0);
    m_textDirty = true;
    InvalidateVisual();
    TextChanged.Invoke(m_text);
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int minWidth = 120;
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (m_padding * 2);
    const int desiredWidth = std::max(minWidth, availableWidth > 0 ? std::min(minWidth, availableWidth) : minWidth);
    const int desiredHeight = std::max(minHeight, availableHeight > 0 ? std::min(minHeight, availableHeight) : minHeight);

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());
    }

    SetDesiredSize(desiredWidth, desiredHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateScrollMetrics();
    ClampScrollOffsets();
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Arrange: final=%dx%d, textDirty=%d, lineCount=%zu", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_textLayout.GetLineCount());
    }
    if (m_textDirty) {
        EnsureTextTexture();
    }
    EnsureCaretVisible();
}

void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    auto border = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Border"));
    if (border) {
        border->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        border->SetOpacity(GetOpacity());
        border->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            m_backgroundColor,
            GetOpacity());

        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            1.0f,
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY + GetFinalHeight() - 1),
            static_cast<float>(GetFinalWidth()),
            1.0f,
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX + GetFinalWidth() - 1),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const int inset = std::max(0, m_padding);
        contentHost->SetRect(absX + inset, absY + inset, std::max(0, GetFinalWidth() - (inset * 2)), std::max(0, GetFinalHeight() - (inset * 2)));
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    const int scrollbarThickness = 6;
    const int contentInset = m_padding;
    const int viewportWidth = GetViewportWidth();
    const int viewportHeight = GetViewportHeight();
    const int contentX = absX + contentInset;
    const int contentY = absY + contentInset;
    const auto clip = NeoWPF::Rendering::ClipRect{ static_cast<float>(contentX), static_cast<float>(contentY), static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), true };

    int currentY = -m_verticalScrollBar->GetValue();
    const auto& lines = m_textLayout.GetLines();
    for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const auto& line = lines[lineIndex];
        if (line.textureId == 0 || line.width <= 0 || line.height <= 0) {
            currentY += std::max(1, line.lineHeight);
            continue;
        }

        const int drawWidth = std::min(line.width, viewportWidth);
        const float uvRight = drawWidth > 0 ? static_cast<float>(drawWidth) / static_cast<float>(line.width) : 1.0f;
        const int drawHeight = std::max(1, line.height);
        const float uvBottom = drawHeight > 0 ? static_cast<float>(drawHeight) / static_cast<float>(line.height) : 1.0f;
        const int lineVerticalOffset = GetLineVisualOffset(lineIndex);

        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] BuildRenderCommands: line=%zu width=%d height=%d drawWidth=%d drawHeight=%d uvRight=%.3f uvBottom=%.3f", lineIndex, line.width, line.height, drawWidth, drawHeight, uvRight, uvBottom);
        }

        commands.DrawTexturedQuad(
            static_cast<float>(contentX - m_horizontalScrollBar->GetValue()),
            static_cast<float>(contentY + currentY + lineVerticalOffset),
            static_cast<float>(drawWidth),
            static_cast<float>(drawHeight),
            0.0f, 0.0f,
            uvRight, uvBottom,
            line.textureId,
            GetOpacity(),
            clip);

        currentY += std::max(1, line.lineHeight);
    }

    if (m_isFocused) {
        int caretX = GetCaretX();
        int rawCaretX = caretX;
        int caretY = GetCaretY();
        caretX = std::min(caretX, viewportWidth);
        const int caretHeight = std::max(1, GetCaretHeight());
        const auto [caretLineIndex, _] = GetCaretLineColumn(m_caretPosition);
        const int caretVerticalOffset = GetLineVisualOffset(caretLineIndex);
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] BuildRenderCommands: caretPosition=%zu rawCaretX=%d clampedCaretX=%d caretY=%d caretVerticalOffset=%d visibleTextWidth=%d", m_caretPosition, rawCaretX, caretX, caretY, caretVerticalOffset, viewportWidth);
        }

        const uint32_t caretTextureId = GetCaretTextureId();
        commands.DrawTexturedQuad(
            static_cast<float>(contentX + caretX - m_horizontalScrollBar->GetValue()),
            static_cast<float>(contentY + caretY + caretVerticalOffset - m_verticalScrollBar->GetValue()),
            1.0f,
            static_cast<float>(caretHeight),
            0.0f, 0.0f,
            1.0f, 1.0f,
            caretTextureId,
            GetOpacity(),
            clip);
    }

    const bool showVertical = m_showVerticalScrollbar && m_verticalScrollBar->GetScrollRange() > 0;
    const bool showHorizontal = m_showHorizontalScrollbar && m_horizontalScrollBar->GetScrollRange() > 0;

    if (showVertical) {
        const int barX = absX + GetFinalWidth() - scrollbarThickness - 2;
        const int barY = absY + contentInset + 4;
        const int barHeight = std::min(72, std::max(24, GetFinalHeight() - contentInset * 2 - 8));
        const int effectiveContentHeight = std::max(m_textLayout.GetContentHeight(), viewportHeight + 1);
        const int effectiveViewportHeight = std::max(1, viewportHeight);
        NeoWPF::Diagnostics::LogInfo("[TextBox] Vertical scrollbar metrics: content=%d viewport=%d track=%d range=%d thumb=%d", effectiveContentHeight, effectiveViewportHeight, barHeight, m_verticalScrollBar->GetScrollRange(), m_verticalScrollBar->GetThumbLength(barHeight));
        m_verticalScrollBar->SetOrientation(ScrollBarOrientation::Vertical);
        m_verticalScrollBar->SetViewportSize(effectiveViewportHeight);
        m_verticalScrollBar->SetContentSize(effectiveContentHeight);
        m_verticalScrollBar->Arrange(scrollbarThickness, barHeight);
        m_verticalScrollBar->SetLayoutOffset(barX, barY);
        m_verticalScrollBar->BuildRenderCommands(commands);
    }

    if (showHorizontal) {
        const int barX = absX + contentInset + 4;
        const int barY = absY + GetFinalHeight() - scrollbarThickness - 2;
        const int barWidth = std::min(96, std::max(24, GetFinalWidth() - contentInset * 2 - 8));
        const int effectiveContentWidth = std::max(m_textLayout.GetContentWidth(), viewportWidth + 1);
        const int effectiveViewportWidth = std::max(1, viewportWidth);
        NeoWPF::Diagnostics::LogInfo("[TextBox] Horizontal scrollbar metrics: content=%d viewport=%d track=%d range=%d thumb=%d", effectiveContentWidth, effectiveViewportWidth, barWidth, m_horizontalScrollBar->GetScrollRange(), m_horizontalScrollBar->GetThumbLength(barWidth));
        m_horizontalScrollBar->SetOrientation(ScrollBarOrientation::Horizontal);
        m_horizontalScrollBar->SetViewportSize(effectiveViewportWidth);
        m_horizontalScrollBar->SetContentSize(effectiveContentWidth);
        m_horizontalScrollBar->Arrange(barWidth, scrollbarThickness);
        m_horizontalScrollBar->SetLayoutOffset(barX, barY);
        m_horizontalScrollBar->BuildRenderCommands(commands);
    }
}

void TextBox::UpdateScrollMetrics()
{
    m_textLayout.Refresh(m_text, m_fontSize, false);

    const int contentWidth = m_textLayout.GetContentWidth();
    const int contentHeight = m_textLayout.GetContentHeight();
    const int baseViewportWidth = std::max(0, GetFinalWidth() - (m_padding * 2));
    const int baseViewportHeight = std::max(0, GetFinalHeight() - (m_padding * 2));
    const int scrollbarThickness = 8;

    int viewportWidth = baseViewportWidth;
    int viewportHeight = baseViewportHeight;
    bool hasHorizontalOverflow = false;
    bool hasVerticalOverflow = false;

    for (int pass = 0; pass < 2; ++pass) {
        hasHorizontalOverflow = contentWidth > viewportWidth;
        hasVerticalOverflow = contentHeight > viewportHeight;

        const int nextViewportWidth = std::max(0, baseViewportWidth - (hasVerticalOverflow ? scrollbarThickness : 0));
        const int nextViewportHeight = std::max(0, baseViewportHeight - (hasHorizontalOverflow ? scrollbarThickness : 0));
        if (nextViewportWidth == viewportWidth && nextViewportHeight == viewportHeight) {
            break;
        }

        viewportWidth = nextViewportWidth;
        viewportHeight = nextViewportHeight;
    }

    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_showVerticalScrollbar = hasVerticalOverflow && (contentHeight - m_viewportHeight) > 0;
    m_showHorizontalScrollbar = hasHorizontalOverflow && (contentWidth - m_viewportWidth) > 0;

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

    const int trackLength = std::max(0, m_verticalScrollBar->GetFinalHeight());
    return m_verticalScrollBar->GetThumbLength(trackLength);
}

int TextBox::GetVerticalThumbPosition() const
{
    if (!m_verticalScrollBar) {
        return 0;
    }

    const int trackLength = std::max(0, m_verticalScrollBar->GetFinalHeight());
    const int thumbLength = GetVerticalThumbLength();
    return m_verticalScrollBar->GetThumbPosition(trackLength, thumbLength);
}

int TextBox::GetHorizontalThumbLength() const
{
    if (!m_horizontalScrollBar) {
        return 0;
    }

    const int trackLength = std::max(0, m_horizontalScrollBar->GetFinalWidth());
    return m_horizontalScrollBar->GetThumbLength(trackLength);
}

int TextBox::GetHorizontalThumbPosition() const
{
    if (!m_horizontalScrollBar) {
        return 0;
    }

    const int trackLength = std::max(0, m_horizontalScrollBar->GetFinalWidth());
    const int thumbLength = GetHorizontalThumbLength();
    return m_horizontalScrollBar->GetThumbPosition(trackLength, thumbLength);
}

void TextBox::UpdateVerticalScrollFromPointer(int pointerY)
{
    m_verticalScrollBar->SetValue(pointerY);
    InvalidateVisual();
}

void TextBox::UpdateHorizontalScrollFromPointer(int pointerX)
{
    m_horizontalScrollBar->SetValue(pointerX);
    InvalidateVisual();
}

int TextBox::GetLineVisualOffset(std::size_t lineIndex) const
{
    return m_textLayout.GetLineVisualOffset(lineIndex);
}

void TextBox::EnsureTextTexture()
{
    if (!m_textDirty) {
        return;
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] EnsureTextTexture: textLength=%zu, lineCount=%zu", m_text.size(), m_textLayout.GetLineCount());
    }

    m_textLayout.Refresh(m_text, m_fontSize, true);
    m_textDirty = false;
}

}
