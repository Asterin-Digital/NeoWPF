#include <NeoWPF/UI/Localization/LocaleOperations.h>

#include <NeoWPF/UI/Localization/LocaleExpression.h>

namespace NeoWPF::UI::Localization {

std::shared_ptr<LocaleExpression> LocaleOperations::SetLocale(
    Properties::DependencyObject& target,
    const Properties::DependencyProperty& property,
    const std::wstring& key)
{
    if (key.empty()) {
        return nullptr;
    }

    auto expression = std::make_shared<LocaleExpression>(target, property, key);
    target.SetLocaleExpression(property, expression);
    expression->Attach();
    return expression;
}

std::shared_ptr<LocaleExpression> LocaleOperations::SetLocale(
    Properties::DependencyObject& target,
    const Properties::DependencyProperty* property,
    const std::wstring& key)
{
    return property ? SetLocale(target, *property, key) : nullptr;
}

void LocaleOperations::ClearLocale(Properties::DependencyObject& target,
                                    const Properties::DependencyProperty& property)
{
    target.ClearLocaleExpressionValue(property);
}

void LocaleOperations::ClearLocale(Properties::DependencyObject& target,
                                    const Properties::DependencyProperty* property)
{
    if (property) {
        ClearLocale(target, *property);
    }
}

std::shared_ptr<LocaleExpression> LocaleOperations::GetLocaleExpression(
    const Properties::DependencyObject& target,
    const Properties::DependencyProperty& property)
{
    return target.GetLocaleExpression(property);
}

std::shared_ptr<LocaleExpression> LocaleOperations::GetLocaleExpression(
    const Properties::DependencyObject& target,
    const Properties::DependencyProperty* property)
{
    return property ? GetLocaleExpression(target, *property) : nullptr;
}

} // namespace NeoWPF::UI::Localization