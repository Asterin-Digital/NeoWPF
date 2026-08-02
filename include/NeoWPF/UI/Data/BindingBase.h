#pragma once

#include <memory>

namespace NeoWPF::UI::Properties {
    class DependencyProperty;
    class DependencyObject;
}

namespace NeoWPF::UI::Data {

enum class BindingMode {
    OneWay,
    TwoWay,
    OneTime,
};

class BindingBase {
public:
    virtual ~BindingBase() = default;

    BindingMode GetMode() const noexcept {
        return m_mode;
    }

protected:
    explicit BindingBase(BindingMode mode = BindingMode::OneWay) : m_mode(mode) {}

    BindingMode m_mode;
};

}
