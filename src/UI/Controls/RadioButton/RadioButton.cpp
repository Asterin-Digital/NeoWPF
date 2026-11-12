#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

namespace {

void ApplyRadioButtonTheme(RadioButton& radioButton)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeFontSize;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;

    auto& themeManager = ThemeManager::Instance();
    radioButton.SetThemePropertyValue("Foreground", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Foreground));
    radioButton.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Surface));
    radioButton.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Border));

    int fontSize = 0;
    if (themeManager.TryGetFontSize(ThemeFontSize::Control, fontSize)) {
        radioButton.SetThemePropertyValue("FontSize", fontSize);
    }

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::RadioButton, padding)) {
        radioButton.SetThemePropertyValue("Padding", padding);
    }
}

}

RadioButton::RadioButton()
    : GroupedToggleBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<RadioButtonTemplate>());
    SetPresenter(std::make_shared<NeoWPF::UI::Content::ContentPresenter>());
    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    SetContent(m_textContent);
    ApplyRadioButtonTheme(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyRadioButtonTheme(*this);
    });
}

RadioButton::~RadioButton()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
    NeoWPF::UI::Localization::LocaleOperations::ClearLocale(*this, TextProperty());
}

void RadioButton::SetChecked(bool value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("IsChecked", *this)) {
            SetValue(*property, value);
            return;
        }
    }
    GroupedToggleBase::SetChecked(value);
}

void RadioButton::SetText(const std::wstring& text)
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this)) {
            SetValue(*property, text);
            return;
        }
    }
    m_text = text;
    if (m_textContent) m_textContent->SetText(text);
    InvalidateVisual();
}

void RadioButton::SetLocale(const std::wstring& key)
{
    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*this, TextProperty(), key);
}

void RadioButton::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textContent) m_textContent->SetTextHorizontalAlignment(alignment);
    InvalidateVisual();
}

TextHorizontalAlignment RadioButton::GetTextHorizontalAlignment() const noexcept
{
    return m_textContent ? m_textContent->GetTextHorizontalAlignment() : TextHorizontalAlignment::Left;
}

void RadioButton::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textContent) m_textContent->SetTextVerticalAlignment(alignment);
    InvalidateVisual();
}

TextVerticalAlignment RadioButton::GetTextVerticalAlignment() const noexcept
{
    return m_textContent ? m_textContent->GetTextVerticalAlignment() : TextVerticalAlignment::Top;
}

void RadioButton::SetForeground(std::shared_ptr<Brush> brush)
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Foreground", *this)) {
            SetValue(*property, brush);
            return;
        }
    }
    if (m_textContent) m_textContent->SetForeground(std::move(brush));
    InvalidateVisual();
}

void RadioButton::SetPadding(Thickness padding) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this)) {
            SetValue(*property, padding);
            return;
        }
    }
    InvalidateMeasure();
    InvalidateVisual();
}

void RadioButton::SetFontSize(int fontSize) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this)) {
            SetValue(*property, fontSize);
            return;
        }
    }
    if (m_textContent) m_textContent->SetFontSize(fontSize);
    InvalidateMeasure();
    InvalidateVisual();
}

void RadioButton::SetFontFamily(const std::wstring& fontFamily) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this)) {
            SetValue(*property, fontFamily);
            return;
        }
    }
    m_fontFamily = fontFamily;
    if (m_textContent) m_textContent->SetFontFamily(fontFamily);
    InvalidateVisual();
}

void RadioButton::SetFontWeight(int fontWeight) noexcept
{
    fontWeight = (std::max)(100, (std::min)(1000, fontWeight));
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontWeight", *this)) {
            SetValue(*property, fontWeight);
            return;
        }
    }
    m_fontWeight = fontWeight;
    if (m_textContent) m_textContent->SetFontWeight(fontWeight);
    InvalidateVisual();
}

void RadioButton::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontStyle", *this)) {
            SetValue(*property, fontStyle);
            return;
        }
    }
    m_fontStyle = fontStyle;
    if (m_textContent) m_textContent->SetFontStyle(fontStyle);
    InvalidateVisual();
}

std::shared_ptr<Brush> RadioButton::GetForeground() const
{
    return GetValue<std::shared_ptr<Brush>>(ForegroundProperty());
}
Thickness RadioButton::GetPadding() const noexcept { return GetValue<Thickness>(PaddingProperty()); }
int RadioButton::GetFontSize() const noexcept { return GetValue<int>(FontSizeProperty()); }
int RadioButton::GetFontWeight() const noexcept { return m_fontWeight; }
NeoWPF::UI::FontStyle RadioButton::GetFontStyle() const noexcept { return m_fontStyle; }

void RadioButton::Measure(double availableWidth, double availableHeight)
{
    const auto padding = GetPadding();
    const double contentWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - padding.Horizontal() - 18.0) : 0.0;
    Content::ContentControl::Measure(contentWidth, availableHeight);
    const auto presenter = GetPresenter();
    const double measuredWidth = presenter ? presenter->GetDesiredWidth() : 0.0;
    const double measuredHeight = presenter ? presenter->GetDesiredHeight() : 0.0;
    const double indicatorSize = (std::max)(18.0, measuredHeight);
    SetDesiredSize(indicatorSize + padding.Horizontal() + measuredWidth, indicatorSize + padding.Vertical());
}

void RadioButton::Arrange(double finalWidth, double finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
    if (auto presenter = GetPresenter()) {
        const auto padding = GetPadding();
        const double offsetX = 18.0 + padding.Left;
        presenter->SetLayoutOffset(offsetX, padding.Top);
        presenter->Arrange((std::max)(0.0, finalWidth - offsetX - padding.Right),
            (std::max)(0.0, finalHeight - padding.Vertical()));
    }
}

void RadioButton::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const double indicatorSize = 18.0;
    const auto transform = GetWorldRenderTransform();
    double left = 0.0;
    double top = 0.0;
    double right = GetFinalWidth();
    double bottom = GetFinalHeight();
    transform.positionMatrix.TransformPoint(left, top, left, top);
    transform.positionMatrix.TransformPoint(right, bottom, right, bottom);
    const double width = std::abs(right - left);
    const double height = std::abs(bottom - top);
    const double indicatorTop = top + (height - indicatorSize) / 2.0;

    auto circle = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Circle"));
    if (circle) {
        circle->SetRect(left, indicatorTop, indicatorSize, indicatorSize);
        circle->SetFillBrush(GetBackgroundBrush());
        circle->SetBorderBrush(GetBorderBrush());
        circle->SetBorderThickness(1);
        circle->SetOpacity(GetOpacity());
        circle->BuildRenderCommands(commands);
    }
    else {
        const auto backgroundBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
        const auto borderBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBorderBrush());
        commands.DrawRoundedRect(left, indicatorTop, indicatorSize, indicatorSize, 9.0f, 9.0f, 9.0f, 9.0f,
            NeoWPF::Rendering::Paint::Solid(backgroundBrush ? backgroundBrush->GetArgb() : 0x00000000u), GetOpacity(), NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
        commands.DrawRoundedRect(left, indicatorTop, indicatorSize, indicatorSize, 9.0f, 9.0f, 9.0f, 9.0f,
            NeoWPF::Rendering::Paint::Solid(borderBrush ? borderBrush->GetArgb() : 0x00000000u), GetOpacity(), NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
    }

    if (IsChecked()) {
        const double dotSize = 8.0;
        commands.DrawRoundedRect(left + (indicatorSize - dotSize) / 2.0, indicatorTop + (indicatorSize - dotSize) / 2.0,
            dotSize, dotSize, 4.0f, 4.0f, 4.0f, 4.0f,
            NeoWPF::Rendering::Paint::Solid(std::dynamic_pointer_cast<SolidColorBrush>(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
                NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent))->GetArgb()), GetOpacity(),
            NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
    }

    if (auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"))) {
        const auto padding = GetPadding();
        contentHost->SetRect(left + indicatorSize + padding.Left, top + padding.Top,
            (std::max)(0.0, width - indicatorSize - padding.Horizontal()),
            (std::max)(1.0, height - padding.Vertical()));
        contentHost->BuildRenderCommands(commands);
    }
    Content::ContentControl::BuildRenderCommands(commands);
}

void RadioButton::OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args) {
        return;
    }
    if (args->GetRoutedEvent() != &NeoWPF::Input::Core::PointerReleasedEvent()) {
        GroupedToggleBase::OnRoutedEvent(args);
        return;
    }

    auto* pointerArgs = dynamic_cast<NeoWPF::Input::Core::RoutedPointerEventArgs*>(args);
    if (!pointerArgs) {
        return;
    }
    if (!IsEnabled()) {
        m_isPressed = false;
        ApplyVisualState("Disabled", false);
        return;
    }
    const bool shouldActivate = m_isPressed && pointerArgs->GetChangedButton() == 0x01u;
    m_isPressed = false;
    if (shouldActivate) {
        if (!IsChecked()) SetChecked(true);
        RaiseClick();
    }
    ApplyVisualState(IsChecked() ? "CheckedMouseOver" : "MouseOver", false);
}

void RadioButton::OnVisualStateApplied(const std::string& stateName) { (void)stateName; }

bool RadioButton::IsGroupedPeer(const Visual* visual) const noexcept
{
    return dynamic_cast<const RadioButton*>(visual) != nullptr;
}

}