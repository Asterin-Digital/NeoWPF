#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

namespace {

int DefaultPadding() noexcept { return 0; }
NeoWPF::UI::Color DefaultBackgroundColor() noexcept { return NeoWPF::UI::Color::FromArgb(0x00000000u); }
NeoWPF::UI::Color DefaultBorderColor() noexcept { return NeoWPF::UI::Color::FromArgb(0x00000000u); }
int DefaultBorderThickness() noexcept { return 0; }

}

int Control::GetPadding() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_padding;
}

void Control::SetPadding(int padding) noexcept {
    const int oldPadding = GetPadding();
    if (IsApplyingPropertyValue()) {
        m_padding = padding;
        OnPaddingChanged(padding);
        if (oldPadding != padding) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
                "[Control] SetPadding (applying): old=%d new=%d this=%p", oldPadding, padding, this);
        }
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
            "[Control] SetPadding: old=%d new=%d this=%p", oldPadding, padding, this);
    }
}

NeoWPF::UI::Color Control::GetBackgroundColor() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BackgroundColor", *this);
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

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BackgroundColor", *this);
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
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderColor", *this);
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

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderColor", *this);
    if (property) {
        if (GetBorderColor() != color) {
            SetValue(*property, color);
        }
        return;
    }

    m_borderColor = color;
    OnBorderColorChanged(color);
}

int Control::GetBorderThickness() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderThickness", *this);
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_borderThickness;
}

void Control::SetBorderThickness(int thickness) noexcept {
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

void Control::OnPaddingChanged(int) {}
void Control::OnBackgroundColorChanged(NeoWPF::UI::Color) {}
void Control::OnBorderColorChanged(NeoWPF::UI::Color) {}
void Control::OnBorderThicknessChanged(int) {}

}
