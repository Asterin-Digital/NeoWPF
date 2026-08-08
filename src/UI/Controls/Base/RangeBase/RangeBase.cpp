#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
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
    const double clamped = std::max(value, 0.0);
    if (m_minimum == clamped) {
        return;
    }

    m_minimum = clamped;
    if (m_value < m_minimum) {
        SetValue(m_minimum);
        return;
    }

    OnValueChanged(m_value);
}

void RangeBase::SetMaximum(double value) noexcept
{
    const double clamped = std::max(value, m_minimum);
    if (m_maximum == clamped) {
        return;
    }

    m_maximum = clamped;
    if (m_value > m_maximum) {
        SetValue(m_maximum);
        return;
    }

    OnValueChanged(m_value);
}

void RangeBase::SetValue(double value) noexcept
{
    const double clamped = ClampValue(value, m_minimum, m_maximum);
    if (m_value == clamped) {
        return;
    }

    m_value = clamped;
    OnValueChanged(m_value);
}

void RangeBase::SetSmallChange(double value) noexcept
{
    m_smallChange = std::max(0.0, value);
}

void RangeBase::SetLargeChange(double value) noexcept
{
    m_largeChange = std::max(0.0, value);
}

void RangeBase::OnValueChanged(double value)
{
    InvalidateVisual();
    ValueChanged.Invoke(value);
}

}
