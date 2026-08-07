#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
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

    void ReleaseCaretTextureId() {
        const uint32_t caretTextureId = GetCaretTextureId();
        if (caretTextureId != 0) {
            NeoWPF::Rendering::Renderer_DestroyTexture(caretTextureId);
        }
    }
}

namespace {
    const NeoWPF::UI::Properties::DependencyProperty& EnsureTextProperty()
    {
        NeoWPF::UI::Properties::RegisterTextBoxProperties();
        if (const auto* property = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(NeoWPF::UI::TextBox))) {
            return *property;
        }
        return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::TextBox, std::wstring>("Text");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::TextBox::TextProperty = EnsureTextProperty();

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
    m_verticalScrollBar->SetOrientation(Orientation::Vertical);
    m_verticalScrollBar->SetThumbSize(24);

    m_horizontalScrollBar = std::make_shared<ScrollBar>();
    m_horizontalScrollBar->SetOrientation(Orientation::Horizontal);
    m_horizontalScrollBar->SetThumbSize(24);

    ApplyTextBoxThemeStyle(this);

    InitializeVisualStates();
}

void TextBox::OnInitialize()
{
    UIElement::OnInitialize();
    ApplyVisualState("Normal", false);
}
TextBox::~TextBox()
{
    ReleaseCaretTextureId();
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

    if (m_isUpdatingText) {
        m_text = text;
        m_caret.position = std::min(m_caret.position, m_text.size());
        m_textDirty = true;
        InvalidateVisual();
        TextChanged.Invoke(m_text);
        return;
    }

    m_isUpdatingText = true;

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this);
    if (property && !IsApplyingPropertyValue()) {
        SetValue(*property, text);
        m_isUpdatingText = false;
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] SetText: oldLength=%zu, newLength=%zu", m_text.size(), text.size());

    m_text = text;
    m_caret.position = std::min(m_caret.position, m_text.size());
    m_scroll.offsetX = 0;
    m_scroll.offsetY = 0;
    if (m_verticalScrollBar) {
        m_verticalScrollBar->SetValue(0);
    }
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->SetValue(0);
    }
    m_textDirty = true;
    InvalidateVisual();
    TextChanged.Invoke(m_text);
    m_isUpdatingText = false;
}

void TextBox::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                           const Properties::PropertyMetadata& metadata,
                                           const std::any& oldValue,
                                           const std::any& newValue,
                                           Properties::PropertyValueSource source)
{
    (void)metadata;
    (void)oldValue;
    (void)source;

    if (property.GetName() == "Text") {
        if (newValue.has_value()) {
            const std::wstring textValue = NeoWPF::UI::Properties::CoerceToWString(newValue);
            if (!m_isUpdatingText && m_text != textValue) {
                SetText(textValue);
                return;
            }
        }
    }

    Content::ContentControl::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
}

void TextBox::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_textColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextColor", *this);
    if (property) { SetValue(*property, color); return; }
    m_textColor = color; InvalidateVisual();
}

void TextBox::SetFontFamily(const std::wstring& fontFamily) noexcept
{
    // renderer already has a fallback font, so do not add one here as well 
    
    if (IsApplyingPropertyValue()) {
        m_fontFamily = fontFamily;
        m_textDirty = true;
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        SetValue(*property, fontFamily);
        return;
    }

    if (m_fontFamily == fontFamily) {
        return;
    }

    m_fontFamily = fontFamily;
    m_textDirty = true;
    InvalidateVisual();
}

std::wstring TextBox::GetFontFamily() const noexcept
{
    // dependency props can override the local field, so we check them first
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        try {
            const auto value = GetValue<std::wstring>(*property);
            return value;
        } catch (const std::bad_any_cast&) {}
    }
    return m_fontFamily;
}

void TextBox::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textHorizontalAlignment == alignment) {
        return;
    }
    m_textHorizontalAlignment = alignment;
    InvalidateVisual();
}

TextHorizontalAlignment TextBox::GetTextHorizontalAlignment() const noexcept
{
    return m_textHorizontalAlignment;
}

void TextBox::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textVerticalAlignment == alignment) {
        return;
    }
    m_textVerticalAlignment = alignment;
    InvalidateVisual();
}

TextVerticalAlignment TextBox::GetTextVerticalAlignment() const noexcept
{
    return m_textVerticalAlignment;
}

namespace {

int GetTextVerticalAlignmentOffset(int contentWidth, int contentHeight, int viewportHeight, TextVerticalAlignment alignment) noexcept
{
    int unusedX = 0;
    int outY = 0;
    TextAlignment::ApplyTextAlignment(contentWidth, contentHeight, contentWidth, viewportHeight, TextHorizontalAlignment::Left, alignment, unusedX, outY);
    return outY;
}

int GetLineHorizontalAlignmentOffset(int lineWidth, int viewportWidth, TextHorizontalAlignment alignment) noexcept
{
    int outX = 0;
    int unusedY = 0;
    TextAlignment::ApplyTextAlignment(lineWidth, 1, viewportWidth, 1, alignment, TextVerticalAlignment::Top, outX, unusedY);
    return outX;
}

}

void TextBox::SetTextColor(std::uint32_t color) noexcept
{
    SetTextColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetTextColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextColor", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_textColor; }
    }
    return m_textColor;
}

void TextBox::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
    InvalidateVisual();
}

void TextBox::OnPaddingChanged(int padding)
{
    if (auto presenter = GetPresenter()) {
        presenter->SetPadding(padding);
    }
    InvalidateVisual();
}

void TextBox::OnBackgroundColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetFillColor(color.ToArgb());
    }
    InvalidateVisual();
}

void TextBox::OnBorderColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderColor(color.ToArgb());
    }
    InvalidateVisual();
}

void TextBox::OnBorderThicknessChanged(int thickness)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderThickness(thickness);
    }
    InvalidateVisual();
}

void TextBox::SetBackgroundColor(std::uint32_t color) noexcept
{
    SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBackgroundColor() const noexcept
{
    return Control::GetBackgroundColor();
}

void TextBox::SetFontSize(int fontSize) noexcept
{
    if (IsApplyingPropertyValue()) { m_fontSize = fontSize; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this);
    if (property) { SetValue(*property, fontSize); return; }
    m_fontSize = fontSize; InvalidateVisual();
}

int TextBox::GetFontSize() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this);
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) { return m_fontSize; }
    }
    return m_fontSize;
}

void TextBox::SetPadding(int padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

int TextBox::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void TextBox::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBorderColor(color);
    InvalidateVisual();
}

void TextBox::SetBorderColor(std::uint32_t color) noexcept
{
    SetBorderColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBorderColor() const noexcept
{
    return Control::GetBorderColor();
}

void TextBox::SetCaretColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_caretColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretColor", *this);
    if (property) { SetValue(*property, color); return; }
    m_caretColor = color; InvalidateVisual();
}

void TextBox::SetCaretColor(std::uint32_t color) noexcept
{
    SetCaretColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetCaretColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretColor", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_caretColor; }
    }
    return m_caretColor;
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, GetFontFamily(), 0, 0);
    const int minWidth = 120;
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (GetPadding() * 2);
    int desiredWidth = std::max(minWidth, availableWidth > 0 ? std::min(minWidth, availableWidth) : minWidth);
    int desiredHeight = std::max(minHeight, availableHeight > 0 ? std::min(minHeight, availableHeight) : minHeight);

    if (m_textWrapping != TextWrapping::NoWrap) {
        TextLayout measureLayout;
        const int contentHostWidth = std::max(0, availableWidth > 0 ? availableWidth - (GetPadding() * 2) : 0);
        const int contentHostHeight = std::max(0, availableHeight > 0 ? availableHeight - (GetPadding() * 2) : 0);
        measureLayout.Refresh(m_text, m_fontSize, GetFontFamily(), contentHostWidth, contentHostHeight, m_textWrapping, false);
        desiredHeight = std::max(minHeight, measureLayout.GetContentHeight() + (GetPadding() * 2));
        desiredWidth = std::max(desiredWidth, measureLayout.GetContentWidth() + (GetPadding() * 2));
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());

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
    m_scroll.contentHostWidth = std::max(0, finalWidth - (GetPadding() * 2));
    m_scroll.contentHostHeight = std::max(0, finalHeight - (GetPadding() * 2));
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

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Arrange: final=%dx%d, textDirty=%d, lineCount=%zu", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_textLayout.GetLineCount());
    if (m_textDirty) {
        EnsureTextTexture();
    }
    if (m_isFocused && m_textDirty) {
        EnsureCaretVisible();
    }
}

void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_scroll.contentHostWidth <= 0 || m_scroll.contentHostHeight <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: content host size not ready (%d,%d); skipping layout render", m_scroll.contentHostWidth, m_scroll.contentHostHeight);
        return;
    }

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
        if (GetBorderColor().ToArgb() != 0x00000000u) {
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

        if (GetBackgroundColor().ToArgb() != 0x00000000u) {
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
    int contentHostX = absX + GetPadding();
    int contentHostY = absY + GetPadding();
    int contentHostWidth = std::max(0, GetFinalWidth() - (GetPadding() * 2));
    int contentHostHeight = std::max(0, GetFinalHeight() - (GetPadding() * 2));
    if (GetFinalWidth() < 0 || GetFinalHeight() < 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: negative final size (%d,%d)", GetFinalWidth(), GetFinalHeight());
    }
    if (contentHost) {
        const int inset = std::max(0, GetPadding());
        contentHostWidth = std::max(0, GetFinalWidth() - (inset * 2));
        contentHostHeight = std::max(0, GetFinalHeight() - (inset * 2));
        contentHost->SetRect(absX + inset, absY + inset, contentHostWidth, contentHostHeight);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHostX = contentHost->GetX();
        contentHostY = contentHost->GetY();
        contentHost->BuildRenderCommands(commands);
    }
    m_scroll.contentHostWidth = contentHostWidth;
    m_scroll.contentHostHeight = contentHostHeight;

    const int scrollbarThickness = 6;
    const int viewportWidth = std::max(0, m_scroll.contentHostWidth);
    const int viewportHeight = std::max(0, m_scroll.contentHostHeight);
    const int contentX = contentHostX;
    const int contentY = contentHostY;
    const auto clip = NeoWPF::Rendering::ClipRect{ static_cast<float>(contentX), static_cast<float>(contentY), static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), true };

    const int contentWidth = std::max(0, m_textLayout.GetContentWidth());
    const int contentHeight = std::max(0, m_textLayout.GetContentHeight());
    if (contentWidth < 0 || contentHeight < 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: negative layout content size (%d,%d)", contentWidth, contentHeight);
    }
    int textOffsetX = 0;
    int textOffsetY = 0;
    TextAlignment::ApplyTextAlignment(
        contentWidth,
        contentHeight,
        viewportWidth,
        viewportHeight,
        m_textHorizontalAlignment,
        m_textVerticalAlignment,
        textOffsetX,
        textOffsetY);

    int currentY = textOffsetY;
    if (!m_verticalScrollBar || !m_horizontalScrollBar) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: missing scrollbar instances");
        return;
    }
    currentY -= m_verticalScrollBar->GetValue();
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
        const int lineOffsetX = GetLineHorizontalAlignmentOffset(line.width, viewportWidth, m_textHorizontalAlignment);
        const int textureY = contentY + currentY + lineVerticalOffset;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] BuildRenderCommands: line=%zu width=%d height=%d drawWidth=%d drawHeight=%d uvRight=%.3f uvBottom=%.3f offsetX=%d", lineIndex, line.width, line.height, drawWidth, drawHeight, uvRight, uvBottom, lineOffsetX);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: bitmap texture Y=%d for line=%zu textureId=%u lineHeight=%d lineVerticalOffset=%d currentY=%d contentY=%d", textureY, lineIndex, static_cast<unsigned int>(line.textureId), lineHeight, lineVerticalOffset, currentY, contentY);

        const int horizontalOffset = m_scroll.showHorizontal ? m_horizontalScrollBar->GetValue() : 0;
        commands.DrawTexturedQuad(
            static_cast<float>(contentX - horizontalOffset + lineOffsetX),
            static_cast<float>(textureY),
            static_cast<float>(drawWidth),
            static_cast<float>(drawHeight),
            0.0f, 0.0f,
            uvRight, uvBottom,
            line.textureId,
            GetTextColor().ToArgb(),
            GetOpacity(),
            clip);

        currentY += lineHeight;
    }

    if (m_isFocused) {
        int caretX = GetCaretX();
        int rawCaretX = caretX;
        int caretY = GetCaretY();
        if (caretY < 0 || caretX < 0) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] BuildRenderCommands: negative caret metrics (%d,%d)", caretX, caretY);
            caretX = std::max(0, caretX);
            caretY = std::max(0, caretY);
        }
        caretX = std::min(caretX, viewportWidth);
        const int caretHeight = std::max(1, GetCaretHeight());
        const auto [caretLineIndex, _] = GetCaretLineColumn(m_caret.position);
        const int caretVerticalOffset = GetLineVisualOffset(caretLineIndex);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] BuildRenderCommands: caretPosition=%zu rawCaretX=%d clampedCaretX=%d caretY=%d caretVerticalOffset=%d visibleTextWidth=%d", m_caret.position, rawCaretX, caretX, caretY, caretVerticalOffset, viewportWidth);

        const uint32_t caretTextureId = GetCaretTextureId();
        const int horizontalOffset = m_scroll.showHorizontal ? m_horizontalScrollBar->GetValue() : 0;
        const int verticalOffset = m_scroll.showVertical ? m_verticalScrollBar->GetValue() : 0;
        commands.DrawTexturedQuad(
            static_cast<float>(contentX + caretX - horizontalOffset),
            static_cast<float>(contentY + caretY + caretVerticalOffset - verticalOffset),
            1.0f,
            static_cast<float>(caretHeight),
            0.0f, 0.0f,
            1.0f, 1.0f,
            caretTextureId,
            GetTextColor().ToArgb(),
            GetOpacity(),
            clip);
    }

    const bool showVertical = m_scroll.showVertical && m_verticalScrollBar->GetScrollRange() > 0;
    const bool showHorizontal = m_scroll.showHorizontal && m_horizontalScrollBar->GetScrollRange() > 0;

    if (showVertical) {
        const auto geometry = ComputeVerticalScrollBarGeometry(contentX, contentY, viewportWidth, viewportHeight);
        const int relativeBarX = geometry.barX - absX;
        const int relativeBarY = geometry.barY - absY;
        const int effectiveContentHeight = std::max(m_textLayout.GetContentHeight(), viewportHeight + 1);
        const int effectiveViewportHeight = std::max(1, viewportHeight);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Vertical scrollbar debug: contentHost=(%d,%d) viewport=(%d,%d) content=(%d,%d) bar=%d", contentHostX, contentHostY, viewportWidth, viewportHeight, m_textLayout.GetContentWidth(), m_textLayout.GetContentHeight(), geometry.barLength);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Vertical scrollbar metrics: content=%d viewport=%d track=%d range=%d (thumbQuery returns %d)", effectiveContentHeight, effectiveViewportHeight, geometry.barLength, m_verticalScrollBar->GetScrollRange(), geometry.thumbLength);
        m_verticalScrollBar->SetOrientation(Orientation::Vertical);
        m_verticalScrollBar->SetViewportSize(effectiveViewportHeight);
        m_verticalScrollBar->SetContentSize(effectiveContentHeight);
        m_verticalScrollBar->SetWidth(scrollbarThickness);
        m_verticalScrollBar->SetHeight(geometry.barLength);
        m_verticalScrollBar->Arrange(scrollbarThickness, geometry.barLength);
        m_verticalScrollBar->SetLayoutOffset(relativeBarX, relativeBarY);
        m_verticalScrollBar->BuildRenderCommands(commands);
    }

    if (showHorizontal) {
        const auto geometry = ComputeHorizontalScrollBarGeometry(contentX, contentY, viewportWidth, viewportHeight);
        const int relativeBarX = geometry.barX - absX;
        const int relativeBarY = geometry.barY - absY;
        const int effectiveContentWidth = std::max(m_textLayout.GetContentWidth(), viewportWidth + 1);
        const int effectiveViewportWidth = std::max(1, viewportWidth);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Horizontal scrollbar metrics: content=%d viewport=%d track=%d range=%d thumb=%d", effectiveContentWidth, effectiveViewportWidth, geometry.barLength, m_horizontalScrollBar->GetScrollRange(), geometry.thumbLength);
        m_horizontalScrollBar->SetOrientation(Orientation::Horizontal);
        m_horizontalScrollBar->SetViewportSize(effectiveViewportWidth);
        m_horizontalScrollBar->SetContentSize(effectiveContentWidth);
        m_horizontalScrollBar->SetWidth(geometry.barLength);
        m_horizontalScrollBar->SetHeight(scrollbarThickness);
        m_horizontalScrollBar->Arrange(geometry.barLength, scrollbarThickness);
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

    if (m_scroll.contentHostWidth < 0 || m_scroll.contentHostHeight < 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] EnsureTextTexture: negative content host size (%d,%d); clamping", m_scroll.contentHostWidth, m_scroll.contentHostHeight);
        m_scroll.contentHostWidth = std::max(0, m_scroll.contentHostWidth);
        m_scroll.contentHostHeight = std::max(0, m_scroll.contentHostHeight);
    }

    m_textLayout.Refresh(m_text, m_fontSize, GetFontFamily(), m_scroll.contentHostWidth, m_scroll.contentHostHeight, m_textWrapping, true);
    m_textDirty = false;
    InvalidateVisual();
}

TextBox::ScrollBarGeometry TextBox::ComputeVerticalScrollBarGeometry(int contentX, int contentY, int viewportWidth, int viewportHeight) const
{
    ScrollBarGeometry geometry;
    const int scrollbarThickness = 6;
    const int barX = contentX + viewportWidth - scrollbarThickness;
    const int barY = contentY;
    const int availableVerticalSpace = std::max(8, viewportHeight - (m_scroll.showHorizontal ? scrollbarThickness : 0));
    geometry.barX = barX;
    geometry.barY = barY;
    geometry.barLength = std::max(8, availableVerticalSpace);
    geometry.trackLength = geometry.barLength;
    geometry.thumbLength = m_verticalScrollBar ? m_verticalScrollBar->GetThumbLength(geometry.barLength) : 0;
    geometry.thumbOffset = m_verticalScrollBar ? m_verticalScrollBar->GetThumbPosition(geometry.barLength, geometry.thumbLength) : 0;
    return geometry;
}

TextBox::ScrollBarGeometry TextBox::ComputeHorizontalScrollBarGeometry(int contentX, int contentY, int viewportWidth, int viewportHeight) const
{
    ScrollBarGeometry geometry;
    const int scrollbarThickness = 6;
    const int barX = contentX;
    const int barY = contentY + viewportHeight - scrollbarThickness;
    const int availableHorizontalSpace = std::max(8, viewportWidth - (m_scroll.showVertical ? scrollbarThickness : 0));
    geometry.barX = barX;
    geometry.barY = barY;
    geometry.barLength = std::max(8, availableHorizontalSpace);
    geometry.trackLength = geometry.barLength;
    geometry.thumbLength = m_horizontalScrollBar ? m_horizontalScrollBar->GetThumbLength(geometry.barLength) : 0;
    geometry.thumbOffset = m_horizontalScrollBar ? m_horizontalScrollBar->GetThumbPosition(geometry.barLength, geometry.thumbLength) : 0;
    return geometry;
}

int TextBox::GetLineVisualOffset(std::size_t lineIndex) const
{
    return m_textLayout.GetLineVisualOffset(lineIndex);
}

}
