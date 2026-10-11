#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI {

Control::~Control()
{
    if (const auto brush = m_subscribedBackgroundBrush.lock();
        brush && m_backgroundBrushChangedHandlerId != 0) {
        brush->RemoveChangedHandler(m_backgroundBrushChangedHandlerId);
    }
    if (const auto brush = m_subscribedBorderBrush.lock();
        brush && m_borderBrushChangedHandlerId != 0) {
        brush->RemoveChangedHandler(m_borderBrushChangedHandlerId);
    }
}

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

std::shared_ptr<Brush> Control::GetBackgroundBrush() const {
    return GetValue<std::shared_ptr<Brush>>(BackgroundProperty());
}

void Control::SetBackgroundBrush(std::shared_ptr<Brush> brush) {
    if (!IsApplyingPropertyValue()) {
        if (GetBackgroundBrush() != brush) {
            SetValue(BackgroundProperty(), brush);
        }
        return;
    }

    if (m_subscribedBackgroundBrush.lock() == brush) {
        OnBackgroundBrushChanged(brush);
        return;
    }
    if (const auto oldBrush = m_subscribedBackgroundBrush.lock();
        oldBrush && m_backgroundBrushChangedHandlerId != 0) {
        oldBrush->RemoveChangedHandler(m_backgroundBrushChangedHandlerId);
        m_backgroundBrushChangedHandlerId = 0;
    }
    m_subscribedBackgroundBrush = brush;
    if (brush) {
        m_backgroundBrushChangedHandlerId = brush->AddChangedHandler([this] {
            OnBackgroundBrushChanged(GetBackgroundBrush());
            InvalidateVisual();
        });
    }
    OnBackgroundBrushChanged(brush);
}

std::shared_ptr<Brush> Control::GetBorderBrush() const {
    return GetValue<std::shared_ptr<Brush>>(BorderBrushProperty());
}

void Control::SetBorderBrush(std::shared_ptr<Brush> brush) {
    if (!IsApplyingPropertyValue()) {
        if (GetBorderBrush() != brush) {
            SetValue(BorderBrushProperty(), brush);
        }
        return;
    }

    if (m_subscribedBorderBrush.lock() == brush) {
        OnBorderBrushChanged(brush);
        return;
    }
    if (const auto oldBrush = m_subscribedBorderBrush.lock();
        oldBrush && m_borderBrushChangedHandlerId != 0) {
        oldBrush->RemoveChangedHandler(m_borderBrushChangedHandlerId);
        m_borderBrushChangedHandlerId = 0;
    }
    m_subscribedBorderBrush = brush;
    if (brush) {
        m_borderBrushChangedHandlerId = brush->AddChangedHandler([this] {
            OnBorderBrushChanged(GetBorderBrush());
            InvalidateVisual();
        });
    }
    OnBorderBrushChanged(brush);
}

void Control::OnBackgroundBrushChanged(const std::shared_ptr<Brush>& brush)
{
}

void Control::OnBorderBrushChanged(const std::shared_ptr<Brush>& brush)
{
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

void Control::OnBorderThicknessChanged(double) {}

}
