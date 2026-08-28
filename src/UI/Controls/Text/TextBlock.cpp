#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

TextBlock::TextBlock()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

TextBlock::TextBlock(const std::wstring& text) : TextBlock()
{
    SetText(text);
}

TextBlock::~TextBlock()
{
    // TextRasterCache destructor already handles cleanup
}

void TextBlock::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }
    m_text = text;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBlock::SetFontFamily(const std::wstring& fontFamily) noexcept
{
    if (fontFamily.empty()) {
        return;
    }

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

std::wstring TextBlock::GetFontFamily() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        try { return GetValue<std::wstring>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_fontFamily;
}

void TextBlock::SetFontWeight(int fontWeight) noexcept
{
    if (fontWeight < 100) {
        fontWeight = 100;
    }
    if (fontWeight > 1000) {
        fontWeight = 1000;
    }

    if (m_fontWeight == fontWeight) {
        return;
    }

    m_fontWeight = fontWeight;
    m_textDirty = true;
    InvalidateMeasure();
    InvalidateVisual();
}

int TextBlock::GetFontWeight() const noexcept
{
    return m_fontWeight;
}

void TextBlock::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (m_fontStyle == fontStyle) {
        return;
    }

    m_fontStyle = fontStyle;
    m_textDirty = true;
    InvalidateMeasure();
    InvalidateVisual();
}

NeoWPF::UI::FontStyle TextBlock::GetFontStyle() const noexcept
{
    return m_fontStyle;
}

void TextBlock::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textHorizontalAlignment == alignment) {
        return;
    }
    m_textHorizontalAlignment = alignment;
    InvalidateVisual();
}

TextHorizontalAlignment TextBlock::GetTextHorizontalAlignment() const noexcept
{
    return m_textHorizontalAlignment;
}

void TextBlock::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textVerticalAlignment == alignment) {
        return;
    }
    m_textVerticalAlignment = alignment;
    InvalidateVisual();
}

TextVerticalAlignment TextBlock::GetTextVerticalAlignment() const noexcept
{
    return m_textVerticalAlignment;
}

void TextBlock::SetTextWrapping(TextWrapping wrapping) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_textWrapping = wrapping;
        m_textDirty = true;
        InvalidateMeasure();
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextWrapping", *this);
    if (property) {
        SetValue(*property, wrapping);
        return;
    }

    if (m_textWrapping == wrapping) return;
    m_textWrapping = wrapping;
    m_textDirty = true;
    InvalidateMeasure();
    InvalidateVisual();
}

TextWrapping TextBlock::GetTextWrapping() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextWrapping", *this);
    if (property) {
        try { return GetValue<TextWrapping>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_textWrapping;
}

Thickness TextBlock::GetPadding() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        try { return GetValue<Thickness>(*property); } catch (const std::bad_any_cast&) {}
    }
    return Thickness(m_padding);
}

void TextBlock::SetPadding(Thickness padding) noexcept {
    if (IsApplyingPropertyValue()) {
        m_padding = padding.Left;
        OnPaddingChanged(padding.Left);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        if (GetPadding() != padding) {
            SetValue(*property, padding);
        }
        return;
    }

    m_padding = padding.Left;
    OnPaddingChanged(padding.Left);
}

void TextBlock::OnPaddingChanged(Thickness padding) {
    (void)padding;
    InvalidateMeasure();
    InvalidateVisual();
}

void TextBlock::Measure(int availableWidth, int availableHeight)
{
    const auto padding = GetPadding();
    if (m_text.empty()) {
        SetDesiredSize(padding.Horizontal(), padding.Vertical());
        return;
    }

    int widthConstraint = 0;
    if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0) {
        widthConstraint = std::max(0, availableWidth - padding.Horizontal());
    }

    auto flow = NeoWPF::UI::Text::BreakText(m_text, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
    SetDesiredSize(flow.contentWidth + padding.Horizontal(), flow.contentHeight + padding.Vertical());
}

void TextBlock::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    const auto padding = GetPadding();
    const int innerWidth = std::max(0, finalWidth - padding.Horizontal());
    if (m_textDirty || innerWidth != m_lastArrangeWidth) {
        EnsureTextTexture(innerWidth, finalHeight);
        m_lastArrangeWidth = innerWidth;
    }
}

void TextBlock::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto& lines = m_textCache.GetLines();
    if (m_text.empty() || lines.empty()) return;

    const auto clip = GetClipRect();
    const auto padding = GetPadding();
    const int innerWidth = std::max(0, GetFinalWidth() - padding.Horizontal());
    const int innerHeight = std::max(0, GetFinalHeight() - padding.Vertical());

    const int blockOffsetY = TextAlignment::GetBlockVerticalOffset(
    m_textCache.GetContentWidth(), m_textCache.GetContentHeight(), innerHeight, m_textVerticalAlignment);
    const float origBlockW = static_cast<float>(GetFinalWidth());
    const float origBlockH = static_cast<float>(GetFinalHeight());
    const auto worldTransform = GetWorldTransformMatrix();
    const float rotationRadians = GetWorldRotationRadians();
    const auto origin = GetRenderTransformOrigin();
    float pivotX = 0.0f;
    float pivotY = 0.0f;
    worldTransform.TransformPoint(origBlockW * origin.X, origBlockH * origin.Y, pivotX, pivotY);
    int currentY = blockOffsetY;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        const auto textureId = m_textCache.GetLineTextureId(i);
        const int lineHeight = std::max(1, line.lineHeight);
        if (textureId == 0 || line.width <= 0 || line.height <= 0) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
                NeoWPF::Diagnostics::Level::Debug,
                "[TextBlock] skip line=%zu textureId=%u size=%dx%d lineHeight=%d",
                i, textureId, line.width, line.height, lineHeight);
            currentY += lineHeight;
            continue;
        }

        const int lineOffsetX = TextAlignment::GetLineHorizontalOffset(
            line.width, innerWidth, m_textHorizontalAlignment);

        const float localX = static_cast<float>(padding.Left + lineOffsetX);
        const float localY = static_cast<float>(padding.Top + currentY);

        /* @note 
        do not use ink top here. it's not needed and only breaks text positioning 
        for example if you use it, the text may position at the bottom or at the top slightly outside
        even if you use - or +, so do not use it.*/
        float drawX = 0.0f;
        float drawY = 0.0f;
        float rightX = 0.0f;
        float rightY = 0.0f;
        float bottomX = 0.0f;
        float bottomY = 0.0f;
        worldTransform.TransformPoint(localX, localY, drawX, drawY);
        worldTransform.TransformPoint(localX + static_cast<float>(line.width), localY, rightX, rightY);
        worldTransform.TransformPoint(localX, localY + static_cast<float>(line.height), bottomX, bottomY);
        const float drawWidth = std::hypot(rightX - drawX, rightY - drawY);
        const float drawHeight = std::hypot(bottomX - drawX, bottomY - drawY);

        commands.DrawTexturedQuad(drawX, drawY, drawWidth, drawHeight,
                                   0.0f, 0.0f, 1.0f, 1.0f,
                                   textureId, m_textColor.ToArgb(), GetOpacity(), clip, rotationRadians, pivotX, pivotY);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
            NeoWPF::Diagnostics::Level::Debug,
            "[TextBlock] draw line=%zu textureId=%u pos=%.1f,%.1f size=%.1fx%.1f opacity=%.2f",
            i, textureId, drawX, drawY, drawWidth, drawHeight, GetOpacity());

        currentY += lineHeight;
    }
}

void TextBlock::EnsureTextTexture(int availableWidth, int /*availableHeight*/)
{
    if (!m_textDirty && availableWidth == m_lastArrangeWidth) {
        return;
    }

    int widthConstraint = 0;
    if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0) {
        widthConstraint = availableWidth;
    }

    m_textCache.Refresh(m_text, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
    m_lastArrangeWidth = availableWidth;
    m_textDirty = false;
}

}
