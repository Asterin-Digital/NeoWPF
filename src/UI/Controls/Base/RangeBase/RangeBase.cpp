#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <algorithm>

namespace NeoWPF::UI {

namespace {
double ClampValue(double value, double minimum, double maximum)
{
    return std::clamp(value, minimum, maximum);
}
}

void RangeBase::SetMinimum(double value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Minimum", *this)) {
            DependencyObject::SetValue(*property, value);
            return;
        }
    }

    const double clamped = std::max(value, 0.0);
    if (m_minimum == clamped) {
        return;
    }

    m_minimum = clamped;
    if (m_value < m_minimum) {
        m_value = m_minimum;
        OnValueChanged(m_value);
    }
}

void RangeBase::SetMaximum(double value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Maximum", *this)) {
            DependencyObject::SetValue(*property, value);
            return;
        }
    }

    const double clamped = std::max(value, m_minimum);
    if (m_maximum == clamped) {
        return;
    }

    m_maximum = clamped;
    if (m_value > m_maximum) {
        m_value = m_maximum;
        OnValueChanged(m_value);
    }
}

void RangeBase::SetValue(double value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Value", *this)) {
            DependencyObject::SetValue(*property, value);
            return;
        }
    }

    const double clamped = ClampValue(value, m_minimum, m_maximum);
    if (m_value == clamped) {
        return;
    }

    m_value = clamped;
    OnValueChanged(m_value);
}

void RangeBase::SetSmallChange(double value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("SmallChange", *this)) {
            DependencyObject::SetValue(*property, value);
            return;
        }
    }

    m_smallChange = std::max(0.0, value);
}

void RangeBase::SetLargeChange(double value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("LargeChange", *this)) {
            DependencyObject::SetValue(*property, value);
            return;
        }
    }
    m_largeChange = std::max(0.0, value);
}

void RangeBase::OnValueChanged(double value)
{
    InvalidateVisual();
    ValueChanged.Invoke(value);
}

}
