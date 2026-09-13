#pragma once

#include <memory>
#include <string>

#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Data/Binding.h>

namespace NeoWPF::UI::Data {
    class BindingExpression;
}

namespace NeoWPF::UI::Data {

/*
    @summary
    Provides operations for assigning bindings to dependency properties.
*/
class BindingOperations {
public:
    /*
        @summary
        Assigns a binding to a dependency property.

        @param target
        Object whose property is updated.

        @param property
        Property to update.

        @param binding
        Binding configuration.

        @returns
        The attached binding expression.
    */
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const Properties::DependencyProperty& property,
                                                         const Binding& binding);
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const Properties::DependencyProperty* property,
                                                         const Binding& binding);
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const std::string& propertyName,
                                                         const Binding& binding);

    /*
        @summary
        Removes a binding from a dependency property.

        @param target
        Object whose property is cleared.

        @param property
        Property to clear.
    */
    static void ClearBinding(Properties::DependencyObject& target,
                             const Properties::DependencyProperty& property);
    static void ClearBinding(Properties::DependencyObject& target,
                             const Properties::DependencyProperty* property);
    static void ClearBinding(Properties::DependencyObject& target,
                             const std::string& propertyName);

    /*
        @summary
        Returns the binding expression assigned to a dependency property.

        @param target
        Object to inspect.

        @param property
        Property to inspect.
    */
    static std::shared_ptr<BindingExpression> GetBindingExpression(const Properties::DependencyObject& target,
                                                                  const Properties::DependencyProperty& property);
    static std::shared_ptr<BindingExpression> GetBindingExpression(const Properties::DependencyObject& target,
                                                                  const Properties::DependencyProperty* property);
};

}
