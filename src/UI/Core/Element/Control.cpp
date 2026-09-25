#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

Thickness Control::GetPadding() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        try { return GetValue<Thickness>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_padding;
}

double Control::GetPaddingLeft() const noexcept { return GetPadding().Left; }
double Control::GetPaddingTop() const noexcept { return GetPadding().Top; }
double Control::GetPaddingRight() const noexcept { return GetPadding().Right; }
double Control::GetPaddingBottom() const noexcept { return GetPadding().Bottom; }
double Control::GetPaddingHorizontal() const noexcept { return GetPadding().Horizontal(); }
double Control::GetPaddingVertical() const noexcept { return GetPadding().Vertical(); }

void Control::SetPadding(Thickness padding) noexcept {
    const auto oldPadding = GetPadding();
    if (IsApplyingPropertyValue()) {
        m_padding = padding;
        OnPaddingChanged(padding);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        if (GetPadding() != padding) {
            SetValue(*property, padding);
        }
        return;
    }

    m_padding = padding;
    OnPaddingChanged(padding);
    if (oldPadding != padding) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
            "[Control] SetPadding: old=(%g,%g,%g,%g) new=(%g,%g,%g,%g) this=%p",
            oldPadding.Left, oldPadding.Top, oldPadding.Right, oldPadding.Bottom,
            padding.Left, padding.Top, padding.Right, padding.Bottom, this);
    }
}

void Control::SetPadding(int left, int top, int right, int bottom) noexcept {
    SetPadding(Thickness(left, top, right, bottom));
}

void Control::OnPaddingChanged(Thickness)
{
}

NeoWPF::UI::Color Control::GetBackgroundColor() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Background", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_backgroundColor;
}

void Control::SetBackgroundColor(NeoWPF::UI::Color color) noexcept {
    if (IsApplyingPropertyValue()) {
        m_backgroundColor = color;
        OnBackgroundColorChanged(color);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Background", *this);
    if (property) {
        if (GetBackgroundColor() != color) {
            SetValue(*property, color);
        }
        return;
    }

    m_backgroundColor = color;
    OnBackgroundColorChanged(color);
}

NeoWPF::UI::Color Control::GetBorderColor() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderBrush", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_borderColor;
}

void Control::SetBorderColor(NeoWPF::UI::Color color) noexcept {
    if (IsApplyingPropertyValue()) {
        m_borderColor = color;
        OnBorderColorChanged(color);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderBrush", *this);
    if (property) {
        if (GetBorderColor() != color) {
            SetValue(*property, color);
        }
        return;
    }

    m_borderColor = color;
    OnBorderColorChanged(color);
}

double Control::GetBorderThickness() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderThickness", *this);
    if (property) {
        try { return GetValue<double>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_borderThickness;
}

void Control::SetBorderThickness(double thickness) noexcept {
    if (IsApplyingPropertyValue()) {
        m_borderThickness = thickness;
        OnBorderThicknessChanged(thickness);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderThickness", *this);
    if (property) {
        if (GetBorderThickness() != thickness) {
            SetValue(*property, thickness);
        }
        return;
    }

    m_borderThickness = thickness;
    OnBorderThicknessChanged(thickness);
}

void Control::OnBackgroundColorChanged(NeoWPF::UI::Color) {}
void Control::OnBorderColorChanged(NeoWPF::UI::Color) {}
void Control::OnBorderThicknessChanged(double) {}

}
