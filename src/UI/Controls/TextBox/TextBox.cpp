#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
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

bool NeoWPF::UI::TextBox::s_textBoxDebug = false;

namespace NeoWPF::UI {

TextBox::TextBox()
    : Content::ContentControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(m_padding);
    SetPresenter(presenter);

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

void TextBox::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();
}

void TextBox::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;

    if (auto templateManager = GetTemplateVisualStateManager()) {
        templateManager->GoToState(this, stateName, useTransitions);
    }
    else if (m_visualStateManager) {
        m_visualStateManager->GoToState(this, stateName, useTransitions);
    }
}

void TextBox::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }

    if (IsApplyingPropertyValue()) {
        m_text = text;
        m_caretPosition = std::min(m_caretPosition, m_text.size());
        m_verticalScrollBar->SetValue(0);
        m_horizontalScrollBar->SetValue(0);
        m_textDirty = true;
        InvalidateVisual();
        TextChanged.Invoke(m_text);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(*this));
    if (property) {
        SetValue(*property, text);
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

void TextBox::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_textColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextColor", typeid(*this));
    if (property) { SetValue(*property, color); return; }
    m_textColor = color; InvalidateVisual();
}

void TextBox::SetTextColor(std::uint32_t color) noexcept
{
    SetTextColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetTextColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextColor", typeid(*this));
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_textColor; }
    }
    return m_textColor;
}

void TextBox::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_backgroundColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BackgroundColor", typeid(*this));
    if (property) { SetValue(*property, color); return; }
    m_backgroundColor = color; InvalidateVisual();
}

void TextBox::SetBackgroundColor(std::uint32_t color) noexcept
{
    SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBackgroundColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BackgroundColor", typeid(*this));
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_backgroundColor; }
    }
    return m_backgroundColor;
}

void TextBox::SetFontSize(int fontSize) noexcept
{
    if (IsApplyingPropertyValue()) { m_fontSize = fontSize; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", typeid(*this));
    if (property) { SetValue(*property, fontSize); return; }
    m_fontSize = fontSize; InvalidateVisual();
}

int TextBox::GetFontSize() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", typeid(*this));
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) { return m_fontSize; }
    }
    return m_fontSize;
}

void TextBox::SetPadding(int padding) noexcept
{
    if (IsApplyingPropertyValue()) { m_padding = padding; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(*this));
    if (property) { SetValue(*property, padding); return; }
    m_padding = padding; InvalidateVisual();
}

int TextBox::GetPadding() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(*this));
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) { return m_padding; }
    }
    return m_padding;
}

void TextBox::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_borderColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderColor", typeid(*this));
    if (property) { SetValue(*property, color); return; }
    m_borderColor = color; InvalidateVisual();
}

void TextBox::SetBorderColor(std::uint32_t color) noexcept
{
    SetBorderColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBorderColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderColor", typeid(*this));
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_borderColor; }
    }
    return m_borderColor;
}

void TextBox::SetCaretColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_caretColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretColor", typeid(*this));
    if (property) { SetValue(*property, color); return; }
    m_caretColor = color; InvalidateVisual();
}

void TextBox::SetCaretColor(std::uint32_t color) noexcept
{
    SetCaretColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetCaretColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretColor", typeid(*this));
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_caretColor; }
    }
    return m_caretColor;
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int minWidth = 120;
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (m_padding * 2);
    int desiredWidth = std::max(minWidth, availableWidth > 0 ? std::min(minWidth, availableWidth) : minWidth);
    int desiredHeight = std::max(minHeight, availableHeight > 0 ? std::min(minHeight, availableHeight) : minHeight);

    if (m_textWrapping != TextWrapping::NoWrap) {
        TextLayout measureLayout;
        const int contentHostWidth = std::max(0, availableWidth > 0 ? availableWidth - (m_padding * 2) : 0);
        const int contentHostHeight = std::max(0, availableHeight > 0 ? availableHeight - (m_padding * 2) : 0);
        measureLayout.Refresh(m_text, m_fontSize, contentHostWidth, contentHostHeight, m_textWrapping, false);
        desiredHeight = std::max(minHeight, measureLayout.GetContentHeight() + (m_padding * 2));
        desiredWidth = std::max(desiredWidth, measureLayout.GetContentWidth() + (m_padding * 2));
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());
    }

    if (auto presenter = GetPresenter()) {
        presenter->Measure(availableWidth, availableHeight);
    }

    const int resolvedWidth = ResolveWidth(availableWidth, desiredWidth);
    const int resolvedHeight = ResolveHeight(availableHeight, desiredHeight);
    SetDesiredSize(resolvedWidth, resolvedHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
    m_contentHostWidth = std::max(0, finalWidth - (m_padding * 2));
    m_contentHostHeight = std::max(0, finalHeight - (m_padding * 2));
    UpdateScrollMetrics();
    ClampScrollOffsets();

    if (m_verticalScrollBar) {
        m_verticalScrollBar->SetHorizontalAlignment(HorizontalAlignment::Left);
        m_verticalScrollBar->SetVerticalAlignment(VerticalAlignment::Top);
        m_verticalScrollBar->SetWidthMode(LayoutSizeMode::Auto);
        m_verticalScrollBar->SetHeightMode(LayoutSizeMode::Auto);
        if (!m_verticalScrollBar->GetParent()) {
                AddChild(m_verticalScrollBar);
        }
    }
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->SetHorizontalAlignment(HorizontalAlignment::Left);
        m_horizontalScrollBar->SetVerticalAlignment(VerticalAlignment::Top);
        m_horizontalScrollBar->SetWidthMode(LayoutSizeMode::Auto);
        m_horizontalScrollBar->SetHeightMode(LayoutSizeMode::Auto);
        if (!m_horizontalScrollBar->GetParent()) {
                AddChild(m_horizontalScrollBar);
        }
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Arrange: final=%dx%d, textDirty=%d, lineCount=%zu", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_textLayout.GetLineCount());
    }
    if (m_textDirty) {
        EnsureTextTexture();
    }
    if (m_isFocused && m_textDirty) {
        EnsureCaretVisible();
    }
}

void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    auto border = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Border"));
    if (border) {
        border->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        border->SetBorderThickness(1);
        border->SetCornerRadius(GetCornerRadius());
        border->SetOpacity(GetOpacity());
        border->BuildRenderCommands(commands);
    }
    else {
        const auto cornerRadius = GetCornerRadius();
        if (m_borderColor.ToArgb() != 0x00000000u) {
            commands.DrawRoundedRect(
                static_cast<float>(absX),
                static_cast<float>(absY),
                static_cast<float>(GetFinalWidth()),
                static_cast<float>(GetFinalHeight()),
                cornerRadius.TopLeft,
                cornerRadius.TopRight,
                cornerRadius.BottomRight,
                cornerRadius.BottomLeft,
                m_borderColor.ToArgb(),
                GetOpacity());
        }

        if (m_backgroundColor.ToArgb() != 0x00000000u) {
            commands.DrawRoundedRect(
                static_cast<float>(absX + 1),
                static_cast<float>(absY + 1),
                static_cast<float>(std::max(0, GetFinalWidth() - 2)),
                static_cast<float>(std::max(0, GetFinalHeight() - 2)),
                std::max(0.0f, cornerRadius.TopLeft - 1.0f),
                std::max(0.0f, cornerRadius.TopRight - 1.0f),
                std::max(0.0f, cornerRadius.BottomRight - 1.0f),
                std::max(0.0f, cornerRadius.BottomLeft - 1.0f),
                m_backgroundColor.ToArgb(),
                GetOpacity());
        }
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
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHostX = contentHost->GetX();
        contentHostY = contentHost->GetY();
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
        const int lineHeight = std::max(1, line.lineHeight);
        if (line.textureId == 0 || line.width <= 0 || line.height <= 0) {
            currentY += lineHeight;
            continue;
        }

        const int drawWidth = std::min(line.width, viewportWidth);
        const float uvRight = drawWidth > 0 ? static_cast<float>(drawWidth) / static_cast<float>(line.width) : 1.0f;
        const int drawHeight = std::max(1, line.height > 0 ? line.height : lineHeight);
        const float uvBottom = drawHeight > 0 ? static_cast<float>(drawHeight) / static_cast<float>(std::max(1, line.height > 0 ? line.height : lineHeight)) : 1.0f;
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

        currentY += lineHeight;
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
        m_verticalScrollBar->SetWidth(scrollbarThickness);
        m_verticalScrollBar->SetHeight(barHeight);
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
        m_horizontalScrollBar->SetWidth(barWidth);
        m_horizontalScrollBar->SetHeight(scrollbarThickness);
        m_horizontalScrollBar->Arrange(barWidth, scrollbarThickness);
        m_horizontalScrollBar->SetLayoutOffset(relativeBarX, relativeBarY);
        m_horizontalScrollBar->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void TextBox::EnsureTextTexture()
{
    if (!m_textDirty) {
        return;
    }

    m_textLayout.Refresh(m_text, m_fontSize, m_contentHostWidth, m_contentHostHeight, m_textWrapping, true);
    m_textDirty = false;
    InvalidateVisual();
}

int TextBox::GetLineVisualOffset(std::size_t lineIndex) const
{
    return m_textLayout.GetLineVisualOffset(lineIndex);
}

}
