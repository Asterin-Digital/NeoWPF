/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <any>
#include <string>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling::Triggers {

/*
    @summary
    Assigns a dependency property value to an element.
*/
class Setter : public Properties::DependencyObject {
public:
    using Properties::DependencyObject::SetValue;

    Setter() = default;
    /*
        @summary
        Creates a setter for a property value.

        @param propertyName
        Property name.

        @param value
        Value to assign.
    */
    Setter(const std::string& propertyName, const std::any& value);

    static const Properties::DependencyProperty& PropertyProperty();
    static const Properties::DependencyProperty& ValueProperty();

    /*
        @summary
        Sets the property name targeted by the setter.

        @param propertyName
        Property name.
    */
    void SetProperty(const std::string& propertyName) noexcept;
    const std::string& GetProperty() const noexcept;

    /*
        @summary
        Sets the value assigned by the setter.

        @param value
        Value to assign.
    */
    void SetValue(const std::any& value);
    const std::any& GetValue() const noexcept;

    /*
        @summary
        Applies the setter to an element.

        @param element
        Element to update.
    */
    void Apply(UIElement* element) const;

private:
    std::string m_propertyName;
    std::any m_value;
};

}
