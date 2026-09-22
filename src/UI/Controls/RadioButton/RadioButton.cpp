#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Controls/RadioButtonStyles.h>
#include <NeoWPF/ControlStyles.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

RadioButton::RadioButton()
    : GroupedToggleBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<RadioButtonTemplate>());
    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::RadioButtonStyles, *this);
    SetPresenter(std::make_shared<NeoWPF::UI::Content::ContentPresenter>());
    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);
}

RadioButton::~RadioButton()
{
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

void RadioButton::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Foreground", *this)) {
            SetValue(*property, color);
            return;
        }
    }
    m_textColor = color;
    if (m_textContent) m_textContent->SetTextColor(color.ToArgb());
    InvalidateVisual();
}

void RadioButton::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Background", *this)) {
            SetValue(*property, color);
            return;
        }
    }
    m_bgColor = color;
    InvalidateVisual();
}

void RadioButton::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderBrush", *this)) {
            SetValue(*property, color);
            return;
        }
    }
    m_borderColor = color;
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
    m_padding = padding;
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
    m_fontSize = fontSize;
    if (m_textContent) m_textContent->SetFontSize(fontSize);
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

NeoWPF::UI::Color RadioButton::GetTextColor() const noexcept { return m_textColor; }
NeoWPF::UI::Color RadioButton::GetBackgroundColor() const noexcept { return m_bgColor; }
NeoWPF::UI::Color RadioButton::GetBorderColor() const noexcept { return m_borderColor; }
Thickness RadioButton::GetPadding() const noexcept { return m_padding; }
int RadioButton::GetFontSize() const noexcept { return m_fontSize; }
int RadioButton::GetFontWeight() const noexcept { return m_fontWeight; }
NeoWPF::UI::FontStyle RadioButton::GetFontStyle() const noexcept { return m_fontStyle; }

void RadioButton::Measure(double availableWidth, double availableHeight)
{
    const double contentWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - m_padding.Horizontal() - 18.0) : 0.0;
    Content::ContentControl::Measure(contentWidth, availableHeight);
    const auto presenter = GetPresenter();
    const double measuredWidth = presenter ? presenter->GetDesiredWidth() : 0.0;
    const double measuredHeight = presenter ? presenter->GetDesiredHeight() : 0.0;
    const double indicatorSize = (std::max)(18.0, measuredHeight);
    SetDesiredSize(indicatorSize + m_padding.Horizontal() + measuredWidth, indicatorSize + m_padding.Vertical());
}

void RadioButton::Arrange(double finalWidth, double finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
    if (auto presenter = GetPresenter()) {
        const double offsetX = 18.0 + m_padding.Left;
        presenter->SetLayoutOffset(offsetX, m_padding.Top);
        presenter->Arrange((std::max)(0.0, finalWidth - offsetX - m_padding.Right),
            (std::max)(0.0, finalHeight - m_padding.Vertical()));
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
        circle->SetBorderThickness(1);
        circle->SetOpacity(GetOpacity());
        circle->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRoundedRect(left, indicatorTop, indicatorSize, indicatorSize, 9.0f, 9.0f, 9.0f, 9.0f,
            m_bgColor.ToArgb(), GetOpacity(), NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
        commands.DrawRoundedRect(left, indicatorTop, indicatorSize, indicatorSize, 9.0f, 9.0f, 9.0f, 9.0f,
            m_borderColor.ToArgb(), GetOpacity(), NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
    }

    if (IsChecked()) {
        const double dotSize = 8.0;
        commands.DrawRoundedRect(left + (indicatorSize - dotSize) / 2.0, indicatorTop + (indicatorSize - dotSize) / 2.0,
            dotSize, dotSize, 4.0f, 4.0f, 4.0f, 4.0f, m_borderColor.ToArgb(), GetOpacity(),
            NeoWPF::Rendering::ClipRect{}, transform.rotationRadians, transform.pivot.X, transform.pivot.Y);
    }

    if (auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"))) {
        contentHost->SetRect(left + indicatorSize + m_padding.Left, top + m_padding.Top,
            (std::max)(0.0, width - indicatorSize - m_padding.Horizontal()),
            (std::max)(1.0, height - m_padding.Vertical()));
        contentHost->BuildRenderCommands(commands);
    }
    Content::ContentControl::BuildRenderCommands(commands);
}

void RadioButton::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    (void)x;
    (void)y;
    if (!IsEnabled()) {
        m_isPressed = false;
        ApplyVisualState("Disabled", false);
        return;
    }
    const bool shouldActivate = m_isPressed && ((buttonState & 0x01) == 0);
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