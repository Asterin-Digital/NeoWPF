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

class BindingOperations {
public:
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const Properties::DependencyProperty& property,
                                                         const Binding& binding);
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const Properties::DependencyProperty* property,
                                                         const Binding& binding);
    static std::shared_ptr<BindingExpression> SetBinding(Properties::DependencyObject& target,
                                                         const std::string& propertyName,
                                                         const Binding& binding);

    static void ClearBinding(Properties::DependencyObject& target,
                             const Properties::DependencyProperty& property);
    static void ClearBinding(Properties::DependencyObject& target,
                             const Properties::DependencyProperty* property);
    static void ClearBinding(Properties::DependencyObject& target,
                             const std::string& propertyName);

    static std::shared_ptr<BindingExpression> GetBindingExpression(const Properties::DependencyObject& target,
                                                                  const Properties::DependencyProperty& property);
    static std::shared_ptr<BindingExpression> GetBindingExpression(const Properties::DependencyObject& target,
                                                                  const Properties::DependencyProperty* property);
};

}
