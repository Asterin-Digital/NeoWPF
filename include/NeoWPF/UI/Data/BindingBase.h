/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <memory>

namespace NeoWPF::UI::Properties {
    class DependencyProperty;
    class DependencyObject;
}

namespace NeoWPF::UI::Data {

/*
    @summary
    Defines the update direction for a binding.
*/
enum class BindingMode {
    OneWay,
    TwoWay,
    OneTime,
};

/*
    @summary
    Stores common binding configuration.
*/
class BindingBase {
public:
    virtual ~BindingBase() = default;

    /*
        @summary
        Returns the binding update mode.
    */
    BindingMode GetMode() const noexcept {
        return m_mode;
    }

protected:
    explicit BindingBase(BindingMode mode = BindingMode::OneWay) : m_mode(mode) {}

    BindingMode m_mode;
};

}
