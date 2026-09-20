/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

class RangeBase : public Control {
public:
    static const Properties::DependencyProperty& MinimumProperty();
    static const Properties::DependencyProperty& MaximumProperty();
    static const Properties::DependencyProperty& ValueProperty();
    static const Properties::DependencyProperty& SmallChangeProperty();
    static const Properties::DependencyProperty& LargeChangeProperty();

    RangeBase() = default;
    virtual ~RangeBase() = default;

    double GetMinimum() const noexcept { return m_minimum; }
    void SetMinimum(double value) noexcept;

    double GetMaximum() const noexcept { return m_maximum; }
    void SetMaximum(double value) noexcept;

    double GetValue() const noexcept { return m_value; }
    void SetValue(double value) noexcept;

    double GetSmallChange() const noexcept { return m_smallChange; }
    void SetSmallChange(double value) noexcept;

    double GetLargeChange() const noexcept { return m_largeChange; }
    void SetLargeChange(double value) noexcept;

    NeoWPF::Core::Event<double> ValueChanged;

protected:
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;
    virtual void OnValueChanged(double value);

private:
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    double m_value = 0.0;
    double m_smallChange = 1.0;
    double m_largeChange = 10.0;
};

}
