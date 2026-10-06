#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <memory>
#include <cmath>

namespace NeoWPF::UI {

namespace {

NeoWPF::UI::Color GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor paletteColor)
{
    NeoWPF::UI::Color color;
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().TryGetPaletteColor(paletteColor, color);
    return color;
}

void ApplyThemeStyle(
    NeoWPF::UI::Button& button,
    int& appliedFontSize,
    NeoWPF::UI::Thickness& appliedPadding,
    bool& hasAppliedFontSize,
    bool& hasAppliedPadding)
{
    using NeoWPF::Core::ApplicationThemes::PaletteColor;
    using NeoWPF::Core::ApplicationThemes::ThemeFontSize;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;
    using NeoWPF::UI::Properties::PropertyValueSource;

    auto& themeManager = ThemeManager::Instance();
    NeoWPF::UI::Color surface;
    NeoWPF::UI::Color border;
    NeoWPF::UI::Color foreground;
    if (themeManager.TryGetPaletteColor(PaletteColor::Surface, surface)) {
        button.SetStylePropertyValue("Background", surface);
    }
    if (themeManager.TryGetPaletteColor(PaletteColor::Border, border)) {
        button.SetStylePropertyValue("BorderBrush", border);
    }
    if (themeManager.TryGetPaletteColor(PaletteColor::Foreground, foreground)) {
        button.SetStylePropertyValue("Foreground", foreground);
    }

    int fontSize = 0;
    if (themeManager.TryGetFontSize(ThemeFontSize::Control, fontSize)) {
        const auto source = button.GetValueSource(NeoWPF::UI::Button::FontSizeProperty());
        if (source == PropertyValueSource::Default ||
            (hasAppliedFontSize && source == PropertyValueSource::Style &&
             button.GetFontSize() == appliedFontSize)) {
            button.SetStylePropertyValue("FontSize", fontSize);
            appliedFontSize = fontSize;
            hasAppliedFontSize = true;
        }
    }

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::Button, padding)) {
        const auto source = button.GetValueSource(NeoWPF::UI::Control::PaddingProperty());
        if (source == PropertyValueSource::Default ||
            (hasAppliedPadding && source == PropertyValueSource::Style &&
             button.GetPadding() == appliedPadding)) {
            button.SetStylePropertyValue("Padding", padding);
            appliedPadding = padding;
            hasAppliedPadding = true;
        }
    }
}

}

Button::Button()
    : ButtonBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Center);
    SetVerticalAlignment(VerticalAlignment::Center);
    SetTemplate(std::make_shared<ButtonTemplate>());

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Center);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    ApplyThemeStyle(
        *this,
        m_appliedThemeFontSize,
        m_appliedThemePadding,
        m_hasAppliedThemeFontSize,
        m_hasAppliedThemePadding);
    m_themeAppliedSubscriptionId =
        NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
            ApplyThemeStyle(
                *this,
                m_appliedThemeFontSize,
                m_appliedThemePadding,
                m_hasAppliedThemeFontSize,
                m_hasAppliedThemePadding);
        });
}

Button::Button(const std::wstring& text)
    : Button()
{
    SetText(text);
}

Button::~Button()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(
        m_themeAppliedSubscriptionId);
    NeoWPF::UI::Localization::LocaleOperations::ClearLocale(*this, TextProperty());
}

void Button::SetText(const std::wstring& text)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(TextProperty(), text);
        return;
    }

    m_text = text;
    if (m_textContent) {
        m_textContent->SetText(text);
    }
    InvalidateVisual();
}

void Button::SetLocale(const std::wstring& key)
{
    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*this, TextProperty(), key);
}

void Button::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(TextHorizontalAlignmentProperty(), alignment);
        return;
    }

    if (m_textContent) {
        m_textContent->SetTextHorizontalAlignment(alignment);
    }
    InvalidateVisual();
}

TextHorizontalAlignment Button::GetTextHorizontalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextHorizontalAlignment();
    }
    return TextHorizontalAlignment::Left;
}

void Button::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(TextVerticalAlignmentProperty(), alignment);
        return;
    }

    if (m_textContent) {
        m_textContent->SetTextVerticalAlignment(alignment);
    }
    InvalidateVisual();
}

TextVerticalAlignment Button::GetTextVerticalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextVerticalAlignment();
    }
    return TextVerticalAlignment::Top;
}

void Button::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(TextColorProperty(), color);
        return;
    }

    m_textColor = color;
    if (m_textContent) {
        m_textContent->SetTextColor(color.ToArgb());
    }
    InvalidateVisual();
}

void Button::SetFontSize(int fontSize) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(FontSizeProperty(), fontSize);
        return;
    }

    m_fontSize = fontSize;
    if (m_textContent) {
        m_textContent->SetFontSize(fontSize);
    }
    InvalidateVisual();
}

void Button::SetFontFamily(const std::wstring& fontFamily) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(FontFamilyProperty(), fontFamily);
        return;
    }

    m_fontFamily = fontFamily;
    if (m_textContent) {
        m_textContent->SetFontFamily(fontFamily);
    }
    InvalidateVisual();
}

void Button::SetFontWeight(int fontWeight) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(FontWeightProperty(), fontWeight);
        return;
    }

    m_fontWeight = fontWeight;
    if (m_textContent) {
        m_textContent->SetFontWeight(fontWeight);
    }
    InvalidateVisual();
}

void Button::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (!IsApplyingPropertyValue()) {
        SetValue(FontStyleProperty(), fontStyle);
        return;
    }

    m_fontStyle = fontStyle;
    if (m_textContent) {
        m_textContent->SetFontStyle(fontStyle);
    }
    InvalidateVisual();
}

void Button::SetPadding(Thickness padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

Thickness Button::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void Button::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
}

void Button::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBorderColor(color);
}

void Button::OnPaddingChanged(Thickness padding)
{
    if (auto presenter = GetPresenter()) {
        presenter->SetPadding(padding);
    }
    InvalidateVisual();
}

void Button::OnBackgroundColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetFillColor(color.ToArgb());
    }

    InvalidateVisual();
}

void Button::OnBorderColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderColor(color.ToArgb());
    }

    InvalidateVisual();
}

void Button::OnBorderThicknessChanged(double)
{
    InvalidateVisual();
}

void Button::Measure(double availableWidth, double availableHeight)
{
    Content::ContentControl::Measure(availableWidth, availableHeight);
}

void Button::Arrange(double finalWidth, double finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
}

void Button::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto renderTransform = GetWorldRenderTransform();
    double drawX = 0.0;
    double drawY = 0.0;
    double drawWidth = GetFinalWidth();
    double drawHeight = GetFinalHeight();
    renderTransform.positionMatrix.TransformPoint(drawX, drawY, drawX, drawY);
    double right = drawWidth;
    double bottom = drawHeight;
    renderTransform.positionMatrix.TransformPoint(right, bottom, right, bottom);
    drawWidth = std::abs(right - drawX);
    drawHeight = std::abs(bottom - drawY);
    float rotationRadians = GetWorldRotationRadians();
    rotationRadians = renderTransform.rotationRadians;

    const auto cornerRadius = GetCornerRadius();
    const auto nonNegative = [](double value) {
        return value > 0.0 ? value : 0.0;
    };

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(
            drawX,
            drawY,
            nonNegative(drawWidth),
            nonNegative(drawHeight));
        background->SetBorderThickness(1);
        background->SetCornerRadius(cornerRadius);
        background->SetOpacity(GetOpacity());
        background->SetRotation(rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
        background->BuildRenderCommands(commands);
    }
    else {
        const double borderThickness = 1.0;
        const uint32_t borderColor = GetBorderColor().ToArgb();
        const double borderInset = borderThickness * 0.5;
        const double borderX = drawX + borderInset;
        const double borderY = drawY + borderInset;
        const double borderW = drawWidth > borderThickness ? drawWidth - borderThickness : 0.0;
        const double borderH = drawHeight > borderThickness ? drawHeight - borderThickness : 0.0;

        const double borderTL = cornerRadius.TopLeft > borderInset ? cornerRadius.TopLeft - borderInset : 0.0;
        const double borderTR = cornerRadius.TopRight > borderInset ? cornerRadius.TopRight - borderInset : 0.0;
        const double borderBR = cornerRadius.BottomRight > borderInset ? cornerRadius.BottomRight - borderInset : 0.0;
        const double borderBL = cornerRadius.BottomLeft > borderInset ? cornerRadius.BottomLeft - borderInset : 0.0;

        commands.DrawRoundedRect(
            borderX,
            borderY,
            borderW,
            borderH,
            borderTL,
            borderTR,
            borderBR,
            borderBL,
            borderColor,
            GetOpacity(),
            NeoWPF::Rendering::ClipRect{},
            rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);

        const double inset = borderThickness;
        const double innerX = drawX + inset;
        const double innerY = drawY + inset;
        const double innerW = drawWidth > (2.0 * inset) ? drawWidth - (2.0 * inset) : 0.0;
        const double innerH = drawHeight > (2.0 * inset) ? drawHeight - (2.0 * inset) : 0.0;
        if (innerW > 0.0f && innerH > 0.0f) {
            const double innerTL = cornerRadius.TopLeft > inset ? cornerRadius.TopLeft - inset : 0.0;
            const double innerTR = cornerRadius.TopRight > inset ? cornerRadius.TopRight - inset : 0.0;
            const double innerBR = cornerRadius.BottomRight > inset ? cornerRadius.BottomRight - inset : 0.0;
            const double innerBL = cornerRadius.BottomLeft > inset ? cornerRadius.BottomLeft - inset : 0.0;

            commands.DrawRoundedRect(
                innerX,
                innerY,
                innerW,
                innerH,
                innerTL,
                innerTR,
                innerBR,
                innerBL,
                GetBackgroundColor().ToArgb(),
                GetOpacity(),
                NeoWPF::Rendering::ClipRect{},
                rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
        }
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const auto padding = GetPadding();
        const double absX = drawX;
        const double absY = drawY;
        const double insetX = padding.Left > 0.0 ? padding.Left : 0.0;
        const double insetY = padding.Top > 0.0 ? padding.Top : 0.0;
        const double chX = drawX + insetX;
        const double chY = drawY + insetY;
        const double chW = drawWidth - padding.Horizontal();
        const double chH = drawHeight - padding.Vertical();
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
            "[Button] BuildRenderCommands: abs=(%g,%g) size=(%g,%g) inset=(%g,%g) contentHost=(%g,%g,%g,%g) contentHostPtr=%p",
            absX, absY, GetFinalWidth(), GetFinalHeight(), insetX, insetY, chX, chY, chW, chH, contentHost.get());

        contentHost->SetRect(chX, chY, chW > 0.0 ? chW : 0.0, chH > 0.0 ? chH : 0.0);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->SetRotation(rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
        contentHost->BuildRenderCommands(commands);

        auto presenter = GetPresenter();
        if (presenter) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
                "[Button] Presenter: ptr=%p final=(%g,%g) layoutOffset=(%g,%g)",
                presenter.get(), presenter->GetFinalWidth(), presenter->GetFinalHeight(), presenter->GetLayoutOffsetX(), presenter->GetLayoutOffsetY());
        }

        if (m_textContent) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
                "[Button] TextContent: ptr=%p final=(%g,%g) layoutOffset=(%g,%g)",
                m_textContent.get(), m_textContent->GetFinalWidth(), m_textContent->GetFinalHeight(), m_textContent->GetLayoutOffsetX(), m_textContent->GetLayoutOffsetY());
        }
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}