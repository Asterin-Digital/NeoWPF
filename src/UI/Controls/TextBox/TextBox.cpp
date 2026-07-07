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
#include <mutex>

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
    static std::once_flag scrollbarsAdded;
    std::call_once(scrollbarsAdded, [this]() {
        if (auto self = std::dynamic_pointer_cast<Visual>(shared_from_this())) {
            if (m_verticalScrollBar) {
                self->AddVisualChild(m_verticalScrollBar);
            }
            if (m_horizontalScrollBar) {
                self->AddVisualChild(m_horizontalScrollBar);
            }
        }
    });

    FrameworkElement::Arrange(finalWidth, finalHeight);
    m_contentHostWidth = std::max(0, finalWidth - (m_padding * 2));
    m_contentHostHeight = std::max(0, finalHeight - (m_padding * 2));
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
    int contentHostX = absX + m_padding;
    int contentHostY = absY + m_padding;
    int contentHostWidth = std::max(0, GetFinalWidth() - (m_padding * 2));
    int contentHostHeight = std::max(0, GetFinalHeight() - (m_padding * 2));
    if (contentHost) {
        const int inset = std::max(0, m_padding);
        contentHostWidth = std::max(0, GetFinalWidth() - (inset * 2));
        contentHostHeight = std::max(0, GetFinalHeight() - (inset * 2));
        contentHost->SetRect(absX + inset, absY + inset, contentHostWidth, contentHostHeight);
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHostX = contentHost->GetX();
        contentHostY = contentHost->GetY();
        contentHostWidth = std::max(0, contentHost->GetWidth());
        contentHostHeight = std::max(0, contentHost->GetHeight());
        contentHost->BuildRenderCommands(commands);
    }
    m_contentHostWidth = contentHostWidth;
    m_contentHostHeight = contentHostHeight;

    const int scrollbarThickness = 6;
    const int viewportWidth = std::max(0, m_contentHostWidth);
    const int viewportHeight = std::max(0, m_contentHostHeight);
    const int contentX = contentHostX;
    const int contentY = contentHostY;
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
        const int barX = contentX + viewportWidth - scrollbarThickness;
        const int barY = contentY;
        const int relativeBarX = barX - absX;
        const int relativeBarY = barY - absY;
        const int availableVerticalSpace = std::max(8, viewportHeight - (showHorizontal ? scrollbarThickness : 0));
        const int barHeight = std::max(8, availableVerticalSpace);
        const int effectiveContentHeight = std::max(m_textLayout.GetContentHeight(), viewportHeight + 1);
        const int effectiveViewportHeight = std::max(1, viewportHeight);
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] Vertical scrollbar debug: contentHost=(%d,%d) viewport=(%d,%d) content=(%d,%d) bar=%d", contentHostX, contentHostY, viewportWidth, viewportHeight, m_textLayout.GetContentWidth(), m_textLayout.GetContentHeight(), barHeight);
            NeoWPF::Diagnostics::LogInfo("[TextBox] Vertical scrollbar metrics: content=%d viewport=%d track=%d range=%d (thumbQuery returns %d)", effectiveContentHeight, effectiveViewportHeight, barHeight, m_verticalScrollBar->GetScrollRange(), m_verticalScrollBar->GetThumbLength(barHeight));
        }
        m_verticalScrollBar->SetOrientation(ScrollBarOrientation::Vertical);
        m_verticalScrollBar->SetViewportSize(effectiveViewportHeight);
        m_verticalScrollBar->SetContentSize(effectiveContentHeight);
        m_verticalScrollBar->Arrange(scrollbarThickness, barHeight);
        m_verticalScrollBar->SetLayoutOffset(relativeBarX, relativeBarY);
        m_verticalScrollBar->BuildRenderCommands(commands);
    }

    if (showHorizontal) {
        const int barX = contentX;
        const int barY = contentY + viewportHeight - scrollbarThickness;
        const int relativeBarX = barX - absX;
        const int relativeBarY = barY - absY;
        const int availableHorizontalSpace = std::max(8, viewportWidth - (showVertical ? scrollbarThickness : 0));
        const int barWidth = std::max(8, availableHorizontalSpace);
        const int effectiveContentWidth = std::max(m_textLayout.GetContentWidth(), viewportWidth + 1);
        const int effectiveViewportWidth = std::max(1, viewportWidth);
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] Horizontal scrollbar metrics: content=%d viewport=%d track=%d range=%d thumb=%d", effectiveContentWidth, effectiveViewportWidth, barWidth, m_horizontalScrollBar->GetScrollRange(), m_horizontalScrollBar->GetThumbLength(barWidth));
        }
        m_horizontalScrollBar->SetOrientation(ScrollBarOrientation::Horizontal);
        m_horizontalScrollBar->SetViewportSize(effectiveViewportWidth);
        m_horizontalScrollBar->SetContentSize(effectiveContentWidth);
        m_horizontalScrollBar->Arrange(barWidth, scrollbarThickness);
        m_horizontalScrollBar->SetLayoutOffset(relativeBarX, relativeBarY);
        m_horizontalScrollBar->BuildRenderCommands(commands);
    }
}

}
