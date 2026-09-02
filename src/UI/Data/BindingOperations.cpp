#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Data/BindingExpression.h>

namespace NeoWPF::UI::Data {

std::shared_ptr<BindingExpression> BindingOperations::SetBinding(Properties::DependencyObject& target,
                                                                 const Properties::DependencyProperty& property,
                                                                 const Binding& binding)
{
    auto expression = std::make_shared<BindingExpression>(target, property, binding);
    target.SetBindingExpression(property, expression);
    expression->Attach();
    return expression;
}

std::shared_ptr<BindingExpression> BindingOperations::SetBinding(Properties::DependencyObject& target,
                                                                 const Properties::DependencyProperty* property,
                                                                 const Binding& binding)
{
    if (!property) {
        const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kInvalidDependencyPropertyCode) + L": invalid dependency property used for binding";
        NeoWPF::Diagnostics::QueueDeveloperWarning(title,
            L"SetBinding was called with an invalid or unregistered DependencyProperty. Verify that the bound property exists and is registered for the target.",
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
        return nullptr;
    }
    return SetBinding(target, *property, binding);
}

std::shared_ptr<BindingExpression> BindingOperations::SetBinding(Properties::DependencyObject& target,
                                                                 const std::string& propertyName,
                                                                 const Binding& binding)
{
    if (const auto* property = target.FindDependencyProperty(propertyName)) {
        return SetBinding(target, *property, binding);
    }
    const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kInvalidDependencyPropertyCode) + L": invalid dependency property name used for binding";
    NeoWPF::Diagnostics::QueueDeveloperWarning(title,
        L"SetBinding was called with a property name that does not correspond to a registered DependencyProperty for the target.",
        NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
    return nullptr;
}

void BindingOperations::ClearBinding(Properties::DependencyObject& target,
                                     const Properties::DependencyProperty& property)
{
    if (auto expression = target.GetBindingExpression(property)) {
        expression->Detach();
    }
    target.ClearBindingExpression(property);
}

void BindingOperations::ClearBinding(Properties::DependencyObject& target,
                                     const Properties::DependencyProperty* property)
{
    if (!property) {
        return;
    }
    ClearBinding(target, *property);
}

void BindingOperations::ClearBinding(Properties::DependencyObject& target,
                                     const std::string& propertyName)
{
    if (const auto* property = target.FindDependencyProperty(propertyName)) {
        ClearBinding(target, *property);
    }
}

std::shared_ptr<BindingExpression> BindingOperations::GetBindingExpression(const Properties::DependencyObject& target,
                                                                          const Properties::DependencyProperty& property)
{
    return target.GetBindingExpression(property);
}

std::shared_ptr<BindingExpression> BindingOperations::GetBindingExpression(const Properties::DependencyObject& target,
                                                                          const Properties::DependencyProperty* property)
{
    if (!property) {
        return nullptr;
    }
    return GetBindingExpression(target, *property);
}

}
