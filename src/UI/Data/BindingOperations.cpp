#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Data/BindingExpression.h>

namespace NeoWPF::UI::Data {

std::shared_ptr<BindingExpression> BindingOperations::SetBinding(Properties::DependencyObject& target,
                                                                 const Properties::DependencyProperty& property,
                                                                 const Binding& binding)
{
    auto expression = std::make_shared<BindingExpression>(target, property, binding);
    expression->Attach();
    target.SetBindingExpression(property, expression);
    return expression;
}

void BindingOperations::ClearBinding(Properties::DependencyObject& target,
                                     const Properties::DependencyProperty& property)
{
    if (auto expression = target.GetBindingExpression(property)) {
        expression->Detach();
    }
    target.ClearBindingExpression(property);
}

std::shared_ptr<BindingExpression> BindingOperations::GetBindingExpression(const Properties::DependencyObject& target,
                                                                          const Properties::DependencyProperty& property)
{
    return target.GetBindingExpression(property);
}

}
