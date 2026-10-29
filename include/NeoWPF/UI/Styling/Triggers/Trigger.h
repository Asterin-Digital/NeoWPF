/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
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
    Applies setters when an element property matches a value.
*/
class Trigger : public TriggerBase {
public:
    Trigger() = default;

    /*
        @summary
        Creates a property trigger.

        @param propertyName
        Property name.

        @param value
        Value that activates the trigger.
    */
    Trigger(const std::string& propertyName, const std::any& value);

    /*
        @summary
        Sets the property observed by the trigger.

        @param propertyName
        Property name.
    */
    void SetProperty(const std::string& propertyName) noexcept;

    /*
        @summary
        Returns the property name observed by the trigger.
    */
    std::string GetProperty() const;

    /*
        @summary
        Sets the value that activates the trigger.

        @param value
        Activation value.
    */
    void SetValue(const std::any& value);

    /*
        @summary
        Returns the activation value for the trigger.
    */
    std::any GetValue() const;

    /*
        @summary
        Tests whether the element's property value activates the trigger.

        @param element
        Element whose property is evaluated.

        @internal
    */
    bool Evaluate(UIElement* element) const override;

private:
    mutable std::mutex m_triggerMutex;
    std::string m_propertyName;
    std::any m_value;
};

}
