/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <mutex>
#include <string>

#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>

namespace NeoWPF::UI::Styling::Triggers {

/*
    @summary
    Applies setters when a bound data value matches.
*/
class DataTrigger : public TriggerBase {
public:
    DataTrigger() = default;

    /*
        @summary
        Creates a data trigger.

        @param bindingPath
        Bound data path.

        @param value
        Value that activates the trigger.
    */
    DataTrigger(const std::string& bindingPath, const std::any& value);

    /*
        @summary
        Sets the bound data path.

        @param bindingPath
        Data path.
    */
    void SetBindingPath(const std::string& bindingPath) noexcept;
    std::string GetBindingPath() const;

    /*
        @summary
        Sets the value that activates the trigger.

        @param value
        Activation value.
    */
    void SetValue(const std::any& value);
    std::any GetValue() const;

    bool Evaluate(UIElement* element) const override;

private:
    mutable std::mutex m_dataMutex;
    std::string m_bindingPath;
    std::any m_value;
};

}
