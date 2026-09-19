#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBlockStyles.h>
#include <NeoWPF/ControlStyles.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>

namespace NeoWPF::UI {

TextBlock::TextBlock()
{
    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::TextBlockStyles, *this);
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

TextBlock::TextBlock(const std::wstring& text) : TextBlock()
{
    SetText(text);
}

TextBlock::~TextBlock()
{
    NeoWPF::UI::Localization::LocaleOperations::ClearLocale(*this, TextProperty());

}

void TextBlock::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }

    if (!IsApplyingPropertyValue()) {
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this);
        if (property) {
            SetValue(*property, text);
            return;
        }
    }

    m_text = text;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBlock::SetLocale(const std::wstring& key)
{
    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*this, TextProperty(), key);
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

void TextBlock::Measure(double availableWidth, double availableHeight)
{
    const auto padding = GetPadding();
    if (m_text.empty()) {
        SetDesiredSize(padding.Horizontal(), padding.Vertical());
        return;
    }

    double widthConstraint = 0.0;
    if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0.0) {
        widthConstraint = std::max(0.0, availableWidth - padding.Horizontal());
    }

    auto flow = NeoWPF::UI::Text::BreakText(m_text, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
    SetDesiredSize(flow.contentWidth + padding.Horizontal(), flow.contentHeight + padding.Vertical());
}

void TextBlock::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    const auto padding = GetPadding();
    const double innerWidth = std::max(0.0, finalWidth - padding.Horizontal());
    if (m_textDirty || innerWidth != m_lastArrangeWidth) {
        EnsureTextTexture(std::lround(innerWidth), std::lround(finalHeight));
        m_lastArrangeWidth = std::lround(innerWidth);
    }
}

void TextBlock::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto padding = GetPadding();
    const double innerWidth = std::max(0.0, GetFinalWidth() - padding.Horizontal());
    const double innerHeight = std::max(0.0, GetFinalHeight() - padding.Vertical());

    const auto& lines = m_textCache.GetLines();
    if (m_text.empty() || lines.empty()) return;

    const auto clip = GetClipRect();

    const double blockOffsetY = TextAlignment::GetBlockVerticalOffsetF(
    m_textCache.GetContentWidth(), m_textCache.GetContentHeight(), innerHeight, m_textVerticalAlignment);
    const auto worldRenderTransform = GetWorldRenderTransform();
    const float rotationRadians = worldRenderTransform.rotationRadians;
    const float pivotX = worldRenderTransform.pivot.X;
    const float pivotY = worldRenderTransform.pivot.Y;
    double currentY = blockOffsetY;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        const auto textureId = m_textCache.GetLineTextureId(i);
        const double lineHeight = std::max(1.0, line.lineHeight);
        if (textureId == 0 || line.width <= 0.0 || line.height <= 0.0) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextRendering,
                NeoWPF::Diagnostics::Level::Debug,
                "[TextBlock] skip line=%zu textureId=%u size=%.1f,%.1f lineHeight=%.1f",
                i, textureId, line.width, line.height, lineHeight);
            currentY += lineHeight;
            continue;
        }

        const double lineOffsetX = TextAlignment::GetLineHorizontalOffsetF(
            line.width, innerWidth, m_textHorizontalAlignment);

        const double localX = padding.Left + lineOffsetX;
        const double localY = padding.Top + currentY;

        double drawX = localX;
        double drawY = localY;
        worldRenderTransform.positionMatrix.TransformPoint(drawX, drawY, drawX, drawY);
        double right = localX + line.width;
        double bottom = localY + line.height;
        worldRenderTransform.positionMatrix.TransformPoint(right, bottom, right, bottom);
        const double drawWidth = std::abs(right - drawX);
        const double drawHeight = std::abs(bottom - drawY);

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

void TextBlock::EnsureTextTexture(int availableWidth, int)
{
    if (!m_textDirty && availableWidth == m_lastArrangeWidth) {
        return;
    }

    double widthConstraint = 0.0;
    if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0) {
        widthConstraint = availableWidth;
    }

    m_textCache.Refresh(m_text, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
    m_lastArrangeWidth = availableWidth;
    m_textDirty = false;
}

}
